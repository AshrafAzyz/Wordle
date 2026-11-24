/*
 * User interface: all screens of the game.
 *
 * Layout model
 * ------------
 * Every screen is designed on a fixed DESIGN_WIDTH x DESIGN_HEIGHT
 * canvas (the centre panel of the original 1920x1080 layout, i.e.
 * original x minus 500). update_view() computes one scale factor
 * that fits this canvas inside the current window and centres it;
 * the space left over becomes the coloured side frame, just like
 * the black side bars of the original design.
 *
 * Drawing converts design units to pixels (px/py), and hit testing
 * converts pixels back to design units, so clickable areas always
 * match what is drawn, whatever the window size.
 */
#include "ui.h"
#include "gfx.h"
#include "font.h"

#include <stdio.h>
#include <string.h>

#define DESIGN_WIDTH  920
#define DESIGN_HEIGHT 1080

/* Requested window size; gfx_open() shrinks it to fit the screen. */
#define WINDOW_WIDTH  1400
#define WINDOW_HEIGHT 1000


/* ---- colours (from the original pixel-art design) ------------------ */

typedef struct {
    int r, g, b;
} Color;

static const Color COLOR_FRAME        = {   0,   0,   0 };
static const Color COLOR_MENU_BG      = { 247, 241, 227 };
static const Color COLOR_GAME_BG      = { 209, 204, 192 };
static const Color COLOR_END_BG       = {  26,  26,  27 };
static const Color COLOR_TEAL         = {  33, 140, 124 };
static const Color COLOR_ORANGE       = { 204, 142,  53 };
static const Color COLOR_BUTTON_FACE  = { 209, 204, 192 };
static const Color COLOR_BUTTON_LIGHT = { 217, 217, 217 };
static const Color COLOR_SHADOW       = { 132, 129, 122 };
static const Color COLOR_BLACK        = {   0,   0,   0 };
static const Color COLOR_WHITE        = { 255, 255, 255 };
static const Color COLOR_CORRECT      = {  83, 141,  78 };
static const Color COLOR_PRESENT      = { 181, 159,  59 };
static const Color COLOR_ABSENT       = { 128, 128, 128 };
static const Color COLOR_KEY_ABSENT   = {  58,  58,  60 };
static const Color COLOR_ERROR        = { 230,  70,  70 };


/* ---- design-space -> pixel conversion ------------------------------ */

static double view_scale = 1.0;
static int view_x = 0;
static int view_y = 0;

static void update_view(void)
{
    double scale_x = (double)gfx_xsize() / DESIGN_WIDTH;
    double scale_y = (double)gfx_ysize() / DESIGN_HEIGHT;

    view_scale = scale_x < scale_y ? scale_x : scale_y;
    view_x = (int)((gfx_xsize() - DESIGN_WIDTH * view_scale) / 2);
    view_y = (int)((gfx_ysize() - DESIGN_HEIGHT * view_scale) / 2);
}

static int px(int design_x)
{
    return view_x + (int)(design_x * view_scale + 0.5);
}

static int py(int design_y)
{
    return view_y + (int)(design_y * view_scale + 0.5);
}

static void set_color(Color c)
{
    gfx_color(c.r, c.g, c.b);
}

/*
 * Fill a rectangle given in design units. Converting both edges
 * (rather than position + size) keeps neighbouring shapes aligned.
 */
static void fill_rect(int x, int y, int w, int h)
{
    int left = px(x);
    int top = py(y);
    int width = px(x + w) - left;
    int height = py(y + h) - top;

    /* Never let a thin line vanish on a small window. */
    if (w > 0 && width < 1) {
        width = 1;
    }
    if (h > 0 && height < 1) {
        height = 1;
    }

    gfx_fillrectangle(left, top, width, height);
}

/* Rectangle outline with the given thickness (design units). */
static void draw_border(int x, int y, int w, int h, int t)
{
    fill_rect(x, y, w, t);
    fill_rect(x, y + h - t, w, t);
    fill_rect(x, y, t, h);
    fill_rect(x + w - t, y, t, h);
}

/*
 * Pixel-art text centred on (cx, cy). The font square size is
 * rounded down so text never grows wider than it was designed.
 */
