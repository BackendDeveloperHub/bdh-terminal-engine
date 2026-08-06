// src/engine/parser.c - BDH Pure Linux CLI Multiplexer ANSI/VT100 Parser (Arch Linux & Nano Fixed)
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
            } 
            // 🔥 NANO & VIM FIX: Carriage Return (\r) கர்சரை வரியின் தொடக்கத்திற்கு கொண்டு செல்லும்
            else if (ch == '\r') {
                win->cur_c = 0;
            } 
            // 🔥 NANO & VIM FIX: Newline (\n) கர்சரை அடுத்த வரிக்கு இறக்கும்
            else if (ch == '\n') {
                win->cur_r++;
                if (win->cur_r >= inner_h) {
                    win->cur_r = inner_h - 1;
                    // (தேவைப்பட்டால் இங்கு scroll logic இயங்கும், window_put_char பார்த்துக்கொள்ளும்)
                    window_put_char(scr, win, '\n');
                }
            } 
            else {
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
            // 1. Arch Linux / Starship OSC Sequences (\033]...)
            else if (ch == ']') {
                parser->state = STATE_OSC;
            }
            // 2. ESC c (\033c) - Full Terminal Reset:
            else if (ch == 'c') {
                clear_entire_window(win);
                parser->state = STATE_NORMAL;
            }
            // 3. Character Set Selection (\033( அல்லது \033)) - புறக்கணித்தல்:
            else if (ch == '(' || ch == ')' || ch == '=' || ch == '>') {
                parser->state = STATE_NORMAL;
            }
            else {
                parser->state = STATE_NORMAL;
                if (ch != '\033') {
                    window_put_char(scr, win, ch);
                }
            }
            break;

        // --- Arch Linux / Starship OSC Title Sequences-ஐப் பாதுகாப்பாக இக்னோர் செய்ய ---
        case STATE_OSC:
            // OSC குறியீடுகள் ASCII BEL (\007) அல்லது ESC \ (\033\\) உடன் முடிவடையும்:
            if (ch == '\007' || ch == '\\') {
                parser->state = STATE_NORMAL;
            }
            break;

        case STATE_CSI:
            if (ch == '?' || ch == '=' || ch == '>') {
                break;
            }
            else if (ch >= '0' && ch <= '9') {
                parser->cur_val = (parser->cur_val * 10) + (ch - '0');
            }
            else if (ch == ';') {
                if (parser->arg_count < 4) {
                    parser->args[parser->arg_count++] = parser->cur_val;
                }
                parser->cur_val = 0;
            }
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
                else if (ch == 'B') { // 🔥 Cursor Down (Nano Enter & Arrow Key Fix)
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
                // --- Erase Line (\033[K, \033[2K) ---
                else if (ch == 'K') {
                    int mode = parser->args[0];
                    if (mode == 0) { // Cursor to end of line
                        for (int c = win->cur_c; c < inner_w && c < WIN_MAX_COLS; c++) {
                            win->text[win->cur_r][c] = ' ';
                        }
                    } else if (mode == 1) { // Start of line to cursor
                        for (int c = 0; c <= win->cur_c && c < inner_w && c < WIN_MAX_COLS; c++) {
                            win->text[win->cur_r][c] = ' ';
                        }
                    } else if (mode == 2) { // Entire line (DO NOT MOVE CURSOR)
                        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
                            win->text[win->cur_r][c] = ' ';
                        }
                    }
                }
                // --- Erase Display (\033[J, \033[2J) - Arch Linux Screen Wipe Bug Fixed ---
                else if (ch == 'J') {
                    int mode = parser->args[0];
                    if (mode == 2 || mode == 3) {
                        clear_entire_window(win);
                    } 
                    else if (mode == 0) {
                        for (int c = win->cur_c; c < inner_w && c < WIN_MAX_COLS; c++) {
                            win->text[win->cur_r][c] = ' ';
                        }
                        for (int r = win->cur_r + 1; r < inner_h && r < WIN_MAX_ROWS; r++) {
                            for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
                                win->text[r][c] = ' ';
                            }
                        }
                    }
                    else if (mode == 1) {
                        for (int r = 0; r < win->cur_r && r < WIN_MAX_ROWS; r++) {
                            for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
                                win->text[r][c] = ' ';
                            }
                        }
                        for (int c = 0; c <= win->cur_c && c < inner_w && c < WIN_MAX_COLS; c++) {
                            win->text[win->cur_r][c] = ' ';
                        }
                    }
                }

                parser->state = STATE_NORMAL;
            }
            break;
    }
}

void parser_destroy(AnsiParser *parser) {
    free(parser);
}
