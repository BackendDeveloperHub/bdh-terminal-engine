// src/engine/renderer.c - BDH Pure Linux CLI Multiplexer Renderer (Signal 11 Fixed)
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
    // 1. பாதுகாப்பு அரண்: Screen அல்லது Sessions பாயிண்டர் NULL ஆக இருந்தால் கிராஷ் ஆகாமல் திரும்பவும்
    if (!scr || !scr->grid || !sessions_ptr || count <= 0) {
        return;
    }

    SessionRef *sessions = (SessionRef*)sessions_ptr;
    
    cursor_hide(); // ரெண்டர் செய்யும் போது கர்சர் துள்ளுவதைத் தடுக்க
    screen_clear(scr);

    // 2. எந்த டேப் Active-ஆக (is_active == 1) இருக்கிறதோ அதை மட்டும் வரைதல்!
    FloatingWindow *active_win = NULL;
    for (int i = 0; i < count; i++) {
        // --- FIX: win != NULL என்பதை செக் செய்த பிறகே is_active-ஐத் தொட வேண்டும்! (Signal 11 Fix) ---
        if (sessions[i].win != NULL && sessions[i].win->is_active == 1) {
            window_draw(scr, sessions[i].win);
            active_win = sessions[i].win;
            break; // Active டேப்பை வரைந்ததும் லூப்பை முடித்துவிடலாம்
        }
    }

    // 3. Virtual Screen பஃபரை டெர்மினல் அவுட்புட்டுக்கு அனுப்புதல்
    printf("\033[2J\033[H");
    for (int r = 0; r < scr->rows; r++) {
        if (!scr->grid[r]) continue; // Safety check for row buffer
        for (int c = 0; c < scr->cols; c++) {
            putchar(scr->grid[r][c].ch);
        }
        putchar('\r');
        putchar('\n');
    }

    // 4. Active விண்டோவின் உள்ளே மானிட்டர் கர்சரை ஒத்திசைத்தல்
    if (active_win != NULL) {
        cursor_sync_to_window(active_win);
    } else {
        cursor_show();
    }
    fflush(stdout);
}