static void draw_text(const char *text, int cx, int cy, int design_pixel, Color c)
{
    int pixel = (int)(design_pixel * view_scale);

    if (pixel < 1) {
        pixel = 1;
    }

    int width = font_text_width(text, pixel);
    int height = font_text_height(pixel);

    set_color(c);
    font_draw_text(text, px(cx) - width / 2, py(cy) - height / 2, pixel);
}

/* Design-unit width of text at a design pixel size. */
static int text_design_width(const char *text, int design_pixel)
{
    return font_text_width(text, design_pixel);
}


/* ---- backgrounds --------------------------------------------------- */

static void draw_background(Color frame, Color panel)
{
    gfx_clear_color(frame.r, frame.g, frame.b);
    gfx_clear();

    set_color(panel);
    fill_rect(0, 0, DESIGN_WIDTH, DESIGN_HEIGHT);
}


/* ---- the pixel-art WORDLE logo (original block shapes) ------------- */

typedef struct {
    int x, y, w, h;
    Color color;
} Block;

static const Block LOGO[] = {
    /* W */
    { 110,  0, 20, 80, {  33, 140, 124 } },
    { 150, 20, 20, 60, {  33, 140, 124 } },
    { 190,  0, 20, 80, {  33, 140, 124 } },
    { 130, 80, 60, 20, {  33, 140, 124 } },
    /* O */
    { 250,  0, 60, 20, { 217, 217, 217 } },
    { 230, 20, 20, 60, { 217, 217, 217 } },
    { 310, 20, 20, 60, { 217, 217, 217 } },
    { 250, 80, 60, 20, { 217, 217, 217 } },
    /* R */
    { 350,  0, 20, 100, { 132, 129, 122 } },
    { 370,  0, 60, 20, { 132, 129, 122 } },
    { 370, 40, 60, 20, { 132, 129, 122 } },
    { 430, 20, 20, 20, { 132, 129, 122 } },
    { 410, 60, 20, 20, { 132, 129, 122 } },
    { 430, 80, 20, 20, { 132, 129, 122 } },
    /* D */
    { 470,  0, 20, 100, { 132, 129, 122 } },
    { 490,  0, 60, 20, { 132, 129, 122 } },
    { 490, 80, 60, 20, { 132, 129, 122 } },
    { 550, 20, 20, 60, { 132, 129, 122 } },
    /* L */
    { 590,  0, 20, 100, { 217, 217, 217 } },
    { 610, 80, 80, 20, { 217, 217, 217 } },
    /* E */
    { 710,  0, 20, 100, { 204, 142,  53 } },
    { 730,  0, 80, 20, { 204, 142,  53 } },
    { 730, 40, 60, 20, { 204, 142,  53 } },
    { 730, 80, 80, 20, { 204, 142,  53 } },
};

#define LOGO_BLOCKS ((int)(sizeof(LOGO) / sizeof(LOGO[0])))

static void draw_logo(int top)
{
    for (int i = 0; i < LOGO_BLOCKS; i++) {
        set_color(LOGO[i].color);
        fill_rect(LOGO[i].x, top + LOGO[i].y, LOGO[i].w, LOGO[i].h);
    }
}


/* ---- buttons ------------------------------------------------------- */

typedef struct {
    UiAction action;
    int x, y, w, h;
    const char *label;
} Button;

#define COUNT(array) ((int)(sizeof(array) / sizeof((array)[0])))

static const Button MENU_BUTTONS[] = {
    { UI_ACTION_NEW_GAME,    310, 395, 300, 100, "NEW GAME" },
    { UI_ACTION_HOW_TO_PLAY, 310, 540, 300, 100, "HOW TO PLAY" },
    { UI_ACTION_EXIT,        310, 685, 300, 100, "EXIT" },
};

static const Button HOW_TO_PLAY_BUTTONS[] = {
    { UI_ACTION_BACK, 310, 920, 300, 100, "BACK" },
};

static const Button RESULT_BUTTONS[] = {
    { UI_ACTION_PLAY_AGAIN, 310, 620, 300, 100, "PLAY AGAIN" },
    { UI_ACTION_MAIN_MENU,  310, 765, 300, 100, "MAIN MENU" },
};

static const Button GOODBYE_BUTTONS[] = {
    { UI_ACTION_QUIT, 310, 482, 300, 100, "EXIT" },
};

