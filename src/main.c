#include "dictionary.h"
#include "game.h"
#include "input.h"
#include "ui.h"
#include "gfx.h"

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

    /*
     * Seed random number generation.
     */
    srand((unsigned int)time(NULL));

    /*
     * Select the answer.
     */
    const char *answer =
        dictionary_pick_random(
            words,
            word_count
        );

    if (answer == NULL) {
        printf("Failed to select a word.\n");

        dictionary_free(
            words,
            word_count
        );

        return 1;
    }

    /*
     * Initialize the game.
     */
    Game game;

    game_init(
        &game,
        answer
    );

    /*
     * Initialize systems.
     */
    input_init();
    ui_init();

    /*
     * Current unsubmitted guess.
     */
    char current_guess[WORD_LENGTH + 1];

    memset(
        current_guess,
        0,
        sizeof(current_guess)
    );

    int current_length = 0;

    /*
     * Draw initial game state.
     */
    ui_draw_game(&game);

    /*
     * Main game loop.
     */
    while (!game_is_over(&game)) {

        InputEvent event =
            input_wait_event();

        /*
         * User typed a letter.
         */
        if (event.type == INPUT_LETTER) {

            if (current_length < WORD_LENGTH) {

                current_guess[current_length] =
                    event.letter;

                current_length++;

                current_guess[current_length] =
                    '\0';
            }
        }

        /*
         * User pressed backspace.
         */
        else if (event.type == INPUT_BACKSPACE) {

            if (current_length > 0) {

                current_length--;

                current_guess[current_length] =
                    '\0';
            }
        }

        /*
         * User pressed ESC.
         */
        else if (event.type == INPUT_ESCAPE) {

            current_length = 0;

            current_guess[0] = '\0';
        }

        /*
         * User pressed F11.
         *
         * The window size changed, so
         * redraw the entire interface.
         */
        else if (event.type == INPUT_FULLSCREEN) {

            /*
             * The window has now finished resizing.
             *
             * Clear the new window dimensions.
             */
            gfx_clear();

            /*
             * Redraw the complete interface.
             */
            ui_display_title();

            ui_display_instructions();

            ui_draw_game(&game);

            ui_draw_current_guess(
                &game,
                current_guess
            );

            continue;
        }

        /*
         * User pressed ENTER.
         */
        else if (event.type == INPUT_ENTER) {

            /*
             * Only submit complete guesses.
             */
            if (current_length != WORD_LENGTH) {

                continue;
            }

            /*
             * Check dictionary.
             */
            if (!dictionary_contains(
                    words,
                    word_count,
                    current_guess
                )) {

                /*
                 * For now, simply ignore
                 * invalid dictionary words.
                 */
                continue;
            }

            /*
             * Submit guess.
             */
            if (!game_submit_guess(
                    &game,
                    current_guess
                )) {

                continue;
            }

            /*
             * Clear current input.
             */
            current_length = 0;

            current_guess[0] = '\0';
        }

        /*
         * Redraw the board after every event.
         */
        ui_draw_game(&game);

        /*
         * Draw the letters currently being typed.
         */
        ui_draw_current_guess(
            &game,
            current_guess
        );
    }

    /*
     * Display final result.
     */
    if (game.won) {

        ui_display_win(
            game.current_guess
        );
    }
    else {

        ui_display_game_over(
            answer
        );
    }

    /*
     * Free dictionary memory.
     */
    dictionary_free(
        words,
        word_count
    );

    return 0;
}
