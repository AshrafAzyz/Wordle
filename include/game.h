#ifndef GAME_H
#define GAME_H

#include "dictionary.h"

#define MAX_GUESSES 6

typedef enum {
    LETTER_ABSENT,
    LETTER_PRESENT,
    LETTER_CORRECT
} LetterResult;

typedef struct {
    char letters[WORD_LENGTH];
    LetterResult results[WORD_LENGTH];
} GuessResult;

typedef struct {
    const char *answer;
    int current_guess;
    GuessResult guesses[MAX_GUESSES];
    int game_over;
    int won;
} Game;

void game_init(Game *game, const char *answer);

int game_submit_guess(
    Game *game,
    const char *guess
);

int game_is_over(const Game *game);

#endif
