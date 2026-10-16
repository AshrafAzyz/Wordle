#ifndef UI_H
#define UI_H

#include "game.h"

void ui_init(void);

void ui_display_title(void);

void ui_display_instructions(void);

void ui_display_guess(
    const GuessResult *result
);

void ui_display_win(
    int guesses
);

void ui_display_game_over(
    const char *answer
);

#endif
