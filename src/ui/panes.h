// src/ui/panes.h
#ifndef BDH_PANES_H
#define BDH_PANES_H

#include "engine/screen.h"

#define WIN_MAX_ROWS 25
#define WIN_MAX_COLS 80

typedef struct {
    int id;
    int x, y;          
    int width, height; 
    int z_index;       
    char title[64];    
    int is_active;     
    int cur_r, cur_c;  
    char text[WIN_MAX_ROWS][WIN_MAX_COLS]; // <-- விண்டோவின் சொந்த மெமரி (Backing Store)
} FloatingWindow;

FloatingWindow* window_create(int id, int x, int y, int width, int height, const char* title, int z_index);
void window_draw(VirtualScreen *scr, FloatingWindow *win);
void window_put_char(VirtualScreen *scr, FloatingWindow *win, char ch);
void window_destroy(FloatingWindow *win);

#endif // BDH_PANES_H
