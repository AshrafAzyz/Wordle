#include "dictionary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char **dictionary_load(const char *filename, int *word_count)
{
    if (filename == NULL || word_count == NULL) {
        return NULL;
    }

    *word_count = 0;

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return NULL;
    }

    char **words = NULL;
    int count = 0;
    int capacity = 100;

    words = malloc(capacity * sizeof(char *));

    if (words == NULL) {
        fclose(file);
        return NULL;
    }

    char buffer[WORD_LENGTH + 2];

    while (fgets(buffer, sizeof(buffer), file) != NULL) {

        /* Remove newline characters */
        buffer[strcspn(buffer, "\r\n")] = '\0';

        /* Ignore words that aren't exactly 5 letters */
        if (strlen(buffer) != WORD_LENGTH) {
            continue;
        }

        /* Expand the array when necessary */
        if (count >= capacity) {
            capacity *= 2;

            char **new_words = realloc(
                words,
                capacity * sizeof(char *)
            );

            if (new_words == NULL) {
                dictionary_free(words, count);
                fclose(file);
                return NULL;
            }

            words = new_words;
        }

        /* Allocate memory for the word + '\0' */
        words[count] = malloc(WORD_LENGTH + 1);

        if (words[count] == NULL) {
            dictionary_free(words, count);
            fclose(file);
            return NULL;
        }

        strcpy(words[count], buffer);

        count++;
    }

    fclose(file);

    *word_count = count;

    return words;
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
    if (words == NULL || guess == NULL) {
        return 0;
    }

    for (int i = 0; i < word_count; i++) {
        if (strcmp(words[i], guess) == 0) {
            return 1;
        }
    }

    return 0;
}
