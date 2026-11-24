#ifndef UI_H
#define UI_H

#include "game.h"

/*
 * User interface: draws every screen and maps mouse clicks to
 * actions. It never changes game state; main.c does that.
 */

typedef enum {
    SCREEN_MENU,
    SCREEN_HOW_TO_PLAY,
    SCREEN_PLAYING,
    SCREEN_RESULT,
    SCREEN_GOODBYE
} Screen;

/* What a click on the current screen means. */
typedef enum {
    UI_ACTION_NONE,
    UI_ACTION_NEW_GAME,
    UI_ACTION_HOW_TO_PLAY,
    UI_ACTION_EXIT,         /* menu "EXIT": go to the goodbye screen */
    UI_ACTION_BACK,
    UI_ACTION_PLAY_AGAIN,
    UI_ACTION_MAIN_MENU,
    UI_ACTION_QUIT,         /* goodbye "EXIT": close the program     */
    UI_ACTION_LETTER,
    UI_ACTION_ENTER,
    UI_ACTION_DELETE,
    UI_ACTION_CLEAR
} UiAction;

/* Everything needed to draw one frame. */
typedef struct {
    Screen screen;
    const Game *game;        /* SCREEN_PLAYING and SCREEN_RESULT     */
    const char *typed;       /* letters typed in the current row     */
    const char *message;     /* status line under the board, or NULL */
    int message_is_error;
} UiView;

/* Open the window. Returns 1 on success, 0 if no display is available. */
int ui_init(void);

/* Close the window and free graphics resources. */
void ui_shutdown(void);

/* Draw the whole screen described by `view`. */
void ui_draw(const UiView *view);

/*
 * Which action is at window position (x, y) on `screen`?
 * For UI_ACTION_LETTER, the letter is stored in *letter.
 */
UiAction ui_hit_test(Screen screen, int x, int y, char *letter);

#endif
