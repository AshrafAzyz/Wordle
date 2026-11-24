/*
 * Unit tests for the game engine (src/game.c).
 *
 * [Rn] marks which numbered requirement from the project brief a
 * test covers. Run with: make test
 */
#include "game.h"
#include "test_framework.h"

#include <stdlib.h>
#include <string.h>


/*
 * Turn a result array into a short pattern string so tests read
 * like the game: G = green, Y = yellow, _ = grey, . = empty.
 */
static const char *pattern(const LetterResult results[WORD_LENGTH])
{
    static char text[WORD_LENGTH + 1];

    for (int i = 0; i < WORD_LENGTH; i++) {
        switch (results[i]) {
        case LETTER_CORRECT: text[i] = 'G'; break;
        case LETTER_PRESENT: text[i] = 'Y'; break;
        case LETTER_ABSENT:  text[i] = '_'; break;
        default:             text[i] = '.'; break;
        }
    }

    text[WORD_LENGTH] = '\0';

    return text;
}

/* Score one guess with game_evaluate() and return its pattern. */
static const char *score(const char *answer, const char *guess)
{
    LetterResult results[WORD_LENGTH];

    game_evaluate(answer, guess, results);

    return pattern(results);
}


/* ---- initialisation ------------------------------------------------ */

static void test_init_starts_empty_game(void)
{
    Game game;

    CHECK(game_init(&game, "CRANE") == 1);
    CHECK_STR_EQ(game.answer, "CRANE");
    CHECK(game.current_guess == 0);
    CHECK(!game.won);
    CHECK(!game_is_over(&game));
    CHECK(game_keyboard_state(&game, 'A') == LETTER_EMPTY);
}

static void test_init_copies_and_normalises_answer(void)
{
    Game game;
    char answer[] = "plant";

    CHECK(game_init(&game, answer) == 1);

    /* Changing the caller's buffer must not affect the game. */
    strcpy(answer, "XXXXX");
    CHECK_STR_EQ(game.answer, "PLANT");
}

static void test_init_rejects_invalid_answer(void)
{
    Game game;

    CHECK(game_init(&game, "CRAN") == 0);
    CHECK(game_is_over(&game));
    CHECK(game_submit_guess(&game, "CRANE") == GUESS_GAME_OVER);

    CHECK(game_init(&game, NULL) == 0);
    CHECK(game_init(NULL, "CRANE") == 0);
}


/* ---- scoring ------------------------------------------------------- */

/* [R1] */
static void test_correct_guess_is_all_green(void)
{
    Game game;
    game_init(&game, "CRANE");

    CHECK(game_submit_guess(&game, "CRANE") == GUESS_ACCEPTED);
    CHECK_STR_EQ(pattern(game.guesses[0].results), "GGGGG");
    CHECK_STR_EQ(game.guesses[0].letters, "CRANE");
}

/* [R2] */
static void test_incorrect_guess_is_all_grey(void)
{
    Game game;
    game_init(&game, "CRANE");

    CHECK(game_submit_guess(&game, "PILOT") == GUESS_ACCEPTED);
    CHECK_STR_EQ(pattern(game.guesses[0].results), "_____");
    CHECK(!game.won);
    CHECK(!game_is_over(&game));
    CHECK(game.current_guess == 1);
}

/* [R3] */
static void test_partially_correct_guess(void)
{
    /* T absent, R A E in place, C in the answer but misplaced. */
    CHECK_STR_EQ(score("CRANE", "TRACE"), "_GGYG");
    CHECK_STR_EQ(score("CRANE", "NACRE"), "YYYYG");
    CHECK_STR_EQ(score("CRANE", "CLOTH"), "G____");
}

