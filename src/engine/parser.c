// src/engine/parser.c - BDH Pure Linux CLI Multiplexer ANSI/VT100 Parser (Zero Error Fixed)
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

// 1. 🔥 RENAMED: panes.h-உடன் மோதாமல் இருக்க parser_scroll_up என மாற்றப்பட்டுள்ளது:
static void parser_scroll_up(FloatingWindow *win, int n) {
    int inner_w = win->width - 2;
    int inner_h = win->height - 2;
    if (n <= 0) n = 1;
    if (n >= inner_h) {
        clear_entire_window(win);
        return;
    }
    for (int r = 0; r < inner_h - n; r++) {
        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = win->text[r + n][c];
        }
    }
    for (int r = inner_h - n; r < inner_h; r++) {
        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = ' ';
        }
    }
}

// 2. 🔥 RENAMED: panes.h-உடன் மோதாமல் இருக்க parser_scroll_down என மாற்றப்பட்டுள்ளது:
static void parser_scroll_down(FloatingWindow *win, int n) {
    int inner_w = win->width - 2;
    int inner_h = win->height - 2;
    if (n <= 0) n = 1;
    if (n >= inner_h) {
        clear_entire_window(win);
        return;
    }
    for (int r = inner_h - 1; r >= n; r--) {
        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = win->text[r - n][c];
        }
    }
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = ' ';
        }
    }
}

// 3. DELETE LINES (\033[M): Nano/Vim-ல் வரிகளை மேலே இழுக்க உதவும் முக்கிய பங்க்ஷன்:
static void delete_lines(FloatingWindow *win, int n) {
    int inner_w = win->width - 2;
    int inner_h = win->height - 2;
    if (n <= 0) n = 1;
    int start_r = win->cur_r;
    for (int r = start_r; r < inner_h - n; r++) {
        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = win->text[r + n][c];
        }
    }
    for (int r = inner_h - n; r < inner_h; r++) {
        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = ' ';
        }
    }
}

// 4. INSERT LINES (\033[L): Nano/Vim-ல் புதிய வரிகளை செருக உதவும் முக்கிய பங்க்ஷன்:
static void insert_lines(FloatingWindow *win, int n) {
    int inner_w = win->width - 2;
    int inner_h = win->height - 2;
    if (n <= 0) n = 1;
    int start_r = win->cur_r;
    for (int r = inner_h - 1; r >= start_r + n; r--) {
        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = win->text[r - n][c];
        }
    }
    for (int r = start_r; r < start_r + n && r < inner_h; r++) {
        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = ' ';
        }
    }
}

void parser_feed_char(AnsiParser *parser, VirtualScreen *scr, FloatingWindow *win, char ch) {
    int inner_w = win->width - 2;
    int inner_h = win->height - 2;

    switch (parser->state) {
        case STATE_NORMAL:
            if (ch == '\033') {
                parser->state = STATE_ESC;
            } 
            else if (ch == '\r') {
                win->cur_c = 0;
            } 
            else if (ch == '\n') {
                win->cur_r++;
                if (win->cur_r >= inner_h) {
                    parser_scroll_up(win, 1);
                    win->cur_r = inner_h - 1;
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
            else if (ch == ']') {
                parser->state = STATE_OSC;
            }
            else if (ch == 'c') {
                clear_entire_window(win);
                parser->state = STATE_NORMAL;
            }
            else if (ch == '(' || ch == ')' || ch == '*' || ch == '+' || ch == '=' || ch == '>') {
                parser->state = STATE_CHARSET;
            }
            else if (ch == 'D') {
                win->cur_r++;
                if (win->cur_r >= inner_h) {
                    parser_scroll_up(win, 1);
                    win->cur_r = inner_h - 1;
                }
                parser->state = STATE_NORMAL;
            }
            else if (ch == 'M') {
                win->cur_r--;
                if (win->cur_r < 0) {
                    parser_scroll_down(win, 1);
                    win->cur_r = 0;
                }
                parser->state = STATE_NORMAL;
            }
            else if (ch == '7' || ch == '8' || ch == 'E' || ch == 'H') {
                parser->state = STATE_NORMAL;
            }
            else {
                parser->state = STATE_NORMAL;
            }
            break;

        case STATE_CHARSET:
            parser->state = STATE_NORMAL;
            break;

        case STATE_OSC:
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

                if (ch == 'A') {
                    win->cur_r -= n;
                    if (win->cur_r < 0) win->cur_r = 0;
                }
                else if (ch == 'B') {
                    win->cur_r += n;
                    if (win->cur_r >= inner_h) win->cur_r = inner_h - 1;
                }
                else if (ch == 'C') {
                    win->cur_c += n;
                    if (win->cur_c >= inner_w) win->cur_c = inner_w - 1;
                }
                else if (ch == 'D') {
                    win->cur_c -= n;
                    if (win->cur_c < 0) win->cur_c = 0;
                }
                else if (ch == 'G') {
                    int col = (parser->args[0] > 0) ? parser->args[0] - 1 : 0;
                    if (col < 0) col = 0;
                    if (col >= inner_w) col = inner_w - 1;
                    win->cur_c = col;
                }
                else if (ch == 'd') {
                    int row = (parser->args[0] > 0) ? parser->args[0] - 1 : 0;
                    if (row < 0) row = 0;
                    if (row >= inner_h) row = inner_h - 1;
                    win->cur_r = row;
                }
                else if (ch == 'H' || ch == 'f') {
                    int row = (parser->args[0] > 0) ? parser->args[0] - 1 : 0;
                    int col = (parser->arg_count > 1 && parser->args[1] > 0) ? parser->args[1] - 1 : 0;
                    if (row < 0) row = 0;
                    if (row >= inner_h) row = inner_h - 1;
                    if (col < 0) col = 0;
                    if (col >= inner_w) col = inner_w - 1;
                    win->cur_r = row;
                    win->cur_c = col;
                }
                else if (ch == 'L') {
                    insert_lines(win, n);
                }
                else if (ch == 'M') {
                    delete_lines(win, n);
                }
                else if (ch == 'S') {
                    parser_scroll_up(win, n);
                }
                else if (ch == 'T') {
                    parser_scroll_down(win, n);
                }
                else if (ch == 'K') {
                    int mode = parser->args[0];
                    if (mode == 0) {
                        for (int c = win->cur_c; c < inner_w && c < WIN_MAX_COLS; c++) {
                            win->text[win->cur_r][c] = ' ';
                        }
                    } else if (mode == 1) {
                        for (int c = 0; c <= win->cur_c && c < inner_w && c < WIN_MAX_COLS; c++) {
                            win->text[win->cur_r][c] = ' ';
                        }
                    } else if (mode == 2) {
                        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
                            win->text[win->cur_r][c] = ' ';
                        }
                    }
                }
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
