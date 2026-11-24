/*
 * Manual input test (requires an X11 display).
 *
 * Opens a small window and prints every InputEvent produced by the
 * input module. Type letters, Backspace, Enter or F11, or click the
 * window. Press ESC or close the window to quit.
 *
 * Not part of `make test`, because it needs a human at the keyboard.
 * Build with: make manual-tests
 */
#include "gfx.h"
#include "input.h"

#include <stdio.h>

int main(void)
{
    if (!gfx_open(400, 200, "Input Test - press ESC to quit")) {
        return 1;
    }

    input_init();

    printf("Type letters, Backspace, Enter, F11 or click. ESC quits.\n");

    for (int running = 1; running; ) {
        InputEvent event = input_wait_event();

        switch (event.type) {
        case INPUT_LETTER:     printf("LETTER     %c\n", event.letter);          break;
        case INPUT_BACKSPACE:  printf("BACKSPACE\n");                            break;
        case INPUT_ENTER:      printf("ENTER\n");                                break;
        case INPUT_FULLSCREEN: printf("FULLSCREEN\n");                           break;
        case INPUT_CLICK:      printf("CLICK      x=%d y=%d\n", event.x, event.y); break;
        case INPUT_REDRAW:     printf("REDRAW\n");                               break;
        case INPUT_ESCAPE:     printf("ESCAPE - exiting\n");  running = 0;       break;
        case INPUT_QUIT:       printf("QUIT - window closed\n"); running = 0;    break;
        case INPUT_NONE:                                                         break;
        }

        fflush(stdout);
    }

    gfx_close();

    return 0;
}
