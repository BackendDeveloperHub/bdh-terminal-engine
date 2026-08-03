// src/engine/renderer.c - BDH Pure Linux CLI Multiplexer Renderer (Struct Mismatch Fixed)
#include "renderer.h"
#include "engine/session.h"  // <-- FIX 1: அசல் TerminalSession ஹெட்டரை இணைத்துள்ளோம்!
#include <stdio.h>

void renderer_draw_all(VirtualScreen *scr, void *sessions_ptr, int count) {
    // 1. பாதுகாப்பு அரண்: Screen அல்லது Sessions பாயிண்டர் NULL ஆக இருந்தால் உடனே திரும்புதல்
    if (!scr || !scr->grid || !sessions_ptr || count <= 0) {
        return;
    }

    // --- FIX 2: டூப்ளிகேட் SessionRef-க்கு பதிலாக அசல் TerminalSession* பயன்படுத்துதல் ---
    TerminalSession *sessions = (TerminalSession*)sessions_ptr;
    
    cursor_hide(); 
    screen_clear(scr);

    // 2. எந்த டேப் Active-ஆக உள்ளதோ அதை மட்டும் பாதுகாப்பாக வரைதல்
    FloatingWindow *active_win = NULL;
    for (int i = 0; i < count; i++) {
        // --- FIX 3: win பாயிண்டர் NULL ஆக இல்லையென்பதை 100% உறுதி செய்தல் ---
        if (sessions[i].win != NULL && sessions[i].win->is_active == 1) {
            window_draw(scr, sessions[i].win);
            active_win = sessions[i].win;
            break; 
        }
    }

    // 3. Virtual Screen பஃபரை டெர்மினல் அவுட்புட்டுக்கு அனுப்புதல்
    printf("\033[2J\033[H");
    for (int r = 0; r < scr->rows; r++) {
        if (!scr->grid[r]) continue; 
        
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
