/*
 * Minimal X11 graphics layer.
 *
 * Originally: a simple graphics library for CSE 20211 by Douglas
 * Thain, http://www.nd.edu/~dthain/courses/cse20211/fall2011/gfx
 *   Version 3, 11/07/2012 - faster colour changes.
 *   Version 2, 9/23/2011  - fixes jerky animation.
 *   Version 4/4a, January 2017 (Patrick Sebastian) - rectangles and
 *   circles, filled shapes.
 *
 * Wordle project changes:
 *   - Double buffering: all drawing goes to an off-screen Pixmap that
 *     gfx_flush() copies to the window, so frames never flicker.
 *   - gfx_next_event() reports keys, mouse clicks (with correct
 *     xbutton coordinates), redraw requests and window closing.
 *   - The window close button is handled via WM_DELETE_WINDOW.
 *   - Non-blocking fullscreen toggle, gfx_close() for clean shutdown.
 */

#include "gfx.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Display *display = NULL;
static Window window;
static GC gc;
static Colormap colormap;
static int fast_color_mode = 0;
static Atom wm_delete_window;

/* Off-screen frame buffer, recreated when the window size changes. */
static Pixmap buffer = None;
static int buffer_width = 0;
static int buffer_height = 0;

/* Last known window size, kept up to date from ConfigureNotify. */
static int window_width = 0;
static int window_height = 0;

static unsigned long foreground_pixel = 0;
static unsigned long background_pixel = 0;


/* ---- helpers ------------------------------------------------------- */

static unsigned long color_to_pixel(int r, int g, int b)
{
    if (fast_color_mode) {
        /* TrueColor display: the pixel value is the RGB value itself. */
        return (unsigned long)((b & 0xff) | ((g & 0xff) << 8) | ((r & 0xff) << 16));
    }

    /* Otherwise ask the X server to allocate the colour. */
    XColor color;
    color.red   = (unsigned short)((r & 0xff) << 8);
    color.green = (unsigned short)((g & 0xff) << 8);
    color.blue  = (unsigned short)((b & 0xff) << 8);
    color.flags = DoRed | DoGreen | DoBlue;

    if (XAllocColor(display, colormap, &color)) {
        return color.pixel;
    }

    return BlackPixel(display, DefaultScreen(display));
}

/* Make sure the off-screen buffer exists and matches the window size. */
static void ensure_buffer(void)
{
    if (buffer != None &&
        buffer_width == window_width &&
        buffer_height == window_height) {
        return;
    }

    if (buffer != None) {
        XFreePixmap(display, buffer);
    }

    buffer_width  = window_width  > 0 ? window_width  : 1;
    buffer_height = window_height > 0 ? window_height : 1;

    buffer = XCreatePixmap(
        display, window,
        (unsigned int)buffer_width, (unsigned int)buffer_height,
        (unsigned int)DefaultDepth(display, DefaultScreen(display))
    );
}

/*
 * Resizing or uncovering a window produces bursts of events.
 * Remove the queued ones (keeping the newest size) so the program
 * redraws once instead of many times.
 */
static void drain_redraw_events(void)
{
    XEvent e;

    while (XCheckTypedWindowEvent(display, window, ConfigureNotify, &e)) {
        window_width  = e.xconfigure.width;
        window_height = e.xconfigure.height;
    }

    while (XCheckTypedWindowEvent(display, window, Expose, &e)) {
        /* discard */
    }
}


/* ---- window -------------------------------------------------------- */

int gfx_open(int width, int height, const char *title)
{
    display = XOpenDisplay(NULL);

    if (display == NULL) {
        fprintf(stderr,
                "gfx_open: unable to open the X display. "
                "Is a graphical session running (DISPLAY=%s)?\n",
                getenv("DISPLAY") ? getenv("DISPLAY") : "(unset)");
        return 0;
    }

    int screen = DefaultScreen(display);
    Visual *visual = DefaultVisual(display, screen);

    fast_color_mode = (visual != NULL && visual->class == TrueColor);

    /* Shrink to at most 90% of the screen, keeping the aspect ratio. */
    int max_width  = DisplayWidth(display, screen)  * 9 / 10;
    int max_height = DisplayHeight(display, screen) * 9 / 10;

    if (width > max_width || height > max_height) {
        double scale_x = (double)max_width / width;
        double scale_y = (double)max_height / height;
        double scale = scale_x < scale_y ? scale_x : scale_y;

        width  = (int)(width * scale);
        height = (int)(height * scale);
    }

    window_width = width;
    window_height = height;

    unsigned long black = BlackPixel(display, screen);

    window = XCreateSimpleWindow(
        display, RootWindow(display, screen),
        0, 0, (unsigned int)width, (unsigned int)height,
        0, black, black
    );

    /* We repaint everything ourselves, so the server need not clear. */
    XSetWindowBackgroundPixmap(display, window, None);

    XStoreName(display, window, title);

    XSizeHints *hints = XAllocSizeHints();
    if (hints != NULL) {
        hints->flags = PMinSize;
        hints->min_width = 320;
        hints->min_height = 360;
        XSetWMNormalHints(display, window, hints);
        XFree(hints);
    }

    /* Ask to be told (instead of killed) when the user closes us. */
    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    XSelectInput(display, window,
                 StructureNotifyMask | ExposureMask |
                 KeyPressMask | ButtonPressMask);

    gc = XCreateGC(display, window, 0, NULL);
    colormap = DefaultColormap(display, screen);

    foreground_pixel = WhitePixel(display, screen);
    background_pixel = black;
    XSetForeground(display, gc, foreground_pixel);

    XMapWindow(display, window);

    /* Wait until the window is actually on screen. */
    for (;;) {
        XEvent e;
        XNextEvent(display, &e);

        if (e.type == ConfigureNotify) {
            window_width  = e.xconfigure.width;
            window_height = e.xconfigure.height;
        }
        else if (e.type == MapNotify) {
            break;
        }
    }

    ensure_buffer();

    return 1;
}

