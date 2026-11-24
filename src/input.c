#include "input.h"
#include "gfx.h"

#include <X11/keysym.h>


void input_init(void)
{
    /* Nothing to set up: events come from the gfx window. */
}


InputEvent input_from_key(int keysym)
{
    InputEvent event = { INPUT_NONE, '\0', 0, 0 };

    switch (keysym) {
    case XK_Return:
    case XK_KP_Enter:
        event.type = INPUT_ENTER;
        return event;

    case XK_BackSpace:
    case XK_Delete:
    case XK_KP_Delete:
        event.type = INPUT_BACKSPACE;
        return event;

    case XK_Escape:
        event.type = INPUT_ESCAPE;
        return event;

    case XK_F11:
        event.type = INPUT_FULLSCREEN;
        return event;

    case XK_F2:
        event.type = INPUT_TOGGLE_SOUND;
        return event;

    default:
        break;
    }

    /* X11 KeySyms for letters match ASCII: XK_a == 'a', XK_A == 'A'. */
    if (keysym >= XK_a && keysym <= XK_z) {
        keysym = keysym - XK_a + XK_A;
    }

    if (keysym >= XK_A && keysym <= XK_Z) {
        event.type = INPUT_LETTER;
        event.letter = (char)keysym;
    }

    return event;
}


InputEvent input_wait_event(void)
{
    InputEvent event = { INPUT_NONE, '\0', 0, 0 };

    for (;;) {
        GfxEvent raw;
        gfx_next_event(&raw);

        switch (raw.type) {
        case GFX_EVENT_CLOSE:
            event.type = INPUT_QUIT;
            return event;

        case GFX_EVENT_REDRAW:
            event.type = INPUT_REDRAW;
            return event;

        case GFX_EVENT_CLICK:
            /* Only the left button; ignore right/middle/scroll wheel. */
            if (raw.button == 1) {
                event.type = INPUT_CLICK;
                event.x = raw.x;
                event.y = raw.y;
                return event;
            }
            break;

        case GFX_EVENT_KEY:
            event = input_from_key(raw.key);
            if (event.type != INPUT_NONE) {
                return event;
            }
            break;
        }
    }
}
