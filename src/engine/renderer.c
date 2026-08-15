// src/engine/renderer.c - BDH Pure Linux CLI Multiplexer Renderer (True Delta Flush Version)
#include "renderer.h"
#include "engine/session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    if (!scr || !scr->grid || !scr->old_grid || !sessions_ptr) {
        return;
    }

    TerminalSession *sessions = (TerminalSession*)sessions_ptr;
    
    // கர்சர் சிங்கிங்கிற்காக (Cursor Sync) Active Window-ஐ கண்டுபிடிக்கிறோம்
    FloatingWindow *active_win = NULL;
    for (int i = 0; i < count; i++) {
        if (sessions[i].is_alive && sessions[i].win != NULL && sessions[i].win->is_active == 1) {
            active_win = sessions[i].win;
            break; 
        }
    }

    struct abuf ab = ABUF_INIT;
    
    // டெர்மினலில் வரையும்போது கர்சரை மறைக்கிறோம்
    abAppend(&ab, "\033[?25l", 6); 

    int cursor_r = -1;
    int cursor_c = -1;
    int current_color = -1;

    // --- 🔥 DELTA RENDERING ENGINE ---
    for (int r = 0; r < scr->rows; r++) {
        for (int c = 0; c < scr->cols; c++) {
            
            ScreenCell new_cell = scr->grid[r][c];
            ScreenCell old_cell = scr->old_grid[r][c];

            // பழைய ஸ்கிரீனுக்கும் புதிய ஸ்கிரீனுக்கும் வித்தியாசம் இருந்தால் மட்டுமே...
            if (new_cell.ch != old_cell.ch || new_cell.fg_color != old_cell.fg_color) {
                
                // 1. கர்சரை அந்த இடத்திற்கு நகர்த்துகிறோம்
                if (cursor_r != r || cursor_c != c) {
                    char move_buf[32];
                    int len = snprintf(move_buf, sizeof(move_buf), "\033[%d;%dH", r + 1, c + 1);
                    abAppend(&ab, move_buf, len);
                }

                // 2. நிறம் மாறியிருந்தால் அப்டேட் செய்கிறோம்
                if (new_cell.fg_color != current_color) {
                    if (new_cell.fg_color == 2) {
                        abAppend(&ab, "\033[1;32m", 7);
                    } else {
                        abAppend(&ab, "\033[0m", 4);
                    }
                    current_color = new_cell.fg_color;
                }

                // 3. 🔥 எழுத்தை பிரிண்ட் செய்கிறோம் (Empty character-ஐ ஸ்பேஸாக மாற்றுகிறோம்)
                char out_ch = (new_cell.ch == '\0' || new_cell.ch == 0) ? ' ' : new_cell.ch;
                abAppend(&ab, &out_ch, 1);

                // 4. old_grid-ஐ ஒத்திசைக்கிறோம்
                scr->old_grid[r][c] = new_cell;

                // 5. கர்சர் நிலை கண்காணிப்பு
                cursor_r = r;
                cursor_c = c + 1; 
            }
        }
    }

    // நிறத்தை ரீசெட் செய்கிறோம்
    if (current_color != -1 && current_color != 7 && current_color != 0) {
        abAppend(&ab, "\033[0m", 4);
    }

    // மொத்த டெல்டா பஃபரையும் ஒரே அடியில் டெர்மினலுக்கு அனுப்புகிறோம்
    if (ab.len > 0) {
        write(STDOUT_FILENO, ab.b, ab.len);
    }
    abFree(&ab);

    // கடைசியாக கர்சரை சரியான இடத்தில் நிலைநிறுத்துகிறோம்
    if (active_win != NULL) {
        cursor_sync_to_window(active_win);
    } else {
        // Fallback: கர்சரை அடிமட்டத்திற்கு கொண்டு செல்
        char move_buf[32];
        int len = snprintf(move_buf, sizeof(move_buf), "\033[%d;%dH", scr->rows, 1);
        write(STDOUT_FILENO, move_buf, len);
        write(STDOUT_FILENO, "\033[?25h\033[1 q", 11);
    }
}
