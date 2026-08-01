// src/ui/panes.h
#ifndef BDH_PANES_H
#define BDH_PANES_H

#include "engine/screen.h"

// மிதக்கும் விண்டோ (Floating Window) அமைப்பு
typedef struct {
    int id;
    int x, y;          // விண்டோ தொடங்கும் இடம் (Row, Col)
    int width, height; // அகலம் மற்றும் உயரம்
    int z_index;       // அடுக்கு வரிசை (0 = பின்னால், 1 = முன்னால்)
    char title[64];    // விண்டோ தலைப்பு
    int is_active;     // Active விண்டோவா? (Highlight செய்ய)
} FloatingWindow;

// விண்டோ உருவாக்கும் மற்றும் வரையும் பங்க்ஷன்கள்
FloatingWindow* window_create(int id, int x, int y, int width, int height, const char* title, int z_index);
void window_draw(VirtualScreen *scr, FloatingWindow *win);
void window_destroy(FloatingWindow *win);

#endif // BDH_PANES_H
