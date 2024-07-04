#ifndef SHELL_H
#define SHELL_H

#define BUFFER_SIZE 128

typedef enum {
    UNKNOWN = 0,
    BACK_SPACE = 8,
    LINE_FEED = 10,
    CARRIAGE_RETURN = 13,
    REGULAR_INPUT = 256,
} SPECIAL_CHAR;

void shell();

#endif