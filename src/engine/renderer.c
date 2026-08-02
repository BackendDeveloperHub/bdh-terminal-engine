// src/engine/renderer.c
#include "renderer.h"
#include <stdio.h>

typedef struct {
    int id;
    int master_fd;
    int pid;
    FloatingWindow *win;
    void *parser;
} SessionRef;

void renderer_draw_all(VirtualScreen *scr, void *sessions_ptr, int count) {
    SessionRef *sessions = (SessionRef*)sessions_ptr;
    
    cursor_hide(); // 1. ரெண்டர் செய்யும் போது கர்சர் துள்ளுவதைத் தடுக்க
    screen_clear(scr);

    // 2. எந்த டேப் Active-ஆக (is_active == 1) இருக்கிறதோ அதை மட்டும் வரைதல்!
    // பின்னாடி இருக்கும் டேப்கள் (Inactive Tabs) வரையப்படாது!
    FloatingWindow *active_win = NULL;
    for (int i = 0; i < count; i++) {
        if (sessions[i].win->is_active == 1) {
            window_draw(scr, sessions[i].win);
            active_win = sessions[i].win;
            break; // Active டேப்பை வரைந்ததும் லூப்பை முடித்துவிடலாம்
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
