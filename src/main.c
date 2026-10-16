#include "dictionary.h"
#include "game.h"
#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(void)
{
    int word_count = 0;

    /*
     * Load the dictionary.
     */
    char **words = dictionary_load(
        "data/words.txt",
        &word_count
    );

    if (words == NULL || word_count == 0) {
        printf("Failed to load dictionary.\n");
        return 1;
    }

    printf("Dictionary loaded: %d words\n", word_count);

    /*
     * Seed the random number generator.
     */
    srand((unsigned int)time(NULL));

    /*
     * Select the answer.
     */
    const char *answer =
        dictionary_pick_random(words, word_count);

    if (answer == NULL) {
        printf("Failed to select a word.\n");
        dictionary_free(words, word_count);
        return 1;
    }

    /*
     * Initialize the game.
     */
    Game game;

    game_init(&game, answer);

    printf("\n");
    printf("=== WORDLE ===\n");
    printf("Guess the 5-letter word.\n");
    printf("You have %d attempts.\n\n", MAX_GUESSES);

    /*
     * Main game loop.
     */
    while (!game_is_over(&game)) {

        char guess[INPUT_BUFFER_SIZE];

        if (!input_get_guess(
                guess,
                sizeof(guess)
            )) {

            printf("Failed to read input.\n");
            break;
        }

        /*
         * Check that the guess has exactly
         * five letters.
         */
        if (strlen(guess) != WORD_LENGTH) {
            printf(
                "Guess must contain exactly %d letters.\n\n",
                WORD_LENGTH
            );

            continue;
        }

        /*
         * Check whether the word exists
         * in our dictionary.
         */
        if (!dictionary_contains(
                words,
                word_count,
                guess
            )) {

            printf("Word is not in the dictionary.\n\n");
            continue;
        }

        /*
         * Submit the guess to the game engine.
         */
        if (!game_submit_guess(&game, guess)) {
            printf("Could not submit guess.\n");
            continue;
        }

        /*
         * Display the result.
         */
        GuessResult *result =
            &game.guesses[game.current_guess - 1];

        for (int i = 0; i < WORD_LENGTH; i++) {

            printf("%c", result->letters[i]);

            if (result->results[i] == LETTER_CORRECT) {
                printf("!");
            }
            else if (result->results[i] == LETTER_PRESENT) {
                printf("?");
            }
            else {
                printf("X");
            }

            printf(" ");
        }

        printf("\n\n");
    }

    /*
     * Display the final result.
     */
    if (game.won) {
        printf("🎉 You won!\n");
        printf(
            "You solved it in %d guesses.\n",
            game.current_guess
        );
    }
    else {
        printf("Game over!\n");
        printf("The answer was: %s\n", answer);
    }

    /*
     * Free dictionary memory.
     */
    dictionary_free(words, word_count);

    return 0;
}
