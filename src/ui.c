#include "ui.h"

#include <stdio.h>


void ui_init(void)
{
    /*
     * UI initialization will be added here
     * when we connect the graphical interface.
     */
}


void ui_display_title(void)
{
    printf("\n");
    printf("=== WORDLE ===\n");
}


void ui_display_instructions(void)
{
    printf("Guess the 5-letter word.\n");
    printf("You have %d attempts.\n\n", MAX_GUESSES);
}


void ui_display_guess(const GuessResult *result)
{
    if (result == NULL) {
        return;
    }

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


void ui_display_win(int guesses)
{
    printf("You won!\n");
    printf("You solved it in %d guesses.\n", guesses);
}


void ui_display_game_over(const char *answer)
{
    printf("Game over!\n");

    if (answer != NULL) {
        printf("The answer was: %s\n", answer);
    }
}