/* [R4] */
static void test_duplicate_letters(void)
{
    /* Guess has two E's, answer has one: only one may be coloured. */
    CHECK_STR_EQ(score("CRANE", "EERIE"), "__Y_G");

    /* Green claims its letter first, so the earlier copy is grey. */
    CHECK_STR_EQ(score("CRANE", "GEESE"), "____G");

    /* Answer has two B's: both B's in the guess are coloured. */
    CHECK_STR_EQ(score("ABBEY", "BABES"), "YYGG_");

    /* Answer has two O's, guess has two O's in different spots. */
    CHECK_STR_EQ(score("ROBOT", "FLOOR"), "__YGY");

    /* Answer has two E's, guess has three. */
    CHECK_STR_EQ(score("SPEED", "EERIE"), "YY___");

    /* Repeated letters, everything correct. */
    CHECK_STR_EQ(score("SPEED", "SPEED"), "GGGGG");
}


/* ---- input validation ---------------------------------------------- */

/* [R6] */
static void test_empty_input_is_rejected(void)
{
    Game game;
    game_init(&game, "CRANE");

    CHECK(game_submit_guess(&game, "") == GUESS_INVALID);
    CHECK(game_submit_guess(&game, NULL) == GUESS_INVALID);
    CHECK(game.current_guess == 0);   /* no turn used */
}

/* [R7] */
static void test_wrong_length_is_rejected(void)
{
    Game game;
    game_init(&game, "CRANE");

    CHECK(game_submit_guess(&game, "C") == GUESS_INVALID);
    CHECK(game_submit_guess(&game, "CRAN") == GUESS_INVALID);
    CHECK(game_submit_guess(&game, "CRANES") == GUESS_INVALID);
    CHECK(game.current_guess == 0);
}

static void test_non_letters_are_rejected(void)
{
    Game game;
    game_init(&game, "CRANE");

    CHECK(game_submit_guess(&game, "12345") == GUESS_INVALID);
    CHECK(game_submit_guess(&game, "CR NE") == GUESS_INVALID);
    CHECK(game_submit_guess(&game, "CRAN!") == GUESS_INVALID);
    CHECK(game.current_guess == 0);
}

/* [R8] */
static void test_lowercase_input_is_accepted(void)
{
    Game game;
    game_init(&game, "CRANE");

    CHECK(game_submit_guess(&game, "trace") == GUESS_ACCEPTED);
    CHECK_STR_EQ(game.guesses[0].letters, "TRACE");
    CHECK_STR_EQ(pattern(game.guesses[0].results), "_GGYG");

    CHECK(game_submit_guess(&game, "cRaNe") == GUESS_ACCEPTED);
    CHECK(game.won);
}


/* ---- turns, win and loss ------------------------------------------- */

static const char *const WRONG_GUESSES[MAX_GUESSES] = {
    "PILOT", "MOUTH", "BUILT", "JUMPY", "WORLD", "STICK"
};

/* [R9] */
static void test_six_guesses_are_counted(void)
{
    Game game;
    game_init(&game, "CRANE");

    for (int i = 0; i < MAX_GUESSES; i++) {
        CHECK(game.current_guess == i);
        CHECK(game_submit_guess(&game, WRONG_GUESSES[i]) == GUESS_ACCEPTED);
        CHECK_STR_EQ(game.guesses[i].letters, WRONG_GUESSES[i]);
    }

    CHECK(game.current_guess == MAX_GUESSES);

    /* A seventh guess is refused and does not change the count. */
    CHECK(game_submit_guess(&game, "CRANE") == GUESS_GAME_OVER);
    CHECK(game.current_guess == MAX_GUESSES);
}

static void test_invalid_guess_does_not_use_a_turn(void)
{
    Game game;
    game_init(&game, "CRANE");

    for (int i = 0; i < 10; i++) {
        game_submit_guess(&game, "BAD");
    }

    CHECK(game.current_guess == 0);
    CHECK(!game_is_over(&game));
}

/* [R10] */
static void test_win_ends_the_game(void)
{
    Game game;
    game_init(&game, "CRANE");

    game_submit_guess(&game, "PILOT");
    game_submit_guess(&game, "TRACE");
    CHECK(!game_is_over(&game));

    CHECK(game_submit_guess(&game, "CRANE") == GUESS_ACCEPTED);
    CHECK(game.won);
    CHECK(game_is_over(&game));
    CHECK(game.current_guess == 3);

    CHECK(game_submit_guess(&game, "PILOT") == GUESS_GAME_OVER);
    CHECK(game.current_guess == 3);
}

