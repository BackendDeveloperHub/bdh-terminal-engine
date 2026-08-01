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
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->title[sizeof(win->title) - 1] = '\0';
    return win;
}

void window_draw(VirtualScreen *scr, FloatingWindow *win) {
    int start_r = win->x;
    int end_r = win->x + win->height - 1;
    int start_c = win->y;
    int end_c = win->y + win->width - 1;

    // 1. மூலைகள் (Corners)
    screen_put_char(scr, start_r, start_c, '+');
    screen_put_char(scr, start_r, end_c, '+');
    screen_put_char(scr, end_r, start_c, '+');
    screen_put_char(scr, end_r, end_c, '+');

    // 2. மேல் மற்றும் கீழ் கோடுகள்
    for (int c = start_c + 1; c < end_c; c++) {
        screen_put_char(scr, start_r, c, '-');
        screen_put_char(scr, end_r, c, '-');
    }

    // 3. இடது மற்றும் வலது கோடுகள் (மற்றும் உள்ளே காலியாக்குதல்)
    for (int r = start_r + 1; r < end_r; r++) {
        screen_put_char(scr, r, start_c, '|');
        screen_put_char(scr, r, end_c, '|');
        // விண்டோவுக்கு உள்ளே இருக்கும் பழைய எழுத்துக்களை மறைக்க (Override)
        for (int c = start_c + 1; c < end_c; c++) {
            screen_put_char(scr, r, c, ' ');
        }
    }

    // 4. விண்டோ தலைப்பு (Title Bar)
    int title_len = strlen(win->title);
    int title_pos = start_c + (win->width - title_len) / 2;
    for (int i = 0; i < title_len && (title_pos + i) < end_c; i++) {
        screen_put_char(scr, start_r, title_pos + i, win->title[i]);
    }
}

void window_destroy(FloatingWindow *win) {
    free(win);
}
