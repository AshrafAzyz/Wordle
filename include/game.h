#ifndef GAME_H
#define GAME_H

#include "dictionary.h"   /* WORD_LENGTH, dictionary_normalize_word */

#define MAX_GUESSES   6
#define ALPHABET_SIZE 26

/*
 * State of one tile or one keyboard key.
 *
 * The order matters: a higher value carries more information.
 * The on-screen keyboard keeps the highest state seen for each
 * letter, so a green key never turns back to yellow or grey.
 */
typedef enum {
    LETTER_EMPTY,     /* nothing known yet (empty tile / unused key) */
    LETTER_ABSENT,    /* grey:   not in the answer                   */
    LETTER_PRESENT,   /* yellow: in the answer, wrong position       */
    LETTER_CORRECT    /* green:  right letter, right position        */
} LetterResult;

/* Outcome of game_submit_guess(). */
typedef enum {
    GUESS_ACCEPTED,   /* guess was scored and recorded               */
    GUESS_INVALID,    /* not exactly 5 letters A-Z (or NULL)         */
    GUESS_GAME_OVER   /* the game has already been won or lost       */
} GuessStatus;

typedef struct {
    char letters[WORD_LENGTH + 1];   /* uppercase, '\0'-terminated */
    LetterResult results[WORD_LENGTH];
} GuessResult;

typedef struct {
    char answer[WORD_LENGTH + 1];    /* own copy, uppercase */
    int current_guess;               /* guesses used: 0..MAX_GUESSES */
    GuessResult guesses[MAX_GUESSES];
    LetterResult keyboard[ALPHABET_SIZE];   /* index 0 = 'A' */
    int game_over;
    int won;
} Game;

/*
 * Start a new game. The answer is validated and copied, so the
 * caller's string does not need to outlive the game.
 * Returns 1 on success, 0 if the answer is not a valid word (the
 * game is then marked as over so it cannot be played).
 */
int game_init(Game *game, const char *answer);

/*
 * Score `guess` against `answer` (both uppercase, WORD_LENGTH
 * letters) using Wordle rules, including repeated letters.
 */
void game_evaluate(
    const char *answer,
    const char *guess,
    LetterResult results[WORD_LENGTH]
);

/*
 * Submit a guess. Lowercase is accepted and converted.
 * Dictionary membership is checked by the caller.
 */
GuessStatus game_submit_guess(Game *game, const char *guess);

int game_is_over(const Game *game);

/* Best-known state of a letter for the on-screen keyboard. */
LetterResult game_keyboard_state(const Game *game, char letter);

#endif
