/*
 * Unit tests for the dictionary module (src/dictionary.c).
 *
 * Run from the project root (make test does this), because the
 * paths below are relative to it.
 *
 * [Rn] marks which numbered requirement from the project brief a
 * test covers.
 */

/*
 * Ask for POSIX functions (mkstemp, fdopen, unlink) on top of
 * standard C11. Must come before any #include to take effect.
 */
#define _POSIX_C_SOURCE 200809L

#include "dictionary.h"
#include "test_framework.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define REAL_DICTIONARY "data/words.txt"     /* allowed guesses */
#define REAL_ANSWERS    "data/answers.txt"   /* possible answers */
#define FIXTURE(name)   ("tests/data/" name)


/* ---- word validation ----------------------------------------------- */

static void test_normalize_accepts_five_letters(void)
{
    char out[WORD_LENGTH + 1];

    CHECK(dictionary_normalize_word("CRANE", out) == 1);
    CHECK_STR_EQ(out, "CRANE");

    /* [R8] lowercase and mixed case become uppercase */
    CHECK(dictionary_normalize_word("crane", out) == 1);
    CHECK_STR_EQ(out, "CRANE");
    CHECK(dictionary_normalize_word("cRaNe", out) == 1);
    CHECK_STR_EQ(out, "CRANE");
}

static void test_normalize_rejects_bad_words(void)
{
    char out[WORD_LENGTH + 1];

    CHECK(dictionary_normalize_word("", out) == 0);        /* [R6] */
    CHECK(dictionary_normalize_word("CRAN", out) == 0);    /* [R7] */
    CHECK(dictionary_normalize_word("CRANES", out) == 0);  /* [R7] */
    CHECK(dictionary_normalize_word("CR4NE", out) == 0);
    CHECK(dictionary_normalize_word("CRAN ", out) == 0);
    CHECK(dictionary_normalize_word("CRANE\n", out) == 0);
    CHECK(dictionary_normalize_word(NULL, out) == 0);
    CHECK(dictionary_normalize_word("CRANE", NULL) == 0);
}


/* ---- loading the real word list ------------------------------------ */

/* Every word valid, uppercase, unique and in sorted order. */
static int list_is_clean(char **words, int count)
{
    for (int i = 0; i < count; i++) {
        char out[WORD_LENGTH + 1];

        if (!dictionary_normalize_word(words[i], out) ||
            strcmp(out, words[i]) != 0) {
            return 0;
        }
        if (i > 0 && strcmp(words[i - 1], words[i]) >= 0) {
            return 0;   /* duplicate or out of order */
        }
    }

    return 1;
}

static void test_load_real_dictionary(void)
{
    int count = 0;
    char **words = dictionary_load(REAL_DICTIONARY, &count);

    CHECK(words != NULL);
    /* The original 200 words plus 5,000 from SCOWL. */
    CHECK(count == 5200);

    if (words != NULL && count > 0) {
        CHECK_STR_EQ(words[0], "AALII");
        CHECK_STR_EQ(words[count - 1], "ZORCH");
        CHECK(list_is_clean(words, count));

        /* The original 200 words are all still accepted. */
        CHECK(dictionary_contains(words, count, "ADMIT"));
        CHECK(dictionary_contains(words, count, "ZESTY"));
    }

    dictionary_free(words, count);
}

static void test_real_answers_are_allowed_guesses(void)
{
    int word_count = 0;
    int answer_count = 0;
    char **words = dictionary_load(REAL_DICTIONARY, &word_count);
    char **answers = dictionary_load(REAL_ANSWERS, &answer_count);

    CHECK(answers != NULL);
    CHECK(answer_count == 2176);

    if (words != NULL && answers != NULL) {
        CHECK(list_is_clean(answers, answer_count));

        /*
         * If an answer could not be typed as a guess, that game would
         * be impossible to win, so every answer must be in words.txt.
         */
        int missing = 0;
        for (int i = 0; i < answer_count; i++) {
            if (!dictionary_contains(words, word_count, answers[i])) {
                if (missing < 5) {
                    printf("    answer %s is not an allowed guess\n", answers[i]);
                }
                missing++;
            }
        }
        CHECK(missing == 0);
    }

    dictionary_free(words, word_count);
    dictionary_free(answers, answer_count);
}

