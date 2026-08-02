// src/engine/terminal.c
#include "terminal.h"
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

void terminal_disable_raw_mode(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}

void terminal_enable_raw_mode(struct termios *orig_termios) {
    tcgetattr(STDIN_FILENO, orig_termios);
    atexit(void_disable_wrapper); // atexit க்கான ராப் டெஃபினிஷன்
    
    struct termios raw = *orig_termios;
    cfmakeraw(&raw);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
