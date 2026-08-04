// src/engine/renderer.c - BDH Pure Linux CLI Multiplexer Renderer (100% Segfault Fixed + Matrix Green Support)
#include "renderer.h"
#include "engine/session.h"  // <-- FIX 1: டூப்ளிகேட் Struct-ஐ நீக்கிவிட்டு அசல் ஹெட்டரை இணைத்துள்ளோம்!
#include <stdio.h>

#define COLOR_GREEN 2  // <-- ADDED: பச்சை நிறத்திற்கான குறியீடு

void renderer_draw_all(VirtualScreen *scr, void *sessions_ptr, int count) {
    // 1. பாதுகாப்பு அரண்: Screen அல்லது Sessions பாயிண்டர் NULL ஆக இருந்தால் கிராஷ் ஆகாமல் திரும்பவும்
    if (!scr || !scr->grid || !sessions_ptr || count <= 0) {
        return;
    }

    // --- FIX 2: SessionRef-க்கு பதிலாக அசல் TerminalSession* பயன்படுத்துதல் ---
    TerminalSession *sessions = (TerminalSession*)sessions_ptr;
    
    cursor_hide(); // ரெண்டர் செய்யும் போது கர்சர் துள்ளுவதைத் தடுக்க
    screen_clear(scr);

    // 2. எந்த டேப் Active-ஆக (is_active == 1) இருக்கிறதோ அதை மட்டும் வரைதல்!
    FloatingWindow *active_win = NULL;
    for (int i = 0; i < count; i++) {
        // --- FIX 3: win != NULL மற்றும் is_alive என்பதை செக் செய்த பிறகே is_active-ஐத் தொட வேண்டும்! ---
        if (sessions[i].is_alive && sessions[i].win != NULL && sessions[i].win->is_active == 1) {
            window_draw(scr, sessions[i].win);
            active_win = sessions[i].win;
            break; // Active டேப்பை வரைந்ததும் லூப்பை முடித்துவிடலாம்
        }
    }

    // 3. Virtual Screen பஃபரை டெர்மினல் அவுட்புட்டுக்கு அனுப்புதல் (With ANSI Color Support!)
    printf("\033[2J\033[H");
    for (int r = 0; r < scr->rows; r++) {
        if (!scr->grid[r]) continue; // Safety check for row buffer
        for (int c = 0; c < scr->cols; c++) {
            ScreenCell cell = scr->grid[r][c];

            // --- ADDED: பச்சை நிறம் (COLOR_GREEN == 2) செக் செய்து பிரிண்ட் செய்தல் ---
            if (cell.fg_color == COLOR_GREEN || cell.fg_color == 2) {
                // \033[1;32m = Bold Bright Green | \033[0m = Reset Color
                printf("\033[1;32m%c\033[0m", cell.ch);
            } else {
                // வழக்கமான நிறம் (Default Terminal Color)
                putchar(cell.ch);
            }
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
