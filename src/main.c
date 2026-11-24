/*
 * Wordle - program entry point and screen flow.
 *
 * The program is a small state machine. `app.screen` says which
 * screen is active; every event is passed to that screen's handler,
 * which may change state or move to another screen:
 *
 *   MENU --NEW GAME--> PLAYING --game over--> RESULT --PLAY AGAIN--> PLAYING
 *    |  \--HOW TO PLAY--> HOW_TO_PLAY --BACK--> MENU     \--MAIN MENU--> MENU
 *    \--EXIT--> GOODBYE --EXIT--> (quit)
 *
 * Closing the window quits from any screen.
 */

/* For readlink() (POSIX). Must come before any #include. */
#define _POSIX_C_SOURCE 200809L

#include "dictionary.h"
#include "game.h"
#include "input.h"
#include "ui.h"
#include "gfx.h"
#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DICTIONARY_FILE "words.txt"
#define PATH_SIZE 4096

typedef struct {
    char **words;
    int word_count;

    Screen screen;
    int running;

    Game game;
    char typed[WORD_LENGTH + 1];   /* letters typed in the current row */
    int typed_length;

    const char *message;           /* status line, or NULL */
    int message_is_error;
} App;


/* ---- small state helpers ------------------------------------------- */

static void clear_typed(App *app)
{
    app->typed_length = 0;
    app->typed[0] = '\0';
}

static void set_message(App *app, const char *message, int is_error)
{
    app->message = message;
    app->message_is_error = is_error;
}

static void start_new_game(App *app)
{
    const char *answer = dictionary_pick_random(app->words, app->word_count);

    /* Every dictionary word is valid, so this cannot fail. */
    game_init(&app->game, answer);

    clear_typed(app);
    set_message(app, NULL, 0);
    app->screen = SCREEN_PLAYING;
}

static void type_letter(App *app, char letter)
{
    if (app->typed_length < WORD_LENGTH) {
        app->typed[app->typed_length++] = letter;
        app->typed[app->typed_length] = '\0';
        audio_play(SOUND_KEY);
    }
}

static void delete_letter(App *app)
{
    if (app->typed_length > 0) {
        app->typed[--app->typed_length] = '\0';
        audio_play(SOUND_DELETE);
    }
}

static void clear_row(App *app)
{
    if (app->typed_length > 0) {
        clear_typed(app);
        audio_play(SOUND_DELETE);
    }
}

/* Time between the notes of the tile-by-tile reveal. */
#define REVEAL_STEP_MS 110

/* One note per tile (pitch by colour), then the win/lose tune. */
static void play_guess_sounds(const App *app)
{
    const GuessResult *guess = &app->game.guesses[app->game.current_guess - 1];

    for (int i = 0; i < WORD_LENGTH; i++) {
        SoundEffect note = SOUND_TILE_ABSENT;

        if (guess->results[i] == LETTER_CORRECT) note = SOUND_TILE_CORRECT;
        if (guess->results[i] == LETTER_PRESENT) note = SOUND_TILE_PRESENT;

        audio_play_after(note, i * REVEAL_STEP_MS);
    }

    int after_reveal = WORD_LENGTH * REVEAL_STEP_MS + 80;

    if (app->game.won) {
        audio_play_after(SOUND_WIN, after_reveal);
    }
    else if (game_is_over(&app->game)) {
        audio_play_after(SOUND_LOSE, after_reveal);
    }
}

static void submit_guess(App *app)
{
    if (app->typed_length < WORD_LENGTH) {
        set_message(app, "NOT ENOUGH LETTERS", 1);
        audio_play(SOUND_INVALID);
        return;
    }

    if (!dictionary_contains(app->words, app->word_count, app->typed)) {
        set_message(app, "NOT IN WORD LIST", 1);
        audio_play(SOUND_INVALID);
        return;
    }

    if (game_submit_guess(&app->game, app->typed) != GUESS_ACCEPTED) {
        return;
    }

    clear_typed(app);
    play_guess_sounds(app);

    /*
     * Stay on the board so the player can see the final colours;
     * the next Enter or click moves on to the result screen.
     */
    if (game_is_over(&app->game)) {
        set_message(app,
                    app->game.won ? "YOU GOT IT! PRESS ENTER"
                                  : "OUT OF GUESSES - PRESS ENTER",
                    0);
    }
}


