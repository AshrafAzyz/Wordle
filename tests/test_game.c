#include "../src/game.h"

#include <stdio.h>


void print_result(const GuessResult *result)
{
    for (int i = 0; i < WORD_LENGTH; i++) {

        printf("%c:", result->letters[i]);

        if (result->results[i] == LETTER_CORRECT) {
            printf("CORRECT ");
        }
        else if (result->results[i] == LETTER_PRESENT) {
            printf("PRESENT ");
        }
        else {
            printf("ABSENT ");
        }
    }

    printf("\n");
}


int main(void)
{
    Game game;

    game_init(&game, "CRANE");

    printf("=== Six Guess Test ===\n\n");

    const char *guesses[MAX_GUESSES] = {
        "XXXXX",
        "BBBBB",
        "CCCCC",
        "DDDDD",
        "EEEEE",
        "FFFFF"
    };

    for (int i = 0; i < MAX_GUESSES; i++) {

        printf(
            "Guess %d: %s\n",
            i + 1,
            guesses[i]
        );

        int success =
            game_submit_guess(&game, guesses[i]);

        printf(
            "Submitted: %s\n",
            success ? "YES" : "NO"
        );

        print_result(
            &game.guesses[i]
        );

        printf(
            "Game over: %s\n\n",
            game_is_over(&game)
                ? "YES"
                : "NO"
        );
    }

    printf("=== Seventh Guess Test ===\n\n");

    int success =
        game_submit_guess(&game, "CRANE");

    printf(
        "Seventh guess accepted: %s\n",
        success ? "YES" : "NO"
    );

    return 0;
}