static void test_win_on_last_guess_is_a_win(void)
{
    Game game;
    game_init(&game, "CRANE");

    for (int i = 0; i < MAX_GUESSES - 1; i++) {
        game_submit_guess(&game, WRONG_GUESSES[i]);
    }

    CHECK(!game_is_over(&game));
    CHECK(game_submit_guess(&game, "CRANE") == GUESS_ACCEPTED);
    CHECK(game.won);
    CHECK(game_is_over(&game));
}

/* [R11] */
static void test_six_wrong_guesses_is_a_loss(void)
{
    Game game;
    game_init(&game, "CRANE");

    for (int i = 0; i < MAX_GUESSES; i++) {
        CHECK(!game_is_over(&game));
        game_submit_guess(&game, WRONG_GUESSES[i]);
    }

    CHECK(game_is_over(&game));
    CHECK(!game.won);
    CHECK_STR_EQ(game.answer, "CRANE");   /* shown on the loss screen */
}


/* ---- on-screen keyboard state -------------------------------------- */

static void test_keyboard_keeps_best_state(void)
{
    Game game;
    game_init(&game, "CRANE");

    game_submit_guess(&game, "NACRE");          /* C is yellow */
    CHECK(game_keyboard_state(&game, 'C') == LETTER_PRESENT);

    game_submit_guess(&game, "CRIMP");          /* C is green  */
    CHECK(game_keyboard_state(&game, 'C') == LETTER_CORRECT);

    game_submit_guess(&game, "MIMIC");          /* C is only yellow here */
    CHECK(game_keyboard_state(&game, 'C') == LETTER_CORRECT);

    CHECK(game_keyboard_state(&game, 'M') == LETTER_ABSENT);
    CHECK(game_keyboard_state(&game, 'm') == LETTER_ABSENT);
    CHECK(game_keyboard_state(&game, 'Z') == LETTER_EMPTY);
    CHECK(game_keyboard_state(&game, '?') == LETTER_EMPTY);
    CHECK(game_keyboard_state(NULL, 'A') == LETTER_EMPTY);
}


/* ---- stress tests -------------------------------------------------- */

/*
 * An independent scorer using a different algorithm (letter counts
 * instead of "used" flags). If two different methods agree on every
 * input, a shared mistake is very unlikely.
 */
static void reference_score(const char *answer, const char *guess,
                            LetterResult out[WORD_LENGTH])
{
    int unmatched[ALPHABET_SIZE] = {0};

    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess[i] == answer[i]) {
            out[i] = LETTER_CORRECT;
        }
        else {
            out[i] = LETTER_ABSENT;
            unmatched[answer[i] - 'A']++;
        }
    }

    for (int i = 0; i < WORD_LENGTH; i++) {
        if (out[i] != LETTER_CORRECT && unmatched[guess[i] - 'A'] > 0) {
            out[i] = LETTER_PRESENT;
            unmatched[guess[i] - 'A']--;
        }
    }
}

/* Write the n-th word over a small alphabet (base-k counting). */
static void nth_word(int n, const char *alphabet, int k, char word[WORD_LENGTH + 1])
{
    for (int i = WORD_LENGTH - 1; i >= 0; i--) {
        word[i] = alphabet[n % k];
        n /= k;
    }
    word[WORD_LENGTH] = '\0';
}

