// src/ui/panes.c
#include "panes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FloatingWindow* window_create(int id, int x, int y, int width, int height, const char* title, int z_index) {
    FloatingWindow *win = (FloatingWindow*)malloc(sizeof(FloatingWindow));
    win->id = id;
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->z_index = z_index;
    
    // z_index == 1 ஆக இருந்தால் மட்டுமே Active (Bug Fixed):
    win->is_active = (z_index == 1) ? 1 : 0;
    
    win->cur_r = 0;
    win->cur_c = 0;
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->title[sizeof(win->title) - 1] = '\0';

    // ஆரம்பத்தில் விண்டோ மெமரியை காலியாக்குதல் (Spaces)
    for (int r = 0; r < WIN_MAX_ROWS; r++) {
        for (int c = 0; c < WIN_MAX_COLS; c++) {
            win->text[r][c] = ' ';
        }
    }
    return win;
}

// 1. விண்டோ மெமரியை (win->text) ஒரு வரி மேலே நகர்த்தும் பங்க்ஷன் (Scroll Up):
void window_scroll_up(FloatingWindow *win) {
    int inner_height = win->height - 2;
    int inner_width  = win->width - 2;

    // Row 1 முதல் கடைசி வரை உள்ள எழுத்துக்களை ஒரு வரி மேலே (Row 0-க்கு) நகர்த்துதல்:
    for (int r = 0; r < inner_height - 1 && r < WIN_MAX_ROWS - 1; r++) {
        for (int c = 0; c < inner_width && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = win->text[r + 1][c];
        }
    }

    // கடைசி வரியை (Bottom row) காலியாக்குதல் (Spaces):
    int last_r = inner_height - 1;
    if (last_r < WIN_MAX_ROWS) {
        for (int c = 0; c < inner_width && c < WIN_MAX_COLS; c++) {
            win->text[last_r][c] = ' ';
        }
    }
}

void window_draw(VirtualScreen *scr, FloatingWindow *win) {
    int start_r = win->x;
    int end_r = win->x + win->height - 1;
    int start_c = win->y;
    int end_c = win->y + win->width - 1;

    // 1. மூலைகள் & பார்டர்கள்
    screen_put_char(scr, start_r, start_c, '+');
    screen_put_char(scr, start_r, end_c, '+');
    screen_put_char(scr, end_r, start_c, '+');
    screen_put_char(scr, end_r, end_c, '+');

    for (int c = start_c + 1; c < end_c; c++) {
        screen_put_char(scr, start_r, c, '-');
        screen_put_char(scr, end_r, c, '-');
    }

    for (int r = start_r + 1; r < end_r; r++) {
        screen_put_char(scr, r, start_c, '|');
        screen_put_char(scr, r, end_c, '|');
    }

    // 2. விண்டோவின் உள் எழுத்துக்களை (Backing Store Text) வரைதல்
    int inner_w = win->width - 2;
    int inner_h = win->height - 2;
    for (int r = 0; r < inner_h && r < WIN_MAX_ROWS; r++) {
        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            screen_put_char(scr, start_r + 1 + r, start_c + 1 + c, win->text[r][c]);
        }
    }

    // 3. விண்டோ தலைப்பு (Title Bar)
    int title_len = strlen(win->title);
    int title_pos = start_c + (win->width - title_len) / 2;
    for (int i = 0; i < title_len && (title_pos + i) < end_c; i++) {
        screen_put_char(scr, start_r, title_pos + i, win->title[i]);
    }
}

// 2. மாற்றியமைக்கப்பட்ட window_put_char (Scrolling + Backspace ஆதரவுடன்)
void window_put_char(VirtualScreen *scr, FloatingWindow *win, char ch) {
    int inner_width = win->width - 2;
    int inner_height = win->height - 2;

    if (ch == '\r') {
        win->cur_c = 0;
        return;
    }
    if (ch == '\n') {
        win->cur_r++;
        if (win->cur_r >= inner_height) {
            window_scroll_up(win);
            win->cur_r = inner_height - 1;
        }
        return;
    }

    // Backspace ஆதரவு
    if (ch == '\b' || ch == 127) {
        if (win->cur_c > 0) {
            win->cur_c--;
            if (win->cur_r < WIN_MAX_ROWS && win->cur_c < WIN_MAX_COLS) {
                win->text[win->cur_r][win->cur_c] = ' ';
            }
        }
        return;
    }

    if (ch >= 32 && ch <= 126) {
        if (win->cur_r < WIN_MAX_ROWS && win->cur_c < WIN_MAX_COLS) {
            win->text[win->cur_r][win->cur_c] = ch;
        }

        win->cur_c++;
        if (win->cur_c >= inner_width) {
            win->cur_c = 0;
            win->cur_r++;
            if (win->cur_r >= inner_height) {
                window_scroll_up(win);
                win->cur_r = inner_height - 1;
            }
        }
    }
}

void window_destroy(FloatingWindow *win) {
    free(win);
}
