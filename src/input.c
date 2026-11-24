#include "input.h"
#include "gfx.h"

#include <X11/keysym.h>


void input_init(void)
{
    /*
     * Input is handled through the X11
     * graphics event system.
     */
}


InputEvent input_wait_event(void)
{
    InputEvent event;

    event.type = INPUT_NONE;
    event.letter = '\0';

    while (1) {

        int key = gfx_wait_key();

        /*
         * F11
         *
         * Toggle fullscreen.
         */
        if (key == XK_F11) {

            gfx_toggle_fullscreen();

            event.type = INPUT_FULLSCREEN;

            return event;
        }


        /*
         * ENTER
         *
         * Normal keyboard Enter.
         */
        if (key == XK_Return ||
            key == XK_KP_Enter) {

            event.type = INPUT_ENTER;

            return event;
        }


        /*
         * BACKSPACE
         *
         * Handle both Backspace and
         * Delete-style keyboard events.
         */
        if (key == XK_BackSpace ||
            key == XK_Delete ||
            key == XK_KP_Delete) {

            event.type = INPUT_BACKSPACE;

            return event;
        }


        /*
         * ESCAPE
         */
        if (key == XK_Escape) {

            event.type = INPUT_ESCAPE;

            return event;
        }


        /*
         * Convert lowercase letters
         * into uppercase.
         */
        if (key >= XK_a &&
            key <= XK_z) {

            key =
                key - XK_a + XK_A;
        }


        /*
         * Accept A-Z.
         */
        if (key >= XK_A &&
            key <= XK_Z) {

            event.type = INPUT_LETTER;

            event.letter =
                (char)key;

            return event;
        }
    }
}