/* ---- one handler per screen ---------------------------------------- */

/* Clicks become UiActions; keys are translated to the same actions. */
static UiAction click_action(const App *app, const InputEvent *event, char *letter)
{
    if (event->type != INPUT_CLICK) {
        return UI_ACTION_NONE;
    }

    return ui_hit_test(app->screen, event->x, event->y, letter);
}

static void toggle_sound(void)
{
    audio_set_muted(!audio_is_muted());
    audio_play(SOUND_CLICK);   /* heard only when sound was just turned on */
}

static void handle_menu(App *app, const InputEvent *event)
{
    UiAction action = click_action(app, event, NULL);

    if (event->type == INPUT_ENTER)  action = UI_ACTION_NEW_GAME;
    if (event->type == INPUT_ESCAPE) action = UI_ACTION_EXIT;

    switch (action) {
    case UI_ACTION_NEW_GAME:     audio_play(SOUND_CLICK); start_new_game(app);              break;
    case UI_ACTION_HOW_TO_PLAY:  audio_play(SOUND_CLICK); app->screen = SCREEN_HOW_TO_PLAY; break;
    case UI_ACTION_EXIT:         audio_play(SOUND_CLICK); app->screen = SCREEN_GOODBYE;     break;
    case UI_ACTION_TOGGLE_SOUND: toggle_sound();                                            break;
    default:                                                                                 break;
    }
}

static void handle_how_to_play(App *app, const InputEvent *event)
{
    UiAction action = click_action(app, event, NULL);

    if (action == UI_ACTION_BACK ||
        event->type == INPUT_ESCAPE ||
        event->type == INPUT_ENTER) {
        audio_play(SOUND_CLICK);
        app->screen = SCREEN_MENU;
    }
}

static void handle_playing(App *app, const InputEvent *event)
{
    char letter = '\0';
    UiAction action = click_action(app, event, &letter);

    if (action == UI_ACTION_MAIN_MENU) {
        audio_play(SOUND_CLICK);
        app->screen = SCREEN_MENU;
        return;
    }

    /* Game finished: Enter or any click continues to the result. */
    if (game_is_over(&app->game)) {
        if (event->type == INPUT_ENTER || event->type == INPUT_CLICK) {
            audio_play(SOUND_CLICK);
            set_message(app, NULL, 0);
            app->screen = SCREEN_RESULT;
        }
        return;
    }

    switch (event->type) {
    case INPUT_LETTER:    action = UI_ACTION_LETTER; letter = event->letter; break;
    case INPUT_BACKSPACE: action = UI_ACTION_DELETE;                         break;
    case INPUT_ENTER:     action = UI_ACTION_ENTER;                          break;
    case INPUT_ESCAPE:    action = UI_ACTION_CLEAR;                          break;
    default:                                                                  break;
    }

    switch (action) {
    case UI_ACTION_LETTER:
        set_message(app, NULL, 0);
        type_letter(app, letter);
        break;
    case UI_ACTION_DELETE:
        set_message(app, NULL, 0);
        delete_letter(app);
        break;
    case UI_ACTION_CLEAR:
        set_message(app, NULL, 0);
        clear_row(app);
        break;
    case UI_ACTION_ENTER:
        submit_guess(app);
        break;
    default:
        break;
    }
}

static void handle_result(App *app, const InputEvent *event)
{
    UiAction action = click_action(app, event, NULL);

    if (event->type == INPUT_ENTER)  action = UI_ACTION_PLAY_AGAIN;
    if (event->type == INPUT_ESCAPE) action = UI_ACTION_MAIN_MENU;

    if (action == UI_ACTION_PLAY_AGAIN) {
        audio_play(SOUND_CLICK);
        start_new_game(app);
    }
    else if (action == UI_ACTION_MAIN_MENU) {
        audio_play(SOUND_CLICK);
        app->screen = SCREEN_MENU;
    }
}

static void handle_goodbye(App *app, const InputEvent *event)
{
    UiAction action = click_action(app, event, NULL);

    if (action == UI_ACTION_QUIT ||
        event->type == INPUT_ENTER ||
        event->type == INPUT_ESCAPE) {
        app->running = 0;
    }
}

