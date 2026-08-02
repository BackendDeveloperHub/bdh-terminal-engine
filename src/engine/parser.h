// src/engine/parser.h
#ifndef BDH_PARSER_H
#define BDH_PARSER_H

#include "screen.h"
#include "ui/panes.h"

typedef enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_CSI
} ParserState;

typedef struct {
    ParserState state;
    int args[4];       // <-- கம்பைலர் தேடும் புதிய member
    int arg_count;     // <-- கம்பைலர் தேடும் புதிய member
    int cur_val;       // <-- கம்பைலர் தேடும் புதிய member
} AnsiParser;

AnsiParser* parser_create(void);
void parser_feed_char(AnsiParser *parser, VirtualScreen *scr, FloatingWindow *win, char ch);
void parser_destroy(AnsiParser *parser);

#endif // BDH_PARSER_H
