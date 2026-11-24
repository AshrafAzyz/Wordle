#include "game.h"

#include <string.h>


int game_init(Game *game, const char *answer)
{
    if (game == NULL) {
        return 0;
    }

    /*
     * Zero the whole struct: every string becomes "", every
     * counter 0, and every LetterResult LETTER_EMPTY (the first
     * enumerator of an enum is always 0 in C).
     */
    memset(game, 0, sizeof(*game));

    if (!dictionary_normalize_word(answer, game->answer)) {
        game->game_over = 1;
        return 0;
    }

    return 1;
}


void game_evaluate(
    const char *answer,
    const char *guess,
    LetterResult results[WORD_LENGTH]
)
{
    if (answer == NULL || guess == NULL || results == NULL) {
        return;
    }

    /* answer_used[j] = 1 once answer[j] has been matched. */
    int answer_used[WORD_LENGTH] = {0};

    for (int i = 0; i < WORD_LENGTH; i++) {
        results[i] = LETTER_ABSENT;
    }

    /*
     * First pass: exact matches (green).
     *
     * Greens must be claimed before yellows. Otherwise an earlier
     * copy of a letter could "use up" the answer letter that a
     * later, correctly placed copy needs.
     */
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess[i] == answer[i]) {
            results[i] = LETTER_CORRECT;
            answer_used[i] = 1;
        }
    }

    /*
     * Second pass: right letter, wrong place (yellow).
     *
     * Each answer letter can justify at most one yellow, so a
     * guess with two E's against an answer with one E gets at
     * most one coloured E.
     */
    for (int i = 0; i < WORD_LENGTH; i++) {

        if (results[i] == LETTER_CORRECT) {
            continue;
        }

        for (int j = 0; j < WORD_LENGTH; j++) {

            if (!answer_used[j] && guess[i] == answer[j]) {
                results[i] = LETTER_PRESENT;
                answer_used[j] = 1;
                break;
            }
        }
    }
}


GuessStatus game_submit_guess(Game *game, const char *guess)
{
    if (game == NULL) {
        return GUESS_INVALID;
    }

    /* Also guarantees current_guess < MAX_GUESSES below. */
    if (game->game_over) {
        return GUESS_GAME_OVER;
    }

    char word[WORD_LENGTH + 1];

    if (!dictionary_normalize_word(guess, word)) {
        return GUESS_INVALID;
    }

    GuessResult *result = &game->guesses[game->current_guess];

    memcpy(result->letters, word, sizeof(word));
    game_evaluate(game->answer, word, result->results);

    /* Keyboard keeps the most informative state per letter. */
    for (int i = 0; i < WORD_LENGTH; i++) {
        int key = word[i] - 'A';

        if (result->results[i] > game->keyboard[key]) {
            game->keyboard[key] = result->results[i];
        }
    }

    game->current_guess++;

    if (strcmp(word, game->answer) == 0) {
        game->won = 1;
        game->game_over = 1;
    }
    else if (game->current_guess >= MAX_GUESSES) {
        game->game_over = 1;
    }

    return GUESS_ACCEPTED;
}


int game_is_over(const Game *game)
{
    if (game == NULL) {
        return 1;
    }

    return game->game_over;
}


LetterResult game_keyboard_state(const Game *game, char letter)
{
    if (game == NULL) {
        return LETTER_EMPTY;
    }

    if (letter >= 'a' && letter <= 'z') {
        letter = (char)(letter - 'a' + 'A');
    }

    if (letter < 'A' || letter > 'Z') {
        return LETTER_EMPTY;
    }

    return game->keyboard[letter - 'A'];
}
