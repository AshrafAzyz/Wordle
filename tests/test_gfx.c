/*
 * Manual graphics test (requires an X11 display).
 *
 * Draws a few shapes, then waits for a key press, a click or the
 * window being closed. Resize the window to check redrawing.
 * Build with: make manual-tests
 */
#include "gfx.h"

static void draw(void)
{
    gfx_clear_color(247, 241, 227);
    gfx_clear();

    gfx_color(33, 140, 124);
    gfx_fillrectangle(100, 100, 100, 100);

    gfx_color(0, 0, 0);
    gfx_rectangle(100, 100, 100, 100);
    gfx_text("WORDLE - press a key or click to close", 100, 250);

    gfx_flush();
}

int main(void)
{
    if (!gfx_open(800, 600, "Wordle Graphics Test")) {
        return 1;
    }

    draw();

    for (;;) {
        GfxEvent event;
        gfx_next_event(&event);

        if (event.type == GFX_EVENT_REDRAW) {
            draw();
            continue;
        }

        break;   /* key, click or close */
    }

    gfx_close();

    return 0;
}
