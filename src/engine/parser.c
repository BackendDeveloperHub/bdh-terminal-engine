// src/engine/parser.c - BDH Pure Linux CLI Multiplexer ANSI/VT100 Parser
#include "parser.h"
#include <stdlib.h>

AnsiParser* parser_create(void) {
    AnsiParser *parser = (AnsiParser*)malloc(sizeof(AnsiParser));
    parser->state = STATE_NORMAL;
    parser->arg_count = 0;
    parser->cur_val = 0;
    for (int i = 0; i < 4; i++) parser->args[i] = 0;
    return parser;
}

// முழு விண்டோவையும் சுத்தமாகத் துடைத்து கர்சரை (0,0) மூலையில் நிறுத்தும் பங்க்ஷன்:
static void clear_entire_window(FloatingWindow *win) {
    int inner_w = win->width - 2;
    int inner_h = win->height - 2;
    for (int r = 0; r < inner_h && r < WIN_MAX_ROWS; r++) {
        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = ' ';
        }
    }
    win->cur_r = 0;
    win->cur_c = 0;
}

void parser_feed_char(AnsiParser *parser, VirtualScreen *scr, FloatingWindow *win, char ch) {
    int inner_w = win->width - 2;
    int inner_h = win->height - 2;

    switch (parser->state) {
        case STATE_NORMAL:
            if (ch == '\033') { // Escape Byte (0x1B)
                parser->state = STATE_ESC;
            } else {
                window_put_char(scr, win, ch);
            }
            break;

        case STATE_ESC:
            if (ch == '[') {
                parser->state = STATE_CSI;
                parser->arg_count = 0;
                parser->cur_val = 0;
                for (int i = 0; i < 4; i++) parser->args[i] = 0;
            } 
            // 1. ESC c (\033c) - Full Terminal Reset / Clear Screen குறியீடு:
            else if (ch == 'c') {
                clear_entire_window(win);
                parser->state = STATE_NORMAL;
            }
            // 2. Character Set Selection (\033( அல்லது \033)) & OSC Title Sequences - புறக்கணித்தல்:
            else if (ch == '(' || ch == ')' || ch == ']' || ch == '=' || ch == '>') {
                parser->state = STATE_NORMAL; // குப்பை எழுத்துக்கள் திரையில் வராது
            }
            else {
                parser->state = STATE_NORMAL;
                if (ch != '\033') {
                    window_put_char(scr, win, ch);
                }
            }
            break;

        case STATE_CSI:
            // '?' அல்லது மற்ற Private Mode எழுத்துக்களைப் புறக்கணித்தல்:
            if (ch == '?' || ch == '=' || ch == '>') {
                break;
            }
            // 1. எண்களைப் பிரித்தெடுத்தல் ('0'..'9')
            else if (ch >= '0' && ch <= '9') {
                parser->cur_val = (parser->cur_val * 10) + (ch - '0');
            }
            // 2. செமிகோலன் (';') வந்தால் அடுத்த ஆர்குமெண்ட்
            else if (ch == ';') {
                if (parser->arg_count < 4) {
                    parser->args[parser->arg_count++] = parser->cur_val;
                }
                parser->cur_val = 0;
            }
            // 3. குறியீடு முடிவடையும் எழுத்துக்கள் (@ முதல் ~ வரை)
            else if (ch >= 0x40 && ch <= 0x7E) {
                if (parser->arg_count < 4) {
                    parser->args[parser->arg_count++] = parser->cur_val;
                }

                int n = (parser->args[0] > 0) ? parser->args[0] : 1;

                // --- ANSI VT100 Cursor & Screen Handlers ---
                if (ch == 'A') { // Cursor Up
                    win->cur_r -= n;
                    if (win->cur_r < 0) win->cur_r = 0;
                }
                else if (ch == 'B') { // Cursor Down
                    win->cur_r += n;
                    if (win->cur_r >= inner_h) win->cur_r = inner_h - 1;
                }
                else if (ch == 'C') { // Cursor Right
                    win->cur_c += n;
                    if (win->cur_c >= inner_w) win->cur_c = inner_w - 1;
                }
                else if (ch == 'D') { // Cursor Left
                    win->cur_c -= n;
                    if (win->cur_c < 0) win->cur_c = 0;
                }
                else if (ch == 'G') { // Horizontal Absolute
                    int col = (parser->args[0] > 0) ? parser->args[0] - 1 : 0;
                    if (col < 0) col = 0;
                    if (col >= inner_w) col = inner_w - 1;
                    win->cur_c = col;
                }
                else if (ch == 'H' || ch == 'f') { // Cursor Position (row;col)
                    int row = (parser->args[0] > 0) ? parser->args[0] - 1 : 0;
                    int col = (parser->arg_count > 1 && parser->args[1] > 0) ? parser->args[1] - 1 : 0;
                    if (row < 0) row = 0;
                    if (row >= inner_h) row = inner_h - 1;
                    if (col < 0) col = 0;
                    if (col >= inner_w) col = inner_w - 1;
                    win->cur_r = row;
                    win->cur_c = col;
                }
                else if (ch == 'K') { // Erase Line
                    for (int c = win->cur_c; c < inner_w && c < WIN_MAX_COLS; c++) {
                        win->text[win->cur_r][c] = ' ';
                    }
                }
                // CSI J (\033[J, \033[2J, \033[3J) - Clear Screen குறியீடுகள்:
                else if (ch == 'J') {
                    clear_entire_window(win);
                }

                parser->state = STATE_NORMAL;
            }
            break;
    }
}

void parser_destroy(AnsiParser *parser) {
    free(parser);
}
