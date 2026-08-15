// src/engine/parser.h - BDH Pure Linux CLI Multiplexer ANSI/VT100 Parser Header (256-Color Ready)
#ifndef BDH_PARSER_H
#define BDH_PARSER_H

#include "screen.h"
#include "ui/panes.h"

typedef enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_CSI,
    STATE_OSC,  // Arch Linux / Starship OSC Title Sequences-ஐ கையாள
    STATE_CHARSET
} ParserState;

typedef struct {
    ParserState state;
    int args[16];    // 🔥 4-ல் இருந்து 16 ஆக மாற்றப்பட்டுள்ளது (256-Colors-க்காக)
    int arg_count;
    int cur_val;
    int current_fg;  // 🔥 தற்போதைய எழுத்தின் நிறம் (Foreground)
    int current_bg;  // 🔥 தற்போதைய பின்னணி நிறம் (Background)
} AnsiParser;

AnsiParser* parser_create(void);
void parser_feed_char(AnsiParser *parser, VirtualScreen *scr, FloatingWindow *win, char ch);
void parser_destroy(AnsiParser *parser);

#endif // BDH_PARSER_H
