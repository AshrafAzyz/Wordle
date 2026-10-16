#include "../src/gfx.h"

int main(void)
{
    gfx_open(800, 600, "Wordle Graphics Test");

    gfx_clear_color(247, 241, 227);
    gfx_clear();

    gfx_color(33, 140, 124);
    gfx_fillrectangle(100, 100, 100, 100);

    gfx_color(0, 0, 0);
    gfx_rectangle(100, 100, 100, 100);

    gfx_text("WORDLE", 120, 250);

    gfx_flush();

    gfx_wait();

    return 0;
}