static void test_contains_is_case_insensitive(void)
{
    int count = 0;
    char **words = dictionary_load(FIXTURE("three_words.txt"), &count);

    CHECK(count == 3);
    CHECK(dictionary_contains(words, count, "BERRY"));
    CHECK(dictionary_contains(words, count, "berry"));   /* [R8] */
    CHECK(dictionary_contains(words, count, "BeRrY"));

    dictionary_free(words, count);
}

/* [R5] */
static void test_contains_rejects_unknown_words(void)
{
    int count = 0;
    char **words = dictionary_load(FIXTURE("three_words.txt"), &count);

    CHECK(!dictionary_contains(words, count, "XXXXX"));
    CHECK(!dictionary_contains(words, count, "CRANE"));
    CHECK(!dictionary_contains(words, count, ""));        /* [R6] */
    CHECK(!dictionary_contains(words, count, "BERR"));    /* [R7] */
    CHECK(!dictionary_contains(words, count, "BERRYS"));  /* [R7] */
    CHECK(!dictionary_contains(words, count, NULL));
    CHECK(!dictionary_contains(NULL, 0, "BERRY"));

    dictionary_free(words, count);
}


/* ---- tolerant parsing ---------------------------------------------- */

static void test_load_skips_invalid_lines(void)
{
    int count = 0;
    char **words = dictionary_load(FIXTURE("mixed.txt"), &count);

    /*
     * mixed.txt: APPLE, CRANES, ABCDEFGHIJK, house, (blank),
     * 12345, "AB CD", zesty (no final newline).
     */
    CHECK(count == 3);

    if (words != NULL && count == 3) {
        CHECK_STR_EQ(words[0], "APPLE");
        CHECK_STR_EQ(words[1], "HOUSE");   /* lowercase normalised */
        CHECK_STR_EQ(words[2], "ZESTY");   /* last line, no '\n'   */
    }

    /*
     * Regression test for the old fgets() bug: the long line
     * ABCDEFGHIJK used to be split, loading the fake word GHIJK.
     */
    CHECK(!dictionary_contains(words, count, "GHIJK"));
    CHECK(!dictionary_contains(words, count, "CRANE"));

    dictionary_free(words, count);
}

static void test_load_handles_windows_line_endings(void)
{
    int count = 0;
    char **words = dictionary_load(FIXTURE("crlf.txt"), &count);

    CHECK(count == 3);
    CHECK(dictionary_contains(words, count, "APPLE"));
    CHECK(dictionary_contains(words, count, "CHILI"));

    dictionary_free(words, count);
}


/* ---- loading failures [R12] ---------------------------------------- */

static void test_load_missing_file_fails(void)
{
    int count = -1;

    CHECK(dictionary_load(FIXTURE("does_not_exist.txt"), &count) == NULL);
    CHECK(count == 0);
}

static void test_load_empty_file_fails(void)
{
    int count = -1;

    CHECK(dictionary_load(FIXTURE("empty.txt"), &count) == NULL);
    CHECK(count == 0);
}

static void test_load_file_without_valid_words_fails(void)
{
    int count = -1;

    CHECK(dictionary_load(FIXTURE("no_valid_words.txt"), &count) == NULL);
    CHECK(count == 0);
}

static void test_load_directory_fails(void)
{
    int count = -1;

    /* fopen() succeeds on a directory on Linux, but reading fails. */
    CHECK(dictionary_load("tests/data", &count) == NULL);
    CHECK(count == 0);
}

static void test_load_null_arguments_fail(void)
{
    int count = -1;

    CHECK(dictionary_load(NULL, &count) == NULL);
    CHECK(count == 0);
    CHECK(dictionary_load(REAL_DICTIONARY, NULL) == NULL);
}


