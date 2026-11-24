/*
A simple graphics library for CSE 20211 by Douglas Thain
For complete documentation, see:
http://www.nd.edu/~dthain/courses/cse20211/fall2011/gfx
Version 3, 11/07/2012 - Now much faster at changing colors rapidly.
Version 2, 9/23/2011 - Fixes a bug that could result in jerky animation.

Patrick Sebastian added these on version 4 & 4a
Version 4, January 13, 2017 - added functionality to draw rectangles and circles.
Version 4a, January 16, 2017 - added functionality to fill rectangles and circles.
*/

#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfx.h"
#include <X11/keysym.h>

/*
gfx_open creates several X11 objects, and stores them in globals
for use by the other functions in the library.
*/

static Display *gfx_display=0;
static Window  gfx_window;
static GC      gfx_gc;
static Colormap gfx_colormap;
static int      gfx_fast_color_mode = 0;

/* These values are saved by gfx_wait then retrieved later by gfx_xpos and gfx_ypos. */

static int saved_xpos = 0;
static int saved_ypos = 0;
static int gfx_fullscreen = 0;

/* Open a new graphics window. */

void gfx_open( int width, int height, const char *title )
{
	gfx_display = XOpenDisplay(0);
	if(!gfx_display) {
		fprintf(stderr,"gfx_open: unable to open the graphics window.\n");
		exit(1);
	}

	Visual *visual = DefaultVisual(gfx_display,0);
	if(visual && visual->class==TrueColor) {
		gfx_fast_color_mode = 1;
	} else {
		gfx_fast_color_mode = 0;
	}

	int blackColor = BlackPixel(gfx_display, DefaultScreen(gfx_display));
	int whiteColor = WhitePixel(gfx_display, DefaultScreen(gfx_display));

	gfx_window = XCreateSimpleWindow(gfx_display, DefaultRootWindow(gfx_display), 0, 0, width, height, 0, blackColor, blackColor);

	XSetWindowAttributes attr;
	attr.backing_store = Always;

	XChangeWindowAttributes(gfx_display,gfx_window,CWBackingStore,&attr);

	XStoreName(gfx_display,gfx_window,title);

	XSelectInput(gfx_display, gfx_window, StructureNotifyMask|KeyPressMask|ButtonPressMask);

	XMapWindow(gfx_display,gfx_window);

	gfx_gc = XCreateGC(gfx_display, gfx_window, 0, 0);

	gfx_colormap = DefaultColormap(gfx_display,0);

	XSetForeground(gfx_display, gfx_gc, whiteColor);

	// Wait for the MapNotify event

	for(;;) {
		XEvent e;
		XNextEvent(gfx_display, &e);
		if (e.type == MapNotify)
			break;
	}
}

void gfx_toggle_fullscreen(void)
{
    Atom wm_state;
    Atom fullscreen;
    XEvent event;

    wm_state = XInternAtom(
        gfx_display,
        "_NET_WM_STATE",
        False
    );

    fullscreen = XInternAtom(
        gfx_display,
        "_NET_WM_STATE_FULLSCREEN",
        False
    );

    memset(
        &event,
        0,
        sizeof(event)
    );

    event.type = ClientMessage;

    event.xclient.window =
        gfx_window;

    event.xclient.message_type =
        wm_state;

    event.xclient.format = 32;

    /*
     * 2 = toggle
     */
    event.xclient.data.l[0] = 2;

    event.xclient.data.l[1] =
        fullscreen;

    event.xclient.data.l[2] = 0;

    XSendEvent(
        gfx_display,
        DefaultRootWindow(gfx_display),
        False,
        SubstructureNotifyMask |
        SubstructureRedirectMask,
        &event
    );

    XFlush(gfx_display);

    /*
     * Wait until the window manager
     * actually resizes our window.
     */
    while (1) {

        XNextEvent(
            gfx_display,
            &event
        );

        if (event.type == ConfigureNotify &&
            event.xconfigure.window == gfx_window) {

            break;
        }
    }

    gfx_fullscreen =
        !gfx_fullscreen;
}

/* Draw a single point at (x,y) */

void gfx_point( int x, int y )
{
	XDrawPoint(gfx_display,gfx_window,gfx_gc,x,y);
}

/* Draw a line from (x1,y1) to (x2,y2) */

void gfx_line( int x1, int y1, int x2, int y2 )
{
	XDrawLine(gfx_display,gfx_window,gfx_gc,x1,y1,x2,y2);
}

/* Draw a rectangle at (x,y) - top left corner -  with width and height */

