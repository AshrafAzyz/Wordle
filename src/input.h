#ifndef INPUT_H
#define INPUT_H

#define INPUT_BUFFER_SIZE 32

void input_init(void);

int input_get_guess(
    char *buffer,
    int buffer_size
);

#endif
