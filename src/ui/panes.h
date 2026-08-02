// src/ui/panes.h
#ifndef BDH_PANES_H
#define BDH_PANES_H

#include "engine/screen.h"

// இந்த இரண்டு எண்களையும் பெரிய அளவாக மாற்றுங்கள்:
#define WIN_MAX_ROWS 150  // <-- 150
#define WIN_MAX_COLS 300  // <-- 300 (இதுதான் முக்கியம்!)

typedef struct {
    int id;
    int x, y;
    int width, height;
    int z_index;
    int is_active;
    char title[64];
    char text[WIN_MAX_ROWS][WIN_MAX_COLS];
    int cur_r, cur_c;
} FloatingWindow;

FloatingWindow* window_create(int id, int x, int y, int width, int height, const char* title, int z_index);
void window_draw(VirtualScreen *scr, FloatingWindow *win);
void window_put_char(VirtualScreen *scr, FloatingWindow *win, char ch);
void window_scroll_up(FloatingWindow *win);
void window_destroy(FloatingWindow *win);

#endif // BDH_PANES_H