void gfx_rectangle( int x, int y, int width, int height )
{
	XDrawRectangle(gfx_display,gfx_window,gfx_gc,x,y,width,height);
}

/* Draw a circle at (x,y) ) */

void gfx_circle( int x, int y, int radius )
{
	XDrawArc(gfx_display,gfx_window,gfx_gc,x-(radius/2),y-(radius/2),radius, radius,0*64, 360*64);
}

/* Filling a rectangle at (x,y) */

void gfx_fillrectangle(int x, int y, int width, int height)
{
	XFillRectangle(gfx_display,gfx_window,gfx_gc,x,y,width,height);
}

/* Filling a circle */
void gfx_fillcircle( int x, int y, int radius )
{
	XFillArc(gfx_display,gfx_window,gfx_gc,x-(radius/2),y-(radius/2),radius, radius,0*64, 360*64);
}

/*void gfx_text1(char *test1, int x, int y)
{
	XDrawText(gfx_display,gfx_window,gfx_gc,x,y,test1,6);
}
*/


void gfx_text(const char *text, int x, int y)
{
    XDrawString(
        gfx_display,
        gfx_window,
        gfx_gc,
        x,
        y,
        text,
        strlen(text)
    );
}

/* Change the current drawing color. */

void gfx_color( int r, int g, int b )
{
	XColor color;

	if(gfx_fast_color_mode) {
		/* If this is a truecolor display, we can just pick the color directly. */
		color.pixel = ((b&0xff) | ((g&0xff)<<8) | ((r&0xff)<<16) );
	} else {
		/* Otherwise, we have to allocate it from the colormap of the display. */
		color.pixel = 0;
		color.red = r<<8;
		color.green = g<<8;
		color.blue = b<<8;
		XAllocColor(gfx_display,gfx_colormap,&color);
	}

	XSetForeground(gfx_display, gfx_gc, color.pixel);
}

/* Clear the graphics window to the background color. */

void gfx_clear()
{
	XClearWindow(gfx_display,gfx_window);
}

/* Change the current background color. */

void gfx_clear_color( int r, int g, int b )
{
	XColor color;
	color.pixel = 0;
	color.red = r<<8;
	color.green = g<<8;
	color.blue = b<<8;
	XAllocColor(gfx_display,gfx_colormap,&color);

	XSetWindowAttributes attr;
	attr.background_pixel = color.pixel;
	XChangeWindowAttributes(gfx_display,gfx_window,CWBackPixel,&attr);
}

int gfx_event_waiting()
{
       XEvent event;

       gfx_flush();

       while (1) {
               if(XCheckMaskEvent(gfx_display,-1,&event)) {
                       if(event.type==KeyPress) {
                               XPutBackEvent(gfx_display,&event);
                               return 1;
                       } else if (event.type==ButtonPress) {
                               XPutBackEvent(gfx_display,&event);
                               return 1;
                       } else {
                               return 0;
                       }
               } else {
                       return 0;
               }
       }
}

int gfx_wait_key(void)
{
    XEvent event;

    gfx_flush();

    while (1) {

        XNextEvent(
            gfx_display,
            &event
        );

        if (event.type == KeyPress) {

            saved_xpos = event.xkey.x;
            saved_ypos = event.xkey.y;

            return XLookupKeysym(
                &event.xkey,
                0
            );
        }
    }
}
/* Wait for the user to press a key or mouse button. */

char gfx_wait()
{
	XEvent event;

	gfx_flush();

	while(1) {
		XNextEvent(gfx_display,&event);

		if(event.type==KeyPress) {
			saved_xpos = event.xkey.x;
			saved_ypos = event.xkey.y;
			return XLookupKeysym(&event.xkey,0);
		} else if(event.type==ButtonPress) {
			saved_xpos = event.xkey.x;
			saved_ypos = event.xkey.y;
			return event.xbutton.button;
		}
	}
}

int gfx_xsize(void)
{
    XWindowAttributes attributes;

    XGetWindowAttributes(
        gfx_display,
        gfx_window,
        &attributes
    );

    return attributes.width;
}


int gfx_ysize(void)
{
    XWindowAttributes attributes;

    XGetWindowAttributes(
        gfx_display,
        gfx_window,
        &attributes
    );

    return attributes.height;
}


/* Return the X and Y coordinates of the last event. */

int gfx_xpos()
{
	return saved_xpos;
}

int gfx_ypos()
{
	return saved_ypos;
}

/* Flush all previous output to the window. */

void gfx_flush()
{
	XFlush(gfx_display);
}


