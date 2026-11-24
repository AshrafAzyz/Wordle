#ifndef DICTIONARY_H
#define DICTIONARY_H

#define WORD_LENGTH 5

/*
 * Validate and normalise a word.
 *
 * Succeeds only if `input` is exactly WORD_LENGTH ASCII letters
 * (either case). On success, writes the uppercase word plus '\0'
 * into `output` and returns 1. Otherwise returns 0 and leaves
 * `output` unspecified.
 */
int dictionary_normalize_word(
    const char *input,
    char output[WORD_LENGTH + 1]
);

/*
 * Load a word list: one word per line.
 *
 * Lines that are not exactly WORD_LENGTH letters are skipped.
 * Words are stored in uppercase.
 *
 * Returns a heap-allocated array of words and sets *word_count.
 * Returns NULL (and sets *word_count to 0) if the file cannot be
 * opened or read, memory runs out, or no valid words are found.
 *
 * Free the result with dictionary_free().
 */
char **dictionary_load(const char *filename, int *word_count);

void dictionary_free(char **words, int word_count);

/* Returns a random word, or NULL if the list is empty. Uses rand(). */
const char *dictionary_pick_random(char **words, int word_count);

/* Returns 1 if `guess` is in the list (case-insensitive), else 0. */
int dictionary_contains(
    char **words,
    int word_count,
    const char *guess
);

#endif
