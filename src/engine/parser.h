// src/engine/parser.h
#ifndef BDH_PARSER_H
#define BDH_PARSER_H

#include "engine/screen.h"
#include "ui/panes.h"

// Parser-ன் 3 நிலைகள் (States)
typedef enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_CSI
} ParserState;

// State Machine அமைப்பு
typedef struct {
    ParserState state;
} AnsiParser;

AnsiParser* parser_create();
void parser_feed_char(AnsiParser *parser, VirtualScreen *scr, FloatingWindow *win, char ch);
void parser_destroy(AnsiParser *parser);

#endif // BDH_PARSER_H
