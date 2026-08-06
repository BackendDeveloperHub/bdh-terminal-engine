// src/engine/parser.h - BDH Pure Linux CLI Multiplexer ANSI/VT100 Parser Header
#ifndef BDH_PARSER_H
#define BDH_PARSER_H

#include "screen.h"
#include "ui/panes.h"

typedef enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_CSI,
    STATE_OSC,  // <-- ADDED: Arch Linux / Starship OSC Title Sequences-ஐ கையாள
    STATE_CHARSET
} ParserState;

typedef struct {
    ParserState state;
    int args[4];
    int arg_count;
    int cur_val;
} AnsiParser;

AnsiParser* parser_create(void);
void parser_feed_char(AnsiParser *parser, VirtualScreen *scr, FloatingWindow *win, char ch);
void parser_destroy(AnsiParser *parser);

#endif // BDH_PARSER_H