/* Raised button with any label: offset shadow, then the face. */
static void draw_raised(const Button *button, Color face,
                        const char *label, int text_pixel)
{
    set_color(COLOR_SHADOW);
    fill_rect(button->x + 10, button->y + 15, button->w, button->h);

    set_color(face);
    fill_rect(button->x, button->y, button->w, button->h);

    draw_text(label,
              button->x + button->w / 2, button->y + button->h / 2,
              text_pixel, COLOR_BLACK);
}

/* Raised button with its own label (original style). */
static void draw_button(const Button *button, Color face)
{
    draw_raised(button, face, button->label, 4);
}

static void draw_buttons(const Button *buttons, int count, Color face)
{
    for (int i = 0; i < count; i++) {
        draw_button(&buttons[i], face);
    }
}

static int contains(int x, int y, int w, int h, double dx, double dy)
{
    return dx >= x && dx < x + w && dy >= y && dy < y + h;
}

static UiAction hit_buttons(const Button *buttons, int count, double dx, double dy)
{
    for (int i = 0; i < count; i++) {
        const Button *b = &buttons[i];

        if (contains(b->x, b->y, b->w, b->h, dx, dy)) {
            return b->action;
        }
    }

    return UI_ACTION_NONE;
}


/* ---- tiles --------------------------------------------------------- */

static Color tile_color(LetterResult result, Color empty)
{
    switch (result) {
    case LETTER_CORRECT: return COLOR_CORRECT;
    case LETTER_PRESENT: return COLOR_PRESENT;
    case LETTER_ABSENT:  return COLOR_ABSENT;
    case LETTER_EMPTY:   break;
    }

    return empty;
}

/* Square tile with a black border and an optional letter. */
static void draw_tile(int x, int y, int size, LetterResult result,
                      char letter, Color empty, int letter_pixel)
{
    set_color(tile_color(result, empty));
    fill_rect(x, y, size, size);

    set_color(COLOR_BLACK);
    draw_border(x, y, size, size, 3);

    if (letter != '\0') {
        char text[2] = { letter, '\0' };
        draw_text(text, x + size / 2, y + size / 2, letter_pixel, COLOR_BLACK);
    }
}


/* ---- main menu ----------------------------------------------------- */

/* Smaller button under the main three; its label shows the state. */
static const Button SOUND_BUTTON = {
    UI_ACTION_TOGGLE_SOUND, 330, 820, 260, 56, NULL
};

static const char *sound_label(UiSoundState sound)
{
    switch (sound) {
    case UI_SOUND_ON:          return "SOUND: ON";
    case UI_SOUND_OFF:         return "SOUND: OFF";
    case UI_SOUND_UNAVAILABLE: break;
    }

    return "NO SOUND";
}

static void draw_menu(const UiView *view)
{
    draw_background(COLOR_FRAME, COLOR_MENU_BG);
    draw_logo(120);
    draw_text("WELCOME TO OUR WORDLE GAME!!!", 460, 300, 3, COLOR_BLACK);
    draw_buttons(MENU_BUTTONS, COUNT(MENU_BUTTONS), COLOR_BUTTON_FACE);
    draw_raised(&SOUND_BUTTON, COLOR_BUTTON_LIGHT, sound_label(view->sound), 3);
    draw_text("ENTER: NEW GAME   ESC: EXIT   F2: SOUND", 460, 960, 3, COLOR_SHADOW);
}


/* ---- how to play --------------------------------------------------- */

typedef struct {
    const char *word;
    int highlight;          /* which tile is coloured */
    LetterResult result;
    const char *caption;
} Example;

static const Example EXAMPLES[] = {
    { "WESRY", 0, LETTER_CORRECT, "W IS IN THE WORD AND IN THE CORRECT SPOT." },
    { "PILLS", 1, LETTER_PRESENT, "I IS IN THE WORD BUT IN THE WRONG SPOT." },
    { "VAGUE", 2, LETTER_ABSENT,  "G IS NOT IN THE WORD IN ANY SPOT." },
};

