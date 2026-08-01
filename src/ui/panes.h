// src/ui/panes.h
#ifndef BDH_PANES_H
#define BDH_PANES_H

#include "engine/screen.h"

typedef struct {
    int id;
    int x, y;          // விண்டோ தொடங்கும் இடம் (Row, Col)
    int width, height; // அகலம் மற்றும் உயரம்
    int z_index;       // அடுக்கு வரிசை
    char title[64];
    int is_active;
    int cur_r, cur_c;  // விண்டோவுக்கு உள்ளே கர்சர் இருக்கும் இடம் (Row, Col)
} FloatingWindow;

FloatingWindow* window_create(int id, int x, int y, int width, int height, const char* title, int z_index);
void window_draw(VirtualScreen *scr, FloatingWindow *win);
void window_put_char(VirtualScreen *scr, FloatingWindow *win, char ch);
void window_destroy(FloatingWindow *win);

#endif // BDH_PANES_H
