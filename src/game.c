#include "game.h"

#include <string.h>


void game_init(Game *game, const char *answer)
{
    if (game == NULL) {
        return;
    }

    game->answer = answer;
    game->current_guess = 0;
    game->game_over = 0;
    game->won = 0;

    for (int i = 0; i < MAX_GUESSES; i++) {
        for (int j = 0; j < WORD_LENGTH; j++) {
            game->guesses[i].letters[j] = '\0';
            game->guesses[i].results[j] = LETTER_ABSENT;
        }
    }
}


int game_submit_guess(Game *game, const char *guess)
{
    if (game == NULL || guess == NULL) {
        return 0;
    }

    if (game->game_over) {
        return 0;
    }

    if (strlen(guess) != WORD_LENGTH) {
        return 0;
    }

    GuessResult *result =
        &game->guesses[game->current_guess];

    for (int i = 0; i < WORD_LENGTH; i++) {
        result->letters[i] = guess[i];
        result->results[i] = LETTER_ABSENT;
    }

    /*
     * First pass:
     * Find letters in the correct position.
     */
    int answer_used[WORD_LENGTH] = {0};

    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess[i] == game->answer[i]) {
            result->results[i] = LETTER_CORRECT;
            answer_used[i] = 1;
        }
    }

    /*
     * Second pass:
     * Find letters that exist in the answer
     * but are in the wrong position.
     */
    for (int i = 0; i < WORD_LENGTH; i++) {

        if (result->results[i] == LETTER_CORRECT) {
            continue;
        }

        for (int j = 0; j < WORD_LENGTH; j++) {

            if (answer_used[j]) {
                continue;
            }

            if (guess[i] == game->answer[j]) {
                result->results[i] = LETTER_PRESENT;
                answer_used[j] = 1;
                break;
            }
        }
    }

    /*
     * Check whether the player guessed the answer.
     */
    int correct = 1;

    for (int i = 0; i < WORD_LENGTH; i++) {
        if (result->results[i] != LETTER_CORRECT) {
            correct = 0;
            break;
        }
    }

    if (correct) {
        game->won = 1;
        game->game_over = 1;
    }

    game->current_guess++;

    if (game->current_guess >= MAX_GUESSES && !game->won) {
        game->game_over = 1;
    }

    return 1;
}


int game_is_over(const Game *game)
{
    if (game == NULL) {
        return 1;
    }

    return game->game_over;
}
