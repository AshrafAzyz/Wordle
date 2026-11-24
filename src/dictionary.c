#include "dictionary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 128


int dictionary_normalize_word(
    const char *input,
    char output[WORD_LENGTH + 1]
)
{
    if (input == NULL || output == NULL) {
        return 0;
    }

    for (int i = 0; i < WORD_LENGTH; i++) {
        char c = input[i];

        /* ASCII lowercase -> uppercase ('a' - 'A' == 32). */
        if (c >= 'a' && c <= 'z') {
            c = (char)(c - 'a' + 'A');
        }

        /*
         * Rejects digits, punctuation and also '\0', so a word
         * that is too short fails here before we read past it.
         */
        if (c < 'A' || c > 'Z') {
            return 0;
        }

        output[i] = c;
    }

    /* Too long: there are more characters after the 5th. */
    if (input[WORD_LENGTH] != '\0') {
        return 0;
    }

    output[WORD_LENGTH] = '\0';

    return 1;
}


/*
 * Read one line of any length.
 *
 * Up to size - 1 characters are stored in `buffer` (always
 * '\0'-terminated); the rest of a longer line is consumed and
 * discarded, and *too_long is set to 1. This guarantees that one
 * long line is never split into several "words".
 *
 * Returns 0 when the end of the file is reached with nothing read.
 */
static int read_line(
    FILE *file,
    char *buffer,
    size_t size,
    int *too_long
)
{
    size_t length = 0;
    int c;

    *too_long = 0;

    while ((c = fgetc(file)) != EOF && c != '\n') {
        if (length + 1 < size) {
            buffer[length++] = (char)c;
        }
        else {
            *too_long = 1;
        }
    }

    buffer[length] = '\0';

    /* Windows line endings: drop a trailing carriage return. */
    if (length > 0 && buffer[length - 1] == '\r') {
        buffer[length - 1] = '\0';
    }

    return !(c == EOF && length == 0 && !*too_long);
}


char **dictionary_load(const char *filename, int *word_count)
{
    if (word_count != NULL) {
        *word_count = 0;
    }

    if (filename == NULL || word_count == NULL) {
        return NULL;
    }

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return NULL;
    }

    char **words = NULL;
    int count = 0;
    int capacity = 0;

    /* Room for 5 letters, a possible '\r', and '\0'. */
    char line[WORD_LENGTH + 2];
    int too_long;

    while (read_line(file, line, sizeof(line), &too_long)) {

        char word[WORD_LENGTH + 1];

        if (too_long || !dictionary_normalize_word(line, word)) {
            continue;
        }

        /* Grow the array when it is full (doubling strategy). */
        if (count == capacity) {
            int new_capacity =
                (capacity == 0) ? INITIAL_CAPACITY : capacity * 2;

            /* realloc(NULL, n) behaves like malloc(n). */
            char **grown = realloc(
                words,
                (size_t)new_capacity * sizeof(*grown)
            );

            if (grown == NULL) {
                goto fail;
            }

            words = grown;
            capacity = new_capacity;
        }

        words[count] = malloc(WORD_LENGTH + 1);

        if (words[count] == NULL) {
            goto fail;
        }

        memcpy(words[count], word, WORD_LENGTH + 1);
        count++;
    }

    /* A read error, or a file with no usable words, is a failure. */
    if (ferror(file) || count == 0) {
        goto fail;
    }

    fclose(file);
    *word_count = count;

    return words;

fail:
    /* Single cleanup path: free whatever was allocated so far. */
    dictionary_free(words, count);
    fclose(file);

    return NULL;
}


void dictionary_free(char **words, int word_count)
{
    if (words == NULL) {
        return;
    }

    for (int i = 0; i < word_count; i++) {
        free(words[i]);
    }

    free(words);
}


const char *dictionary_pick_random(char **words, int word_count)
{
    if (words == NULL || word_count <= 0) {
        return NULL;
    }

    return words[rand() % word_count];
}


int dictionary_contains(
    char **words,
    int word_count,
    const char *guess
)
{
    char word[WORD_LENGTH + 1];

    if (words == NULL || !dictionary_normalize_word(guess, word)) {
        return 0;
    }

    for (int i = 0; i < word_count; i++) {
        if (strcmp(words[i], word) == 0) {
            return 1;
        }
    }

    return 0;
}
