#ifndef GFX_H
#define GFX_H

/*
 * Window
 */
void gfx_open(
    int width,
    int height,
    const char *title
);


/*
 * Drawing
 */
void gfx_point(
    int x,
    int y
);

void gfx_line(
    int x1,
    int y1,
    int x2,
    int y2
);

void gfx_rectangle(
    int x,
    int y,
    int width,
    int height
);

void gfx_fillrectangle(
    int x,
    int y,
    int width,
    int height
);

void gfx_circle(
    int x,
    int y,
    int radius
);

void gfx_fillcircle(
    int x,
    int y,
    int radius
);

void gfx_text(
    const char *text,
    int x,
    int y
);


/*
 * Colors
 */
void gfx_color(
    int red,
    int green,
    int blue
);

void gfx_clear(void);

void gfx_clear_color(
    int red,
    int green,
    int blue
);


/*
 * Events
 */

/*
 * Wait until a keyboard event occurs
 * and return the X11 KeySym.
 */
int gfx_wait_key(void);


/*
 * Wait for a keyboard or mouse event.
 *
 * Kept for compatibility with the
 * original graphics library.
 */
char gfx_wait(void);


/*
 * Check whether an input event is waiting.
 */
int gfx_event_waiting(void);


/*
 * Last mouse/event position.
 */
int gfx_xpos(void);
int gfx_ypos(void);


/*
 * Rendering
 */
void gfx_flush(void);

/*
 * Window state
 */
void gfx_toggle_fullscreen(void);

int gfx_xsize(void);
int gfx_ysize(void);

#endif
