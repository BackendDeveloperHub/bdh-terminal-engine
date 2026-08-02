// src/engine/terminal.c
#include "terminal.h"
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

// terminal.c மாட்யூலுக்குள்ளேயே பாதுகாப்பாக இருக்கும் Static Variable
static struct termios orig_termios;

// டீபால்ட் டெர்மினல் மோடுக்கு மாற்றும் பங்க்ஷன்
void terminal_disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Raw Mode-ஐ ஆன் செய்யும் பங்க்ஷன்
void terminal_enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    
    // இப்போது எந்த எரரும் இல்லாமல் atexit நேரடியாக வேலை செய்யும்!
    atexit(terminal_disable_raw_mode);
    
    struct termios raw = orig_termios;
    cfmakeraw(&raw);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
