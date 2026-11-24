#ifndef UI_H
#define UI_H

#include "game.h"

void ui_init(void);

void ui_draw_game(
    const Game *game
);

void ui_display_title(void);

void ui_display_instructions(void);

void ui_display_win(
    int guesses
);

void ui_display_game_over(
    const char *answer
);

void ui_draw_current_guess(
    const Game *game,
    const char *guess
);

void ui_shutdown(void);

#endif