/* ---- stress -------------------------------------------------------- */

#define BIG_WORD_COUNT 100000

/* Turn n into a unique 5-letter word: base-26 counting, AAAAA = 0. */
static void number_to_word(int n, char word[WORD_LENGTH + 1])
{
    for (int i = WORD_LENGTH - 1; i >= 0; i--) {
        word[i] = (char)('A' + n % 26);
        n /= 26;
    }
    word[WORD_LENGTH] = '\0';
}

/* 100,000 words plus junk lines: exercises many realloc() calls. */
static void test_load_large_dictionary(void)
{
    char path[] = "/tmp/wordle_test_XXXXXX";

    /* mkstemp replaces XXXXXX with a unique name and opens the file. */
    int fd = mkstemp(path);
    CHECK(fd != -1);
    if (fd == -1) {
        return;
    }

    FILE *file = fdopen(fd, "w");
    CHECK(file != NULL);
    if (file == NULL) {
        close(fd);
        unlink(path);
        return;
    }

    for (int i = 0; i < BIG_WORD_COUNT; i++) {
        char word[WORD_LENGTH + 1];
        number_to_word(i, word);
        fprintf(file, "%s\n", word);

        if (i % 1000 == 0) {
            fprintf(file, "not-a-word\n\n");   /* must be skipped */
        }
    }

    fclose(file);

    int count = 0;
    char **words = dictionary_load(path, &count);

    /* Delete the temporary file before checking, so it never lingers. */
    unlink(path);

    CHECK(words != NULL);
    CHECK(count == BIG_WORD_COUNT);

    if (words != NULL && count == BIG_WORD_COUNT) {
        char last[WORD_LENGTH + 1];
        number_to_word(BIG_WORD_COUNT - 1, last);

        CHECK_STR_EQ(words[0], "AAAAA");
        CHECK_STR_EQ(words[count - 1], last);
        CHECK(dictionary_contains(words, count, last));
        CHECK(!dictionary_contains(words, count, "ZZZZZ"));
    }

    dictionary_free(words, count);
}


/* ---- random choice and cleanup ------------------------------------- */

static void test_pick_random_returns_a_listed_word(void)
{
    int count = 0;
    char **words = dictionary_load(FIXTURE("three_words.txt"), &count);

    srand(12345);   /* fixed seed: same "random" sequence every run */

    for (int i = 0; i < 50; i++) {
        const char *word = dictionary_pick_random(words, count);
        CHECK(word != NULL && dictionary_contains(words, count, word));
    }

    CHECK(dictionary_pick_random(NULL, 3) == NULL);
    CHECK(dictionary_pick_random(words, 0) == NULL);

    dictionary_free(words, count);
}

static void test_free_null_is_safe(void)
{
    dictionary_free(NULL, 0);
    dictionary_free(NULL, 5);
    CHECK(1);   /* reaching this line without crashing is the test */
}


int main(void)
{
    printf("=== dictionary tests ===\n");

    RUN_TEST(test_normalize_accepts_five_letters);
    RUN_TEST(test_normalize_rejects_bad_words);
    RUN_TEST(test_load_real_dictionary);
    RUN_TEST(test_real_answers_are_allowed_guesses);
    RUN_TEST(test_contains_is_case_insensitive);
    RUN_TEST(test_contains_rejects_unknown_words);
    RUN_TEST(test_load_skips_invalid_lines);
    RUN_TEST(test_load_handles_windows_line_endings);
    RUN_TEST(test_load_missing_file_fails);
    RUN_TEST(test_load_empty_file_fails);
    RUN_TEST(test_load_file_without_valid_words_fails);
    RUN_TEST(test_load_directory_fails);
    RUN_TEST(test_load_null_arguments_fail);
    RUN_TEST(test_pick_random_returns_a_listed_word);
    RUN_TEST(test_free_null_is_safe);
    RUN_TEST(test_load_large_dictionary);

    return TEST_SUMMARY();
}