void gfx_close(void)
{
    if (display == NULL) {
        return;
    }

    if (buffer != None) {
        XFreePixmap(display, buffer);
        buffer = None;
    }

    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    display = NULL;
}

void gfx_toggle_fullscreen(void)
{
    XEvent event;

    memset(&event, 0, sizeof(event));

    event.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = XInternAtom(display, "_NET_WM_STATE", False);
    event.xclient.format = 32;
    event.xclient.data.l[0] = 2;   /* 2 = toggle */
    event.xclient.data.l[1] =
        (long)XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

    XSendEvent(display, DefaultRootWindow(display), False,
               SubstructureNotifyMask | SubstructureRedirectMask, &event);
    XFlush(display);

    /*
     * No waiting here: the resize arrives later as a ConfigureNotify,
     * which gfx_next_event() reports as GFX_EVENT_REDRAW.
     */
}

int gfx_xsize(void)
{
    return window_width;
}

int gfx_ysize(void)
{
    return window_height;
}


/* ---- drawing ------------------------------------------------------- */

void gfx_color(int r, int g, int b)
{
    foreground_pixel = color_to_pixel(r, g, b);
    XSetForeground(display, gc, foreground_pixel);
}

void gfx_clear_color(int r, int g, int b)
{
    background_pixel = color_to_pixel(r, g, b);
}

void gfx_clear(void)
{
    ensure_buffer();

    XSetForeground(display, gc, background_pixel);
    XFillRectangle(display, buffer, gc, 0, 0,
                   (unsigned int)buffer_width, (unsigned int)buffer_height);
    XSetForeground(display, gc, foreground_pixel);
}

void gfx_point(int x, int y)
{
    XDrawPoint(display, buffer, gc, x, y);
}

void gfx_line(int x1, int y1, int x2, int y2)
{
    XDrawLine(display, buffer, gc, x1, y1, x2, y2);
}

void gfx_rectangle(int x, int y, int width, int height)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    XDrawRectangle(display, buffer, gc, x, y,
                   (unsigned int)width, (unsigned int)height);
}

void gfx_fillrectangle(int x, int y, int width, int height)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    XFillRectangle(display, buffer, gc, x, y,
                   (unsigned int)width, (unsigned int)height);
}

void gfx_circle(int x, int y, int diameter)
{
    if (diameter <= 0) {
        return;
    }

    XDrawArc(display, buffer, gc, x - diameter / 2, y - diameter / 2,
             (unsigned int)diameter, (unsigned int)diameter, 0, 360 * 64);
}

void gfx_fillcircle(int x, int y, int diameter)
{
    if (diameter <= 0) {
        return;
    }

    XFillArc(display, buffer, gc, x - diameter / 2, y - diameter / 2,
             (unsigned int)diameter, (unsigned int)diameter, 0, 360 * 64);
}

void gfx_text(const char *text, int x, int y)
{
    if (text == NULL) {
        return;
    }

    XDrawString(display, buffer, gc, x, y, text, (int)strlen(text));
}

void gfx_flush(void)
{
    XCopyArea(display, buffer, window, gc, 0, 0,
              (unsigned int)buffer_width, (unsigned int)buffer_height, 0, 0);
    XFlush(display);
}


/* ---- events -------------------------------------------------------- */

void gfx_next_event(GfxEvent *event)
{
    memset(event, 0, sizeof(*event));

    for (;;) {
        XEvent e;
        XNextEvent(display, &e);

        switch (e.type) {

        case KeyPress:
            event->type = GFX_EVENT_KEY;
            /* Index 0 = the key's unshifted symbol, e.g. XK_a. */
            event->key = (int)XLookupKeysym(&e.xkey, 0);
            event->x = e.xkey.x;
            event->y = e.xkey.y;
            return;

        case ButtonPress:
            event->type = GFX_EVENT_CLICK;
            event->button = (int)e.xbutton.button;
            event->x = e.xbutton.x;
            event->y = e.xbutton.y;
            return;

        case ConfigureNotify:
            /* A move without a resize needs no redraw. */
            if (e.xconfigure.width == window_width &&
                e.xconfigure.height == window_height) {
                break;
            }

            window_width  = e.xconfigure.width;
            window_height = e.xconfigure.height;
            drain_redraw_events();
            event->type = GFX_EVENT_REDRAW;
            return;

        case Expose:
            /* count > 0 means more Expose events for this change follow. */
            if (e.xexpose.count > 0) {
                break;
            }

            drain_redraw_events();
            event->type = GFX_EVENT_REDRAW;
            return;

        case ClientMessage:
            if ((Atom)e.xclient.data.l[0] == wm_delete_window) {
                event->type = GFX_EVENT_CLOSE;
                return;
            }
            break;

        default:
            break;
        }
    }
}