static void draw_how_to_play(void)
{
    draw_background(COLOR_FRAME, COLOR_GAME_BG);

    draw_text("HOW TO PLAY", 460, 70, 7, COLOR_BLACK);

    draw_text("GUESS THE WORD IN 6 TRIES.", 460, 150, 3, COLOR_BLACK);
    draw_text("EACH GUESS MUST BE A VALID 5-LETTER WORD.", 460, 190, 3, COLOR_BLACK);
    draw_text("THE TILE COLOURS SHOW HOW CLOSE", 460, 230, 3, COLOR_BLACK);
    draw_text("YOUR GUESS WAS TO THE WORD.", 460, 265, 3, COLOR_BLACK);
    draw_text("EXAMPLES:", 460, 320, 3, COLOR_BLACK);

    for (int i = 0; i < COUNT(EXAMPLES); i++) {
        int top = 350 + i * 150;

        for (int col = 0; col < WORD_LENGTH; col++) {
            LetterResult result =
                (col == EXAMPLES[i].highlight) ? EXAMPLES[i].result : LETTER_EMPTY;

            draw_tile(290 + col * 70, top, 60, result,
                      EXAMPLES[i].word[col], COLOR_BUTTON_LIGHT, 4);
        }

        draw_text(EXAMPLES[i].caption, 460, top + 90, 3, COLOR_BLACK);
    }

    draw_text("TYPE OR CLICK LETTERS. ENTER SUBMITS.", 460, 805, 3, COLOR_BLACK);
    draw_text("BACKSPACE DELETES, ESC CLEARS.", 460, 840, 3, COLOR_BLACK);
    draw_text("F11: FULLSCREEN   F2: SOUND ON/OFF", 460, 875, 3, COLOR_BLACK);

    draw_buttons(HOW_TO_PLAY_BUTTONS, COUNT(HOW_TO_PLAY_BUTTONS), COLOR_BUTTON_LIGHT);
}


/* ---- game screen --------------------------------------------------- */

#define BOARD_TILE   80
#define BOARD_PITCH  90
#define BOARD_LEFT   ((DESIGN_WIDTH - (WORD_LENGTH * BOARD_PITCH - (BOARD_PITCH - BOARD_TILE))) / 2)
#define BOARD_TOP    170

#define MESSAGE_Y    735

#define KEY_WIDTH    58
#define KEY_HEIGHT   66
#define KEY_PITCH_X  66
#define KEY_PITCH_Y  76
#define KEYS_PER_ROW 13
#define KEYS_LEFT    ((DESIGN_WIDTH - (KEYS_PER_ROW * KEY_PITCH_X - (KEY_PITCH_X - KEY_WIDTH))) / 2)
#define KEYS_TOP     770

static const Button GAME_BUTTONS[] = {
    { UI_ACTION_MAIN_MENU, 10, 10, 90, 44, "MENU" },
    { UI_ACTION_ENTER,  KEYS_LEFT,       KEYS_TOP + 2 * KEY_PITCH_Y, 200, KEY_HEIGHT, "ENTER" },
    { UI_ACTION_DELETE, 360,             KEYS_TOP + 2 * KEY_PITCH_Y, 200, KEY_HEIGHT, "DEL" },
    { UI_ACTION_CLEAR,  DESIGN_WIDTH - KEYS_LEFT - 200,
                        KEYS_TOP + 2 * KEY_PITCH_Y, 200, KEY_HEIGHT, "CLEAR" },
};

static void key_position(int index, int *x, int *y)
{
    *x = KEYS_LEFT + (index % KEYS_PER_ROW) * KEY_PITCH_X;
    *y = KEYS_TOP + (index / KEYS_PER_ROW) * KEY_PITCH_Y;
}

static void draw_key(int x, int y, int w, int h, const char *label,
                     Color fill, Color text, int text_pixel)
{
    set_color(fill);
    fill_rect(x, y, w, h);

    set_color(COLOR_BLACK);
    draw_border(x, y, w, h, 2);

    draw_text(label, x + w / 2, y + h / 2, text_pixel, text);
}

static void draw_board(const UiView *view)
{
    const Game *game = view->game;
    int typing_row = game_is_over(game) ? -1 : game->current_guess;

    for (int row = 0; row < MAX_GUESSES; row++) {
        for (int col = 0; col < WORD_LENGTH; col++) {
            LetterResult result = LETTER_EMPTY;
            char letter = '\0';

            if (row < game->current_guess) {
                result = game->guesses[row].results[col];
                letter = game->guesses[row].letters[col];
            }
            else if (row == typing_row && view->typed != NULL &&
                     col < (int)strlen(view->typed)) {
                letter = view->typed[col];
            }

            draw_tile(BOARD_LEFT + col * BOARD_PITCH,
                      BOARD_TOP + row * BOARD_PITCH,
                      BOARD_TILE, result, letter, COLOR_GAME_BG, 6);
        }
    }
}

