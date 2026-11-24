/*
 * Unit tests for the dictionary module (src/dictionary.c).
 *
 * Run from the project root (make test does this), because the
 * paths below are relative to it.
 *
 * [Rn] marks which numbered requirement from the project brief a
 * test covers.
 */
#include "dictionary.h"
#include "test_framework.h"

#include <stdlib.h>
#include <string.h>

#define REAL_DICTIONARY "data/words.txt"
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

static void test_load_real_dictionary(void)
{
    int count = 0;
    char **words = dictionary_load(REAL_DICTIONARY, &count);

    CHECK(words != NULL);
    /* 200 words also exercises growing the array past its first 128. */
    CHECK(count == 200);

    if (words != NULL && count > 0) {
        CHECK_STR_EQ(words[0], "ADMIT");
        CHECK_STR_EQ(words[count - 1], "ZESTY");

        /* Every stored word must already be valid and uppercase. */
        int all_valid = 1;
        for (int i = 0; i < count; i++) {
            char out[WORD_LENGTH + 1];
            if (!dictionary_normalize_word(words[i], out) ||
                strcmp(out, words[i]) != 0) {
                all_valid = 0;
            }
        }
        CHECK(all_valid);
    }

    dictionary_free(words, count);
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

    return TEST_SUMMARY();
}
