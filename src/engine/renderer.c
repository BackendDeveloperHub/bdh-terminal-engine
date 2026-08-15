// src/engine/renderer.c - BDH Pure Linux CLI Multiplexer Renderer (Delta Rendering / Diff Update Version)
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

static void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);
    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

static void abFree(struct abuf *ab) {
    free(ab->b);
}
// -----------------------------------------------------------------

void renderer_draw_all(VirtualScreen *scr, void *sessions_ptr, int count) {
    if (!scr || !scr->grid || !scr->old_grid || !sessions_ptr || count <= 0) {
        return;
    }

    TerminalSession *sessions = (TerminalSession*)sessions_ptr;
    
    // 1. புதிய ஃபிரேமுக்காக (Back buffer) மெமரியை க்ளியர் செய்கிறோம்
    screen_clear(scr);

    // 2. Active Session-ன் அவுட்புட்டை புதிய ஃபிரேமில் (scr->grid) வரைகிறோம்
    FloatingWindow *active_win = NULL;
    for (int i = 0; i < count; i++) {
        if (sessions[i].is_alive && sessions[i].win != NULL && sessions[i].win->is_active == 1) {
            window_draw(scr, sessions[i].win);
            active_win = sessions[i].win;
            break; 
        }
    }

    struct abuf ab = ABUF_INIT;
    
    // கர்சரை மறைக்கிறோம் (பார்வையாளர்களுக்கு கர்சர் தாவுவது தெரியாமல் இருக்க)
    abAppend(&ab, "\033[?25l", 6); 

    int cursor_r = -1;
    int cursor_c = -1;
    int current_color = -1;

    // --- 🔥 DELTA RENDERING ENGINE: மாறியதை மட்டும் பிரிண்ட் செய்யும் மேஜிக்! ---
    for (int r = 0; r < scr->rows; r++) {
        for (int c = 0; c < scr->cols; c++) {
            
            ScreenCell new_cell = scr->grid[r][c];
            ScreenCell old_cell = scr->old_grid[r][c];

            // பழைய செல்லுக்கும் புதிய செல்லுக்கும் வித்தியாசம் இருக்கிறதா என்று சரிபார்த்தல்
            if (new_cell.ch != old_cell.ch || new_cell.fg_color != old_cell.fg_color) {
                
                // 1. கர்சர் அந்த இடத்தில் இல்லையென்றால் மட்டும் நகர்த்தவும் (Optimization)
                if (cursor_r != r || cursor_c != c) {
                    char move_buf[32];
                    int len = snprintf(move_buf, sizeof(move_buf), "\033[%d;%dH", r + 1, c + 1);
                    abAppend(&ab, move_buf, len);
                }

                // 2. நிறம் மாறியிருந்தால் அப்டேட் செய்யவும்
                if (new_cell.fg_color != current_color) {
                    if (new_cell.fg_color == COLOR_GREEN || new_cell.fg_color == 2) {
                        abAppend(&ab, "\033[1;32m", 7);
                    } else {
                        abAppend(&ab, "\033[0m", 4);
                    }
                    current_color = new_cell.fg_color;
                }

                // 3. அந்த ஒரு எழுத்தை மட்டும் பிரிண்ட் செய்யவும்
                abAppend(&ab, &new_cell.ch, 1);

                // 4. old_grid-ஐ புதிய டேட்டாவுடன் ஒத்திசைக்கவும் (Sync)
                scr->old_grid[r][c] = new_cell;

                // 5. கர்சர் நிலை கண்காணிப்பு (Cursor naturally moves 1 step right after printing)
                cursor_r = r;
                cursor_c = c + 1; 
            }
        }
    }

    // நிறம் கடைசியாக மாறாமல் இருந்தால் அதை ரீசெட் செய்தல்
    if (current_color != -1 && current_color != 7 && current_color != 0) {
        abAppend(&ab, "\033[0m", 4);
    }

    // ஒட்டுமொத்த மாற்றங்களையும் ஒரே அடியில் ஸ்கிரீனுக்கு அனுப்புகிறோம்
    if (ab.len > 0) {
        write(STDOUT_FILENO, ab.b, ab.len);
    }
    abFree(&ab);

    // Active விண்டோவில் கர்சரை சரியான இடத்தில் மீண்டும் நிலைநிறுத்துதல்
    if (active_win != NULL) {
        cursor_sync_to_window(active_win);
    } else {
        cursor_show();
    }
}
