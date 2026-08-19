// src/engine/renderer.c - BDH Pure Linux CLI Multiplexer Renderer (True Delta Flush & Cursor Fixed)
#include "engine/renderer.h"
#include "engine/session.h"
#include "engine/cursor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct abuf {
    char *b;
    int len;
    int cap;
};
#define ABUF_INIT {NULL, 0, 0}

static void abAppend(struct abuf *ab, const char *s, int len) {
    if (ab->len + len >= ab->cap) {
        int new_cap = (ab->cap == 0) ? 8192 : ab->cap * 2;
        while (new_cap < ab->len + len) new_cap *= 2;
        char *new_buf = (char*)realloc(ab->b, new_cap);
        if (!new_buf) return;
        ab->b = new_buf;
        ab->cap = new_cap;
    }
    memcpy(&ab->b[ab->len], s, len);
    ab->len += len;
}

static void abFree(struct abuf *ab) {
    if (ab->b) {
        free(ab->b);
        ab->b = NULL;
    }
    ab->len = 0;
    ab->cap = 0;
}

void renderer_draw_all(VirtualScreen *scr, void *sessions_ptr, int count) {
    if (!scr || !scr->grid || !scr->old_grid || !sessions_ptr || count <= 0) {
        return;
    }

    TerminalSession *sessions = (TerminalSession*)sessions_ptr;
    
    // கர்சர் ஒத்திசைவிற்காக (Cursor Sync) Active Window-ஐக் கண்டறிதல்
    FloatingWindow *active_win = NULL;
    for (int i = 0; i < count; i++) {
        if (sessions[i].is_alive && sessions[i].win != NULL && sessions[i].win->is_active == 1) {
            active_win = sessions[i].win;
            break; 
        }
    }

    struct abuf ab = ABUF_INIT;
    
    // ரெண்டரிங் நடக்கும் போது ஸ்கிரீன் அதிராமல் இருக்க கர்சரை தற்காலிகமாக மறைக்கிறோம்
    abAppend(&ab, "\033[?25l", 6); 

    int cursor_r = -1;
    int cursor_c = -1;
    int current_fg = -1;
    int current_bg = -1;

    // --- 🔥 HIGH-PERFORMANCE DELTA ENGINE ---
    for (int r = 0; r < scr->rows; r++) {
        for (int c = 0; c < scr->cols; c++) {
            
            ScreenCell new_cell = scr->grid[r][c];
            ScreenCell old_cell = scr->old_grid[r][c];

            // எழுத்து அல்லது நிறத்தில் மாற்றம் இருந்தால் மட்டுமே அப்டேட் செய்தல்
            if (new_cell.ch != old_cell.ch || 
                new_cell.fg_color != old_cell.fg_color || 
                new_cell.bg_color != old_cell.bg_color) {
                
                // 1. கர்சரை தேவையான இடத்திற்கு நகர்த்துதல்
                if (cursor_r != r || cursor_c != c) {
                    char move_buf[32];
                    int len = snprintf(move_buf, sizeof(move_buf), "\033[%d;%dH", r + 1, c + 1);
                    abAppend(&ab, move_buf, len);
                }

                // 2. நிற மாற்றங்களை 256-Color ANSI வடிவில் அப்ளை செய்தல்
                if (new_cell.fg_color != current_fg || new_cell.bg_color != current_bg) {
                    char color_buf[64];
                    int clen = 0;
                    
                    if (new_cell.fg_color == 7 && new_cell.bg_color == 0) {
                        clen = snprintf(color_buf, sizeof(color_buf), "\033[0m");
                    } else {
                        clen = snprintf(color_buf, sizeof(color_buf), "\033[38;5;%dm\033[48;5;%dm", 
                                        new_cell.fg_color, new_cell.bg_color);
                    }
                    abAppend(&ab, color_buf, clen);
                    current_fg = new_cell.fg_color;
                    current_bg = new_cell.bg_color;
                }

                // 3. எழுத்தை ரெண்டர் செய்தல்
                char out_ch = (new_cell.ch == '\0' || new_cell.ch == 0) ? ' ' : new_cell.ch;
                abAppend(&ab, &out_ch, 1);

                // 4. old_grid-ஐ ஒத்திசைத்தல்
                scr->old_grid[r][c] = new_cell;

                // 5. அடுத்த கர்சர் நிலையை டிராக் செய்தல்
                cursor_r = r;
                cursor_c = c + 1; 
            }
        }
    }

    // நிறங்களை இயல்பு நிலைக்கு மீட்டமைத்தல்
    if (current_fg != -1 || current_bg != -1) {
        abAppend(&ab, "\033[0m", 4);
    }

    // மொத்த டெல்டா பஃபரையும் ஒரே ஃப்ளஷ்ஷில் டெர்மினலுக்கு அனுப்புதல்
    if (ab.len > 0) {
        write(STDOUT_FILENO, ab.b, ab.len);
    }
    abFree(&ab);

    // =========================================================================
    // 🔥 THE ARCHITECT FIX: கர்சரை விண்டோவின் சரியான உள் பொசிஷனுக்கு ஒத்திசைத்தல்
    // =========================================================================
    if (active_win != NULL) {
        if (active_win->scroll_offset > 0) {
            // ஸ்க்ரோல் மோடில் இருக்கும்போது கர்சரை மறைத்துவிட வேண்டும் (Clean UX)
            write(STDOUT_FILENO, "\033[?25l", 6); 
        } else {
            // panes.c-ல் நாம் பயன்படுத்திய அதே விண்டோ பார்டர் அளவுகள்
            int start_r = 1;  
            int start_c = 0;  

            // +1 for inner window offset (border-ஐத் தாண்ட), +1 for 1-based ANSI terminal index
            int ansi_row = start_r + 1 + active_win->cur_r + 1;
            int ansi_col = start_c + 1 + active_win->cur_c + 1;

            char move_buf[32];
            int len = snprintf(move_buf, sizeof(move_buf), "\033[%d;%dH", ansi_row, ansi_col);
            write(STDOUT_FILENO, move_buf, len);
            
            // கர்சரை மீண்டும் ஒளிரச் செய்தல் (Block Cursor Style: \033[2 q)
            write(STDOUT_FILENO, "\033[?25h\033[2 q", 11); 
        }
    } else {
        char move_buf[32];
        int len = snprintf(move_buf, sizeof(move_buf), "\033[%d;%dH", scr->rows, 1);
        write(STDOUT_FILENO, move_buf, len);
        write(STDOUT_FILENO, "\033[?25h\033[1 q", 11);
    }
}
