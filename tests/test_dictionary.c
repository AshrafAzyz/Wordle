#include "../src/dictionary.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    int word_count = 0;

    char **words = dictionary_load(
        "data/words.txt",
        &word_count
    );

    if (words == NULL) {
        printf("Failed to load dictionary.\n");
        return 1;
    }

    printf("Dictionary loaded successfully!\n");
    printf("Word count: %d\n", word_count);

    printf("First word: %s\n", words[0]);
    printf("Last word: %s\n", words[word_count - 1]);

    printf(
        "Contains APPLE: %s\n",
        dictionary_contains(words, word_count, "APPLE")
            ? "YES"
            : "NO"
    );

    printf(
        "Contains XXXXX: %s\n",
        dictionary_contains(words, word_count, "XXXXX")
            ? "YES"
            : "NO"
    );

    srand((unsigned int)time(NULL));

    const char *random_word =
        dictionary_pick_random(words, word_count);

    printf("Random word: %s\n", random_word);

    dictionary_free(words, word_count);

    return 0;
}
