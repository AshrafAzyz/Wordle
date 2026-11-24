#ifndef INPUT_H
#define INPUT_H


typedef enum {
    INPUT_NONE,
    INPUT_LETTER,
    INPUT_BACKSPACE,
    INPUT_ENTER,
    INPUT_ESCAPE,
    INPUT_FULLSCREEN
} InputType;


typedef struct {
    InputType type;
    char letter;
} InputEvent;


void input_init(void);

InputEvent input_wait_event(void);


#endif
