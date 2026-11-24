/*
 * Manual input test (requires an X11 display).
 *
 * Opens a small window and prints every InputEvent produced by the
 * input module. Click the window, then type letters, Backspace,
 * Enter or F11. Press ESC to quit.
 *
 * Not part of `make test`, because it needs a human at the keyboard.
 * Build with: make manual-tests
 */
#include "gfx.h"
#include "input.h"

#include <stdio.h>

int main(void)
{
    gfx_open(400, 200, "Input Test - press ESC to quit");
    input_init();

    printf("Type letters, Backspace, Enter or F11. ESC quits.\n");

    for (;;) {
        InputEvent event = input_wait_event();

        switch (event.type) {
        case INPUT_LETTER:
            printf("LETTER     %c\n", event.letter);
            break;
        case INPUT_BACKSPACE:
            printf("BACKSPACE\n");
            break;
        case INPUT_ENTER:
            printf("ENTER\n");
            break;
        case INPUT_FULLSCREEN:
            printf("FULLSCREEN\n");
            break;
        case INPUT_ESCAPE:
            printf("ESCAPE - exiting\n");
            return 0;
        case INPUT_NONE:
            break;
        }

        fflush(stdout);
    }
}
