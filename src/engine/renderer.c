// src/engine/renderer.c - BDH Pure Linux CLI Multiplexer Renderer (Anti-Glitch Double Buffering Version)
#include "renderer.h"
#include "engine/session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define COLOR_GREEN 2

// --- ADDED: Double Buffering-க்கான Append Buffer ஸ்ட்ரக்சர் ---
struct abuf {
    char *b;
    int len;
};
#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);
    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void abFree(struct abuf *ab) {
    free(ab->b);
}
// -----------------------------------------------------------------

void renderer_draw_all(VirtualScreen *scr, void *sessions_ptr, int count) {
    if (!scr || !scr->grid || !sessions_ptr || count <= 0) {
        return;
    }

    TerminalSession *sessions = (TerminalSession*)sessions_ptr;
    
    // cursor_hide(); // இதை Buffer உள்ளே அனுப்புவோம்
    screen_clear(scr);

    FloatingWindow *active_win = NULL;
    for (int i = 0; i < count; i++) {
        if (sessions[i].is_alive && sessions[i].win != NULL && sessions[i].win->is_active == 1) {
            window_draw(scr, sessions[i].win);
            active_win = sessions[i].win;
            break; 
        }
    }

    // --- FIX: ஸ்கிரீனை அழிப்பதற்கு (2J) பதிலாக, Buffer-ஐப் பயன்படுத்துகிறோம் ---
    struct abuf ab = ABUF_INIT;
    
    // கர்சரை மறைத்து, Top-Left (H) க்கு மட்டும் கொண்டு செல்கிறோம். (2J-ஐ நீக்கிவிட்டோம்!)
    abAppend(&ab, "\033[?25l", 6); 
    abAppend(&ab, "\033[H", 3);

    for (int r = 0; r < scr->rows; r++) {
        if (!scr->grid[r]) continue; 
        
        for (int c = 0; c < scr->cols; c++) {
            ScreenCell cell = scr->grid[r][c];

            if (cell.fg_color == COLOR_GREEN || cell.fg_color == 2) {
                abAppend(&ab, "\033[1;32m", 7);
                abAppend(&ab, &cell.ch, 1);
                abAppend(&ab, "\033[0m", 4);
            } else {
                abAppend(&ab, &cell.ch, 1);
            }
        }
        
        // --- ADDED: பழைய எழுத்துக்கள் மிச்சம் இருந்தால் அதை மட்டும் அழிக்க (Clear to end of line) ---
        abAppend(&ab, "\033[K", 3);
        
        if (r < scr->rows - 1) {
            abAppend(&ab, "\r\n", 2);
        }
    }

    // --- MAGIC HAPPENS HERE: ஒட்டுமொத்த ஸ்கிரீனையும் ஒரே அடியில் வரையச் செய்கிறோம்! ---
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);

    if (active_win != NULL) {
        cursor_sync_to_window(active_win);
    } else {
        cursor_show();
    }
    fflush(stdout);
}
