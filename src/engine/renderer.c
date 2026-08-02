// src/engine/renderer.c
#include "renderer.h"
#include <stdio.h>

// TerminalSession அமைப்பு main.c-ல் உள்ள அதே கட்டமைப்பைக் குறிக்கிறது
typedef struct {
    int id;
    int master_fd;
    int pid;
    FloatingWindow *win;
    void *parser;
} SessionRef;

void renderer_draw_all(VirtualScreen *scr, void *sessions_ptr, int count) {
    SessionRef *sessions = (SessionRef*)sessions_ptr;
    
    cursor_hide(); // ரெண்டர் செய்யும் போது கர்சர் துள்ளுவதைத் தடுக்க
    screen_clear(scr);

    // 1. முதலில் Z-Index 0 (பின்னால் இருக்கும் விண்டோ) வரைதல்
    for (int i = 0; i < count; i++) {
        if (sessions[i].win->z_index == 0) {
            window_draw(scr, sessions[i].win);
        }
    }

    // 2. அடுத்து Z-Index 1 (Active விண்டோ - முன்னால் மிதப்பது) வரைதல்
    FloatingWindow *active_win = NULL;
    for (int i = 0; i < count; i++) {
        if (sessions[i].win->z_index == 1) {
            window_draw(scr, sessions[i].win);
            active_win = sessions[i].win;
        }
    }

    // 3. Virtual Screen பஃபரை டெர்மினல் அவுட்புட்டுக்கு அனுப்புதல்
    printf("\033[2J\033[H");
    for (int r = 0; r < scr->rows; r++) {
        for (int c = 0; c < scr->cols; c++) {
            putchar(scr->grid[r][c].ch);
        }
        putchar('\r');
        putchar('\n');
    }

    // 4. Active விண்டோவின் உள்ளே மானிட்டர் கர்சரை ஒத்திசைத்தல்
    if (active_win) {
        cursor_sync_to_window(active_win);
    } else {
        cursor_show();
    }
    fflush(stdout);
}