static void draw_message(const UiView *view)
{
    if (view->message == NULL || view->message[0] == '\0') {
        return;
    }

    int width = text_design_width(view->message, 3) + 40;
    int x = (DESIGN_WIDTH - width) / 2;

    set_color(view->message_is_error ? COLOR_ERROR : COLOR_END_BG);
    fill_rect(x, MESSAGE_Y - 22, width, 44);

    set_color(COLOR_BLACK);
    draw_border(x, MESSAGE_Y - 22, width, 44, 2);

    draw_text(view->message, DESIGN_WIDTH / 2, MESSAGE_Y, 3,
              view->message_is_error ? COLOR_BLACK : COLOR_WHITE);
}

static void draw_keyboard(const Game *game)
{
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        char label[2] = { (char)('A' + i), '\0' };
        int x, y;
        Color fill = COLOR_BUTTON_LIGHT;
        Color text = COLOR_BLACK;

        key_position(i, &x, &y);

        switch (game_keyboard_state(game, label[0])) {
        case LETTER_CORRECT: fill = COLOR_CORRECT;    text = COLOR_WHITE; break;
        case LETTER_PRESENT: fill = COLOR_PRESENT;    text = COLOR_WHITE; break;
        case LETTER_ABSENT:  fill = COLOR_KEY_ABSENT; text = COLOR_WHITE; break;
        case LETTER_EMPTY:   break;
        }

        draw_key(x, y, KEY_WIDTH, KEY_HEIGHT, label, fill, text, 4);
    }

    for (int i = 0; i < COUNT(GAME_BUTTONS); i++) {
        const Button *b = &GAME_BUTTONS[i];
        draw_key(b->x, b->y, b->w, b->h, b->label,
                 COLOR_BUTTON_LIGHT, COLOR_BLACK, b->h < 50 ? 3 : 4);
    }
}

static void draw_playing(const UiView *view)
{
    draw_background(COLOR_FRAME, COLOR_GAME_BG);
    draw_logo(30);

    if (view->game == NULL) {
        return;
    }

    draw_board(view);
    draw_message(view);
    draw_keyboard(view->game);
}

static UiAction hit_playing(double dx, double dy, char *letter)
{
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        int x, y;
        key_position(i, &x, &y);

        if (contains(x, y, KEY_WIDTH, KEY_HEIGHT, dx, dy)) {
            if (letter != NULL) {
                *letter = (char)('A' + i);
            }
            return UI_ACTION_LETTER;
        }
    }

    return hit_buttons(GAME_BUTTONS, COUNT(GAME_BUTTONS), dx, dy);
}


/* ---- result screen (original end page) ----------------------------- */

static void draw_result(const UiView *view)
{
    const Game *game = view->game;

    /* Original end page: teal left frame, orange right frame. */
    gfx_clear_color(COLOR_TEAL.r, COLOR_TEAL.g, COLOR_TEAL.b);
    gfx_clear();
    set_color(COLOR_ORANGE);
    gfx_fillrectangle(gfx_xsize() / 2, 0, gfx_xsize() - gfx_xsize() / 2, gfx_ysize());

    set_color(COLOR_END_BG);
    fill_rect(0, 0, DESIGN_WIDTH, DESIGN_HEIGHT);

    if (game == NULL) {
        return;
    }

    if (game->won) {
        draw_text("YOU WON!", 460, 180, 10, COLOR_TEAL);
    }
    else {
        draw_text("GAME OVER", 460, 180, 10, COLOR_ORANGE);
    }

    draw_text("THE CORRECT WORD WAS:", 460, 310, 4, COLOR_WHITE);

    for (int i = 0; i < WORD_LENGTH; i++) {
        draw_tile(215 + i * 100, 350, 90, LETTER_CORRECT,
                  game->answer[i], COLOR_GAME_BG, 7);
    }

    char summary[48];

    if (game->won) {
        snprintf(summary, sizeof(summary), "SOLVED IN %d/%d GUESSES",
                 game->current_guess, MAX_GUESSES);
    }
    else {
        snprintf(summary, sizeof(summary), "BETTER LUCK NEXT TIME!");
    }

    draw_text(summary, 460, 520, 4, COLOR_WHITE);

    draw_buttons(RESULT_BUTTONS, COUNT(RESULT_BUTTONS), COLOR_BUTTON_FACE);
}


