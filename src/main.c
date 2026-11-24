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
#include "dictionary.h"
#include "game.h"
#include "input.h"
#include "ui.h"
#include "gfx.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DICTIONARY_PATH "data/words.txt"

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
    }
}

static void delete_letter(App *app)
{
    if (app->typed_length > 0) {
        app->typed[--app->typed_length] = '\0';
    }
}

static void submit_guess(App *app)
{
    if (app->typed_length < WORD_LENGTH) {
        set_message(app, "NOT ENOUGH LETTERS", 1);
        return;
    }

    if (!dictionary_contains(app->words, app->word_count, app->typed)) {
        set_message(app, "NOT IN WORD LIST", 1);
        return;
    }

    if (game_submit_guess(&app->game, app->typed) != GUESS_ACCEPTED) {
        return;
    }

    clear_typed(app);

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

static void handle_menu(App *app, const InputEvent *event)
{
    UiAction action = click_action(app, event, NULL);

    if (event->type == INPUT_ENTER)  action = UI_ACTION_NEW_GAME;
    if (event->type == INPUT_ESCAPE) action = UI_ACTION_EXIT;

    switch (action) {
    case UI_ACTION_NEW_GAME:    start_new_game(app);                break;
    case UI_ACTION_HOW_TO_PLAY: app->screen = SCREEN_HOW_TO_PLAY;   break;
    case UI_ACTION_EXIT:        app->screen = SCREEN_GOODBYE;       break;
    default:                                                         break;
    }
}

static void handle_how_to_play(App *app, const InputEvent *event)
{
    UiAction action = click_action(app, event, NULL);

    if (action == UI_ACTION_BACK ||
        event->type == INPUT_ESCAPE ||
        event->type == INPUT_ENTER) {
        app->screen = SCREEN_MENU;
    }
}

static void handle_playing(App *app, const InputEvent *event)
{
    char letter = '\0';
    UiAction action = click_action(app, event, &letter);

    if (action == UI_ACTION_MAIN_MENU) {
        app->screen = SCREEN_MENU;
        return;
    }

    /* Game finished: Enter or any click continues to the result. */
    if (game_is_over(&app->game)) {
        if (event->type == INPUT_ENTER || event->type == INPUT_CLICK) {
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
        clear_typed(app);
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
        start_new_game(app);
    }
    else if (action == UI_ACTION_MAIN_MENU) {
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

static void draw(const App *app)
{
    UiView view = {
        .screen = app->screen,
        .game = &app->game,
        .typed = app->typed,
        .message = app->message,
        .message_is_error = app->message_is_error,
    };

    ui_draw(&view);
}


int main(void)
{
    static App app;   /* static: zero-initialised, and off the stack */

    app.words = dictionary_load(DICTIONARY_PATH, &app.word_count);

    if (app.words == NULL) {
        fprintf(stderr, "Failed to load dictionary: %s\n", DICTIONARY_PATH);
        return 1;
    }

    srand((unsigned int)time(NULL));

    app.screen = SCREEN_MENU;
    app.running = 1;
    clear_typed(&app);

    input_init();
    ui_init();
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

        if (event.type != INPUT_REDRAW) {
            handle_event(&app, &event);
        }

        if (app.running) {
            draw(&app);
        }
    }

    ui_shutdown();
    dictionary_free(app.words, app.word_count);

    return 0;
}
