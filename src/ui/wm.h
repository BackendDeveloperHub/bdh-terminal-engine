// src/ui/wm.h
#ifndef BDH_WM_H
#define BDH_WM_H

#include "engine/screen.h"
#include "ui/panes.h"

#define MAX_WINDOWS 16

// விண்டோ மேனேஜர் அமைப்பு (Window Manager Struct)
typedef struct {
    FloatingWindow* windows[MAX_WINDOWS];
    int count;
    FloatingWindow* active_win; // தற்போதைய Active விண்டோ
} WindowManager;

// விண்டோ மேனேஜர் பங்க்ஷன்கள்
void wm_init(WindowManager *wm);
int wm_add_window(WindowManager *wm, FloatingWindow *win);
void wm_focus_window(WindowManager *wm, int win_id);
void wm_render_all(VirtualScreen *scr, WindowManager *wm);
void wm_destroy_all(WindowManager *wm);

#endif // BDH_WM_H
