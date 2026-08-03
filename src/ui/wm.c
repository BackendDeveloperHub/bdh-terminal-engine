// src/ui/wm.c - BDH Terminal Window Manager Module
#include "ui/wm.h"
#include <stdio.h>
#include <stdlib.h>

// 1. Window Manager-ஐ தொடக்க நிலையில் அமைத்தல் (Init)
void wm_init(WindowManager *wm) {
    if (!wm) return;
    
    wm->count = 0;
    wm->active_win = NULL;
    
    for (int i = 0; i < MAX_WINDOWS; i++) {
        wm->windows[i] = NULL;
    }
}

// 2. புதிய FloatingWindow-ஐ Window Manager-ல் சேர்த்தல்
int wm_add_window(WindowManager *wm, FloatingWindow *win) {
    if (!wm || !win || wm->count >= MAX_WINDOWS) {
        return 0; // Failure (Limit Reached or Null Pointer)
    }

    wm->windows[wm->count] = win;
    wm->count++;

    // சேர்க்கப்படும் முதல் விண்டோவாக இருந்தால், அதை Active Window-ஆக மாற்றுவோம்:
    if (wm->count == 1) {
        wm->active_win = win;
        win->is_active = 1;
        win->z_index = 1;
    } else {
        win->is_active = 0;
        win->z_index = 0;
    }

    return 1; // Success
}

// 3. குறிப்பிட்ட விண்டோவை Focus (Active) செய்தல் & Z-Index மாற்றுதல்
void wm_focus_window(WindowManager *wm, int win_id) {
    if (!wm || wm->count == 0) return;

    FloatingWindow *target_win = NULL;

    // எல்லா விண்டோக்களையும் Active நிலையில் இருந்து நீக்குதல்:
    for (int i = 0; i < wm->count; i++) {
        if (wm->windows[i]) {
            if (wm->windows[i]->id == win_id) {
                target_win = wm->windows[i];
            } else {
                wm->windows[i]->is_active = 0;
                wm->windows[i]->z_index = 0;
            }
        }
    }

    // தேர்ந்தெடுக்கப்பட்ட விண்டோவை Active செய்து Z-Index-ஐ உயர்த்துதல்:
    if (target_win) {
        target_win->is_active = 1;
        target_win->z_index = 1;
        wm->active_win = target_win;
    }
}

// 4. எல்லா விண்டோக்களையும் VirtualScreen-ல் வரைதல் (Z-Index Order-படி)
void wm_render_all(VirtualScreen *scr, WindowManager *wm) {
    if (!scr || !wm || wm->count == 0) return;

    // Pass 1: முதலில் Active அல்லாத விண்டோக்களை (Background / z_index == 0) வரைவோம்
    for (int i = 0; i < wm->count; i++) {
        if (wm->windows[i] && !wm->windows[i]->is_active) {
            window_draw(scr, wm->windows[i]);
        }
    }

    // Pass 2: கடைசியாக Active விண்டோவை (Foreground / z_index == 1) வரைவோம் 
    // (இதனால் Active விண்டோ எப்போதும் மற்றவற்றின் மேல் தெளிவாகத் தெரியும்!)
    if (wm->active_win && wm->active_win->is_active) {
        window_draw(scr, wm->active_win);
    }
}

// 5. எல்லா விண்டோ மெமரியையும் முழுமையாக அழித்தல் (Cleanup)
void wm_destroy_all(WindowManager *wm) {
    if (!wm) return;

    for (int i = 0; i < wm->count; i++) {
        if (wm->windows[i]) {
            window_destroy(wm->windows[i]);
            wm->windows[i] = NULL;
        }
    }

    wm->count = 0;
    wm->active_win = NULL;
}
