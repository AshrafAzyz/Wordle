#include "../src/input.h"

#include <stdio.h>


int main(void)
{
    char guess[INPUT_BUFFER_SIZE];

    input_init();

    printf("=== Input Test ===\n");

    if (input_get_guess(guess, sizeof(guess))) {
        printf("You entered: %s\n", guess);
    }
    else {
        printf("Failed to read input.\n");
        return 1;
    }

    return 0;
}
