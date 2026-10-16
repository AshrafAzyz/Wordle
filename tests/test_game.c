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


void test_guess(
    const char *answer,
    const char *guess
)
{
    Game game;

    game_init(&game, answer);

    int success =
        game_submit_guess(&game, guess);

    printf("Answer: %s\n", answer);
    printf("Guess:  %s\n", guess);

    if (!success) {
        printf("Submission failed!\n\n");
        return;
    }

    print_result(&game.guesses[0]);

    printf(
        "Won: %s\n\n",
        game.won ? "YES" : "NO"
    );
}


int main(void)
{
    printf("=== Basic Tests ===\n\n");

    test_guess("CRANE", "CRANE");
    test_guess("CRANE", "XXXXX");
    test_guess("CRANE", "CROWN");

    printf("=== Duplicate Letter Tests ===\n\n");

    test_guess("APPLE", "ALLEY");
    test_guess("APPLE", "AAAAA");

    return 0;
}
