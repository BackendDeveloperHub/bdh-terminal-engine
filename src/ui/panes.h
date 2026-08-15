// src/ui/panes.h - BDH Pure Linux CLI Multiplexer Panes Header (256-Color Support)
#ifndef BDH_PANES_H
#define BDH_PANES_H

#include "engine/screen.h"

// இந்த இரண்டு எண்களையும் பெரிய அளவாக மாற்றுங்கள்:
#define WIN_MAX_ROWS 150  
#define WIN_MAX_COLS 300  

typedef struct {
    int id;
    int x, y;
    int width, height;
    int z_index;
    int is_active;
    char title[64];
    
    char text[WIN_MAX_ROWS][WIN_MAX_COLS];
    unsigned char fg[WIN_MAX_ROWS][WIN_MAX_COLS]; // 🔥 எழுத்துக்களின் நிறம் (Foreground)
    unsigned char bg[WIN_MAX_ROWS][WIN_MAX_COLS]; // 🔥 பின்னணி நிறம் (Background)
    
    int cur_r, cur_c;
} FloatingWindow;

FloatingWindow* window_create(int id, int x, int y, int width, int height, const char* title, int z_index);
void window_draw(VirtualScreen *scr, FloatingWindow *win);

// 🔥 புதிய அப்டேட்: எழுத்துக்களோடு சேர்த்து கலர்களையும் வாங்கும் ஃபங்ஷன்
void window_put_char(VirtualScreen *scr, FloatingWindow *win, char ch, int fg, int bg);

void window_scroll_up(FloatingWindow *win);
void window_destroy(FloatingWindow *win);

#endif // BDH_PANES_H
