#ifndef INPUT_H
#define INPUT_H

/*
 * Input module: turns raw window events into game-level events.
 * It only reports what happened; main.c decides what to do.
 */

typedef enum {
    INPUT_NONE,
    INPUT_LETTER,       /* letter: 'A'..'Z' (always uppercase)      */
    INPUT_BACKSPACE,
    INPUT_ENTER,
    INPUT_ESCAPE,
    INPUT_FULLSCREEN,   /* F11 */
    INPUT_TOGGLE_SOUND, /* F2  */
    INPUT_CLICK,        /* left mouse button at x, y                */
    INPUT_REDRAW,       /* window resized/uncovered: redraw screen  */
    INPUT_QUIT          /* window close button                      */
} InputType;

typedef struct {
    InputType type;
    char letter;
    int x;
    int y;
} InputEvent;

void input_init(void);

/*
 * Convert one X11 KeySym into an InputEvent. Keys the game does not
 * use give INPUT_NONE. Needs no window, so it can be unit tested.
 */
InputEvent input_from_key(int keysym);

/* Block until the next event the game cares about. */
InputEvent input_wait_event(void);

#endif
