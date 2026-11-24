#ifndef DICTIONARY_H
#define DICTIONARY_H

#define WORD_LENGTH 5

char **dictionary_load(const char *filename, int *word_count);
void dictionary_free(char **words, int word_count);

const char *dictionary_pick_random(char **words, int word_count);

int dictionary_contains(
    char **words,
    int word_count,
    const char *guess
);

#endif
