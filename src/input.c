#include "input.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>


void input_init(void)
{
    /*
     * Input initialization will be added here
     * when we connect the module to X11.
     */
}


int input_get_guess(
    char *buffer,
    int buffer_size
)
{
    if (buffer == NULL || buffer_size <= 0) {
        return 0;
    }

    printf("Enter your guess: ");

    if (fgets(buffer, buffer_size, stdin) == NULL) {
        return 0;
    }

    /*
     * Remove newline characters.
     */
    buffer[strcspn(buffer, "\r\n")] = '\0';

    /*
     * Convert all letters to uppercase.
     */
    for (int i = 0; buffer[i] != '\0'; i++) {
        buffer[i] = (char)toupper((unsigned char)buffer[i]);
    }

    return 1;
}
