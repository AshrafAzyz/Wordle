#ifndef GFX_H
#define GFX_H

/*
 * Minimal X11 graphics layer.
 *
 * Based on the gfx library by Douglas Thain (CSE 20211, University
 * of Notre Dame), extended with double buffering, a unified event
 * function, fullscreen and clean shutdown.
 *
 * All drawing goes to an off-screen buffer. Nothing appears on
 * screen until gfx_flush() is called, which avoids flicker.
 */

/* ---- window ---------------------------------------------------- */

/*
 * Open the window. If the requested size does not fit on the
 * screen it is shrunk, keeping its aspect ratio.
 *
 * Returns 1 on success. Returns 0 (after printing a message to
 * stderr) if no X display is available; the caller decides how to
 * clean up and exit.
 */
int gfx_open(int width, int height, const char *title);

/* Release every X11 resource and close the window. */
void gfx_close(void);

/*
 * Set the window icon shown in the taskbar / Alt-Tab.
 * `argb` holds width * height pixels, row by row, as 0xAARRGGBB.
 */
void gfx_set_icon(int width, int height, const unsigned int *argb);

/* Ask the window manager to toggle fullscreen (returns at once). */
void gfx_toggle_fullscreen(void);

/* Current window size in pixels. */
int gfx_xsize(void);
int gfx_ysize(void);

/* ---- drawing (to the off-screen buffer) ------------------------ */

void gfx_color(int red, int green, int blue);
void gfx_clear_color(int red, int green, int blue);

/* Fill the whole buffer with the clear colour. Start every frame with this. */
void gfx_clear(void);

void gfx_point(int x, int y);
void gfx_line(int x1, int y1, int x2, int y2);
void gfx_rectangle(int x, int y, int width, int height);
void gfx_fillrectangle(int x, int y, int width, int height);
void gfx_circle(int x, int y, int diameter);
void gfx_fillcircle(int x, int y, int diameter);

/* Small built-in X11 font; (x, y) is the left end of the baseline. */
void gfx_text(const char *text, int x, int y);

/* Copy the finished frame to the window. */
void gfx_flush(void);

/* ---- events ---------------------------------------------------- */

typedef enum {
    GFX_EVENT_KEY,      /* key: X11 KeySym of the key pressed           */
    GFX_EVENT_CLICK,    /* button (1 = left), x, y in window pixels     */
    GFX_EVENT_REDRAW,   /* window resized or uncovered: draw again      */
    GFX_EVENT_CLOSE     /* user closed the window                       */
} GfxEventType;

typedef struct {
    GfxEventType type;
    int key;
    int button;
    int x;
    int y;
} GfxEvent;

/* Block until the next event the program needs to handle. */
void gfx_next_event(GfxEvent *event);

#endif