static void handle_event(App *app, const InputEvent *event)
{
    switch (app->screen) {
    case SCREEN_MENU:        handle_menu(app, event);        break;
    case SCREEN_HOW_TO_PLAY: handle_how_to_play(app, event); break;
    case SCREEN_PLAYING:     handle_playing(app, event);     break;
    case SCREEN_RESULT:      handle_result(app, event);      break;
    case SCREEN_GOODBYE:     handle_goodbye(app, event);     break;
    }
}

/* Menu music plays on the menu and How to Play screens only. */
static void update_music(const App *app)
{
    audio_set_music(app->screen == SCREEN_MENU ||
                    app->screen == SCREEN_HOW_TO_PLAY);
}

static void draw(const App *app)
{
    UiSoundState sound = UI_SOUND_UNAVAILABLE;

    if (audio_is_available()) {
        sound = audio_is_muted() ? UI_SOUND_OFF : UI_SOUND_ON;
    }

    UiView view = {
        .screen = app->screen,
        .game = &app->game,
        .typed = app->typed,
        .message = app->message,
        .message_is_error = app->message_is_error,
        .sound = sound,
    };

    ui_draw(&view);
}


/* ---- locating the word list ---------------------------------------- */

/*
 * Write the folder containing the running program into `dir`.
 * On Linux, /proc/self/exe is a symbolic link to the executable.
 */
static int executable_dir(char *dir, size_t size)
{
    ssize_t length = readlink("/proc/self/exe", dir, size - 1);

    if (length <= 0) {
        return 0;
    }

    dir[length] = '\0';   /* readlink() does not add the terminator */

    char *last_slash = strrchr(dir, '/');
    if (last_slash == NULL) {
        return 0;
    }

    *last_slash = '\0';
    return 1;
}

/*
 * Find and load the word list, so the game works no matter which
 * folder it is started from. Tried in order:
 *   <program dir>/../data/words.txt          development build (build/wordle)
 *   <program dir>/../share/wordle/words.txt  after `make install`
 *   data/words.txt                           relative to the current folder
 */
static char **load_word_list(int *count)
{
    static const char *const FROM_PROGRAM[] = {
        "../data/" DICTIONARY_FILE,          /* adjacent literals are joined */
        "../share/wordle/" DICTIONARY_FILE,
    };

    char dir[PATH_SIZE];

    if (executable_dir(dir, sizeof(dir))) {
        for (size_t i = 0; i < sizeof(FROM_PROGRAM) / sizeof(FROM_PROGRAM[0]); i++) {
            char path[PATH_SIZE + 64];
            int needed = snprintf(path, sizeof(path), "%s/%s", dir, FROM_PROGRAM[i]);

            /* snprintf returns the length it wanted; skip if it was cut short. */
            if (needed < 0 || (size_t)needed >= sizeof(path)) {
                continue;
            }

            char **words = dictionary_load(path, count);
            if (words != NULL) {
                return words;
            }
        }
    }

    return dictionary_load("data/" DICTIONARY_FILE, count);
}


int main(void)
{
    static App app;   /* static: zero-initialised, and off the stack */

    app.words = load_word_list(&app.word_count);

    if (app.words == NULL) {
        fprintf(stderr,
                "Failed to load the word list (" DICTIONARY_FILE ").\n"
                "Run the game from the project folder, or install it with "
                "`make install`.\n");
        return 1;
    }

    srand((unsigned int)time(NULL));

    app.screen = SCREEN_MENU;
    app.running = 1;
    clear_typed(&app);

    input_init();

    if (!ui_init()) {
        /* gfx_open() already explained why; clean up before leaving. */
        dictionary_free(app.words, app.word_count);
        return 1;
    }

    /* Sound is optional: if it fails, the game simply stays silent. */
    audio_init();
    update_music(&app);

    draw(&app);

    while (app.running) {
        InputEvent event = input_wait_event();

        if (event.type == INPUT_QUIT) {
            break;
        }

        if (event.type == INPUT_FULLSCREEN) {
            /* The resize arrives later as an INPUT_REDRAW. */
            gfx_toggle_fullscreen();
            continue;
        }

        if (event.type == INPUT_TOGGLE_SOUND) {
            toggle_sound();
        }
        else if (event.type != INPUT_REDRAW) {
            handle_event(&app, &event);
        }

        update_music(&app);

        if (app.running) {
            draw(&app);
        }
    }

    audio_shutdown();
    ui_shutdown();
    dictionary_free(app.words, app.word_count);

    return 0;
}