/* [R4] every answer/guess pair over {A,B,C}: 243 x 243 = 59,049 */
static void test_evaluate_matches_reference_exhaustively(void)
{
    const int words = 3 * 3 * 3 * 3 * 3;
    int mismatches = 0;

    for (int a = 0; a < words; a++) {
        char answer[WORD_LENGTH + 1];
        nth_word(a, "ABC", 3, answer);

        for (int g = 0; g < words; g++) {
            char guess[WORD_LENGTH + 1];
            LetterResult expected[WORD_LENGTH];
            LetterResult actual[WORD_LENGTH];

            nth_word(g, "ABC", 3, guess);
            reference_score(answer, guess, expected);
            game_evaluate(answer, guess, actual);

            if (memcmp(expected, actual, sizeof(actual)) != 0) {
                if (mismatches < 5) {   /* show the first few only */
                    printf("    answer %s guess %s: expected %s",
                           answer, guess, pattern(expected));
                    printf(", got %s\n", pattern(actual));
                }
                mismatches++;
            }
        }
    }

    CHECK(mismatches == 0);
}

/* [R9][R10][R11] 1,000 random games; rules that must always hold. */
static void test_random_games_keep_invariants(void)
{
    const char *alphabet = "ABCDEF";   /* small: lots of repeated letters */
    int violations = 0;

    srand(2024);   /* fixed seed: the same games every run */

    for (int n = 0; n < 1000; n++) {
        Game game;
        char answer[WORD_LENGTH + 1];

        nth_word(rand() % 7776, alphabet, 6, answer);   /* 6^5 words */
        game_init(&game, answer);

        int turns = 0;

        while (!game_is_over(&game)) {
            char guess[WORD_LENGTH + 1];

            /* Usually a random word; sometimes the answer itself. */
            if (rand() % 8 == 0) {
                strcpy(guess, answer);
            }
            else {
                nth_word(rand() % 7776, alphabet, 6, guess);
            }

            if (game_submit_guess(&game, guess) != GUESS_ACCEPTED) {
                violations++;
                break;
            }

            turns++;

            const LetterResult *r = game.guesses[game.current_guess - 1].results;
            int all_green = 1;
            for (int i = 0; i < WORD_LENGTH; i++) {
                if (r[i] != LETTER_CORRECT) {
                    all_green = 0;
                }
            }

            if (game.current_guess != turns || turns > MAX_GUESSES) violations++;
            if (game.won != all_green)                               violations++;
            if (game_is_over(&game) && !game.won && turns != MAX_GUESSES) violations++;

            /* A guessed letter that is in the answer is never grey. */
            for (int i = 0; i < WORD_LENGTH; i++) {
                int in_answer = strchr(answer, guess[i]) != NULL;
                LetterResult key = game_keyboard_state(&game, guess[i]);

                if (in_answer && key < LETTER_PRESENT)  violations++;
                if (!in_answer && key != LETTER_ABSENT) violations++;
            }
        }

        /* Nothing is accepted after the end. */
        if (game_submit_guess(&game, answer) != GUESS_GAME_OVER) {
            violations++;
        }
    }

    CHECK(violations == 0);
}


int main(void)
{
    printf("=== game engine tests ===\n");

    RUN_TEST(test_init_starts_empty_game);
    RUN_TEST(test_init_copies_and_normalises_answer);
    RUN_TEST(test_init_rejects_invalid_answer);
    RUN_TEST(test_correct_guess_is_all_green);
    RUN_TEST(test_incorrect_guess_is_all_grey);
    RUN_TEST(test_partially_correct_guess);
    RUN_TEST(test_duplicate_letters);
    RUN_TEST(test_empty_input_is_rejected);
    RUN_TEST(test_wrong_length_is_rejected);
    RUN_TEST(test_non_letters_are_rejected);
    RUN_TEST(test_lowercase_input_is_accepted);
    RUN_TEST(test_six_guesses_are_counted);
    RUN_TEST(test_invalid_guess_does_not_use_a_turn);
    RUN_TEST(test_win_ends_the_game);
    RUN_TEST(test_win_on_last_guess_is_a_win);
    RUN_TEST(test_six_wrong_guesses_is_a_loss);
    RUN_TEST(test_keyboard_keeps_best_state);
    RUN_TEST(test_evaluate_matches_reference_exhaustively);
    RUN_TEST(test_random_games_keep_invariants);

    return TEST_SUMMARY();
}
