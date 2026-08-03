// src/engine/terminal.c - BDH Terminal Raw Mode & Mouse Control Module
#include "terminal.h"
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

// terminal.c மாட்யூலுக்குள்ளேயே பாதுகாப்பாக இருக்கும் Static Variable
static struct termios orig_termios;

// டீபால்ட் டெர்மினல் மோடுக்கு மாற்றும் பங்க்ஷன் (Exit ஆகும்போது தானாக இயங்கும்)
void terminal_disable_raw_mode(void) {
    // 1. முதலில் Mouse Tracking-ஐ ஆஃப் செய்ய வேண்டும் (இல்லையெனில் டெர்மினலில் குப்பை எழுத்துக்கள் வரும்)
    write(STDOUT_FILENO, "\033[?1006l\033[?1002l\033[?1000l", 24);
    
    // 2. பழைய Terminal Attributes-ஐ மீட்டெடுத்தல்
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Raw Mode & Mouse Tracking-ஐ ஆன் செய்யும் பங்க்ஷன்
void terminal_enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    
    // இப்போது எந்த எரரும் இல்லாமல் atexit நேரடியாக வேலை செய்யும்!
    atexit(terminal_disable_raw_mode);
    
    struct termios raw = orig_termios;
    cfmakeraw(&raw);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    // --- ADDED: SGR 1006 Mouse Tracking Mode-ஐ ஆன் செய்தல் (For 50x220 Layout) ---
    // ?1000h = Normal click | ?1002h = Click & Drag | ?1006h = Extended 220-Col SGR Coordinates
    write(STDOUT_FILENO, "\033[?1000h\033[?1002h\033[?1006h", 24);
}
