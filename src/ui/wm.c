// src/ui/wm.c
#include <stdio.h>
#include <stdlib.h>
#include "ui/wm.h"

// Window Manager-ஐ தொடங்குதல்
void wm_init(WindowManager *wm) {
    if (!wm) return;
    wm->count = 0;
    wm->active_win = NULL;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        wm->windows[i] = NULL;
    }
}

// புதிய விண்டோவை சேர்ப்பது
int wm_add_window(WindowManager *wm, FloatingWindow *win) {
    if (!wm || !win || wm->count >= MAX_WINDOWS) return -1;

    wm->windows[wm->count] = win;
    wm->count++;
    
    // கடைசியாக சேர்க்கப்பட்ட விண்டோவை Active ஆக்குதல்
    wm->active_win = win;
    win->is_active = 1;

    return 0;
}

// குறிப்பிட்ட ID கொண்ட விண்டோவை Active செய்து முன்னால் கொண்டு வருதல்
void wm_focus_window(WindowManager *wm, int win_id) {
    if (!wm) return;

    for (int i = 0; i < wm->count; i++) {
        if (wm->windows[i] && wm->windows[i]->id == win_id) {
            // பழைய Active விண்டோவை Deactivate செய்தல்
            if (wm->active_win) {
                wm->active_win->is_active = 0;
            }

            wm->active_win = wm->windows[i];
            wm->active_win->is_active = 1;

            // z_index-ஐ அதிகப்படுத்தி முன்னணிக்கு கொண்டு வருதல்
            wm->active_win->z_index = 100; 
            break;
        }
    }
}

// z_index அடிப்படையில் வரிசைப்படுத்தி அனைத்து விண்டோக்களையும் திரையில் வரைதல்
void wm_render_all(VirtualScreen *scr, WindowManager *wm) {
    if (!scr || !wm) return;

    // எளிய Bubble Sort மூலம் z_index வரிசைப்படுத்துதல் (குறைந்த z_index முதலில் வரையப்படும்)
    for (int i = 0; i < wm->count - 1; i++) {
        for (int j = 0; j < wm->count - i - 1; j++) {
            if (wm->windows[j]->z_index > wm->windows[j + 1]->z_index) {
                FloatingWindow *temp = wm->windows[j];
                wm->windows[j] = wm->windows[j + 1];
                wm->windows[j + 1] = temp;
            }
        }
    }

    // வரிசைப்படி அனைத்து விண்டோக்களையும் வரைதல்
    for (int i = 0; i < wm->count; i++) {
        if (wm->windows[i]) {
            window_draw(scr, wm->windows[i]);
        }
    }
}

// அனைத்து விண்டோக்களின் நினைவகத்தையும் விடுவித்தல்
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
