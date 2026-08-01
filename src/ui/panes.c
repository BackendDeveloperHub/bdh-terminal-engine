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
    win->is_active = 1;
    win->cur_r = 0; // கர்சர் முதல் வரியில் தொடங்கும்
    win->cur_c = 0; // கர்சர் முதல் காலத்தில் தொடங்கும்
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->title[sizeof(win->title) - 1] = '\0';
    return win;
}

void window_draw(VirtualScreen *scr, FloatingWindow *win) {
    int start_r = win->x;
    int end_r = win->x + win->height - 1;
    int start_c = win->y;
    int end_c = win->y + win->width - 1;

    // மூலைகள் & பார்டர்கள்
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
        for (int c = start_c + 1; c < end_c; c++) {
            screen_put_char(scr, r, c, ' ');
        }
    }

    int title_len = strlen(win->title);
    int title_pos = start_c + (win->width - title_len) / 2;
    for (int i = 0; i < title_len && (title_pos + i) < end_c; i++) {
        screen_put_char(scr, start_r, title_pos + i, win->title[i]);
    }
}

// Bash PTY எழுத்துக்களை விண்டோ உள்ளே எழுதும் Core Engine Function
void window_put_char(VirtualScreen *scr, FloatingWindow *win, char ch) {
    int inner_width = win->width - 2;   // பார்டரைத் தவிர்த்து உள்ளே உள்ள அகலம்
    int inner_height = win->height - 2; // பார்டரைத் தவிர்த்து உள்ளே உள்ள உயரம்

    if (ch == '\r') {
        win->cur_c = 0; // Carriage Return - வரியின் ஆரம்பத்திற்கு வர
        return;
    }
    if (ch == '\n') {
        win->cur_r++;   // New Line - அடுத்த வரிக்கு செல்ல
        win->cur_c = 0;
        if (win->cur_r >= inner_height) {
            win->cur_r = 0; // MVP-க்காக இப்போதைக்கு மேலே இருந்து மீண்டும் தொடங்கும் (Wrap around)
        }
        return;
    }

    // சாதாரண எழுத்துக்களை (ASCII printable chars) விண்டோ கட்டத்திற்குள் பிரிண்ட் செய்ய
    if (ch >= 32 && ch <= 126) {
        int abs_row = win->x + 1 + win->cur_r;
        int abs_col = win->y + 1 + win->cur_c;
        screen_put_char(scr, abs_row, abs_col, ch);

        win->cur_c++;
        if (win->cur_c >= inner_width) {
            win->cur_c = 0;
            win->cur_r++;
            if (win->cur_r >= inner_height) {
                win->cur_r = 0;
            }
        }
    }
}

void window_destroy(FloatingWindow *win) {
    free(win);
}