/* ---- goodbye screen (original exit page) --------------------------- */

static void draw_goodbye(void)
{
    draw_background(COLOR_FRAME, COLOR_MENU_BG);
    draw_text("THANK YOU FOR PLAYING WORDLE", 460, 280, 4, COLOR_BLACK);
    draw_text("PLAY AGAIN SOON", 460, 400, 4, COLOR_BLACK);
    draw_buttons(GOODBYE_BUTTONS, COUNT(GOODBYE_BUTTONS), COLOR_BUTTON_FACE);
}


/* ---- public API ---------------------------------------------------- */

/*
 * Window icon: a green "correct" tile with a black border and a
 * white W from the pixel font, on an 18 x 18 grid. The same design
 * is stored as assets/wordle.svg for the application menu.
 */
#define ICON_CELLS 18
#define ICON_SCALE 4
#define ICON_SIZE  (ICON_CELLS * ICON_SCALE)

static unsigned int to_argb(Color c)
{
    return 0xFF000000u |               /* fully opaque */
           ((unsigned int)c.r << 16) |
           ((unsigned int)c.g << 8) |
           (unsigned int)c.b;
}

static void set_window_icon(void)
{
    static unsigned int pixels[ICON_SIZE * ICON_SIZE];

    for (int y = 0; y < ICON_SIZE; y++) {
        for (int x = 0; x < ICON_SIZE; x++) {
            int cx = x / ICON_SCALE;
            int cy = y / ICON_SCALE;
            Color c = COLOR_CORRECT;

            if (cx == 0 || cy == 0 || cx == ICON_CELLS - 1 || cy == ICON_CELLS - 1) {
                c = COLOR_BLACK;
            }
            /* W is 10 x 14 cells (glyph x2), centred: cells 4..13, 2..15. */
            else if (cx >= 4 && cy >= 2 &&
                     font_glyph_pixel('W', (cx - 4) / 2, (cy - 2) / 2)) {
                c = COLOR_WHITE;
            }

            pixels[y * ICON_SIZE + x] = to_argb(c);
        }
    }

    gfx_set_icon(ICON_SIZE, ICON_SIZE, pixels);
}

int ui_init(void)
{
    if (!gfx_open(WINDOW_WIDTH, WINDOW_HEIGHT, "Wordle")) {
        return 0;
    }

    set_window_icon();

    return 1;
}

void ui_shutdown(void)
{
    gfx_close();
}

void ui_draw(const UiView *view)
{
    if (view == NULL) {
        return;
    }

    update_view();

    switch (view->screen) {
    case SCREEN_MENU:        draw_menu(view);      break;
    case SCREEN_HOW_TO_PLAY: draw_how_to_play();   break;
    case SCREEN_PLAYING:     draw_playing(view);   break;
    case SCREEN_RESULT:      draw_result(view);    break;
    case SCREEN_GOODBYE:     draw_goodbye();       break;
    }

    gfx_flush();
}

UiAction ui_hit_test(Screen screen, int x, int y, char *letter)
{
    update_view();

    /* Window pixels -> design units (inverse of px/py). */
    double dx = (x - view_x) / view_scale;
    double dy = (y - view_y) / view_scale;

    switch (screen) {
    case SCREEN_MENU:
        if (hit_buttons(&SOUND_BUTTON, 1, dx, dy) != UI_ACTION_NONE) {
            return UI_ACTION_TOGGLE_SOUND;
        }
        return hit_buttons(MENU_BUTTONS, COUNT(MENU_BUTTONS), dx, dy);
    case SCREEN_HOW_TO_PLAY:
        return hit_buttons(HOW_TO_PLAY_BUTTONS, COUNT(HOW_TO_PLAY_BUTTONS), dx, dy);
    case SCREEN_PLAYING:
        return hit_playing(dx, dy, letter);
    case SCREEN_RESULT:
        return hit_buttons(RESULT_BUTTONS, COUNT(RESULT_BUTTONS), dx, dy);
    case SCREEN_GOODBYE:
        return hit_buttons(GOODBYE_BUTTONS, COUNT(GOODBYE_BUTTONS), dx, dy);
    }

    return UI_ACTION_NONE;
}
