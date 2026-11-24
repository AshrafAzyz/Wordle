/*
 * Minimal unit-test helpers (no external library needed).
 *
 *   static void test_something(void) { CHECK(1 + 1 == 2); }
 *
 *   int main(void) {
 *       RUN_TEST(test_something);
 *       return TEST_SUMMARY();
 *   }
 *
 * A failed CHECK prints the file, line and expression, marks the
 * current test as failed and carries on, so one run shows every
 * failure. TEST_SUMMARY() returns 1 if any test failed, which
 * makes `make test` stop with an error.
 */
#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>

/*
 * `static` gives each test program its own private copy.
 * Each test executable includes this header exactly once.
 */
static int test_current_failed = 0;
static int tests_run = 0;
static int tests_failed = 0;

/*
 * #cond turns the expression into a string ("stringification"),
 * and __FILE__ / __LINE__ are filled in by the preprocessor at the
 * place where CHECK is used, not here.
 *
 * do { ... } while (0) makes the macro behave like one statement,
 * so it is safe inside an if/else without braces.
 */
#define CHECK(cond)                                              \
    do {                                                         \
        if (!(cond)) {                                           \
            printf("    %s:%d: CHECK failed: %s\n",              \
                   __FILE__, __LINE__, #cond);                   \
            test_current_failed = 1;                             \
        }                                                        \
    } while (0)

/* Compare two strings and show both values when they differ. */
#define CHECK_STR_EQ(actual, expected)                           \
    do {                                                         \
        const char *actual_ = (actual);                          \
        const char *expected_ = (expected);                      \
        if (actual_ == NULL || expected_ == NULL ||              \
            strcmp(actual_, expected_) != 0) {                   \
            printf("    %s:%d: %s: expected \"%s\", got \"%s\"\n", \
                   __FILE__, __LINE__, #actual,                  \
                   expected_ ? expected_ : "(null)",             \
                   actual_ ? actual_ : "(null)");                \
            test_current_failed = 1;                             \
        }                                                        \
    } while (0)

#define RUN_TEST(fn)                                             \
    do {                                                         \
        test_current_failed = 0;                                 \
        fn();                                                    \
        tests_run++;                                             \
        if (test_current_failed) {                               \
            tests_failed++;                                      \
        }                                                        \
        printf("%s  %s\n",                                       \
               test_current_failed ? "FAIL" : "ok  ", #fn);      \
    } while (0)

#define TEST_SUMMARY()                                           \
    (printf("\n%d/%d tests passed\n",                            \
            tests_run - tests_failed, tests_run),                \
     tests_failed == 0 ? 0 : 1)

#endif
