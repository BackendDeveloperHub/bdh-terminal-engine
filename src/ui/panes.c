// src/ui/panes.c - BDH Pure Linux CLI Multiplexer Windows & Panes (Scrollback Engine Edition)
#include "ui/panes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLOR_GREEN  2   
#define COLOR_RESET  0   

FloatingWindow* window_create(int id, int x, int y, int width, int height, const char* title, int z_index) {
    FloatingWindow *win = (FloatingWindow*)malloc(sizeof(FloatingWindow));
    win->id = id;
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->z_index = z_index;
    win->is_active = (z_index == 1) ? 1 : 0;
    
    win->cur_r = 0;
    win->cur_c = 0;
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->title[sizeof(win->title) - 1] = '\0';

    // தற்போதைய விண்டோ மெமரியை காலியாக்குதல்
    for (int r = 0; r < WIN_MAX_ROWS; r++) {
        for (int c = 0; c < WIN_MAX_COLS; c++) {
            win->text[r][c] = ' ';
            win->fg[r][c] = 7; 
            win->bg[r][c] = 0; 
        }
    }

    // ========================================================
    // 🔥 SCROLLBACK ENGINE: 1000 வரிகளுக்கான மெமரியை உருவாக்குதல்
    // ========================================================
    win->hist_text = (char**)malloc(SCROLLBACK_HISTORY_MAX * sizeof(char*));
    win->hist_fg = (unsigned char**)malloc(SCROLLBACK_HISTORY_MAX * sizeof(unsigned char*));
    win->hist_bg = (unsigned char**)malloc(SCROLLBACK_HISTORY_MAX * sizeof(unsigned char*));

    for (int i = 0; i < SCROLLBACK_HISTORY_MAX; i++) {
        win->hist_text[i] = (char*)malloc(WIN_MAX_COLS);
        win->hist_fg[i] = (unsigned char*)malloc(WIN_MAX_COLS);
        win->hist_bg[i] = (unsigned char*)malloc(WIN_MAX_COLS);
        memset(win->hist_text[i], ' ', WIN_MAX_COLS);
        memset(win->hist_fg[i], 7, WIN_MAX_COLS);
        memset(win->hist_bg[i], 0, WIN_MAX_COLS);
    }
    
    win->hist_head = 0;
    win->hist_count = 0;
    win->scroll_offset = 0;

    return win;
}

// 1. விண்டோ மெமரியை ஸ்க்ரோல் செய்யும் போது பழைய வரியை History-ல் சேமித்தல்:
void window_scroll_up(FloatingWindow *win) {
    int inner_height = win->height - 2;
    int inner_width  = win->width - 2;

    // 🔥 HISTORY BACKUP: மறையும் முதல் வரியை Ring Buffer-க்குள் தள்ளுகிறோம்
    memcpy(win->hist_text[win->hist_head], win->text[0], WIN_MAX_COLS);
    memcpy(win->hist_fg[win->hist_head], win->fg[0], WIN_MAX_COLS);
    memcpy(win->hist_bg[win->hist_head], win->bg[0], WIN_MAX_COLS);

    win->hist_head = (win->hist_head + 1) % SCROLLBACK_HISTORY_MAX;
    if (win->hist_count < SCROLLBACK_HISTORY_MAX) {
        win->hist_count++;
    }

    // வரிகளை மேலே நகர்த்துதல்
    for (int r = 0; r < inner_height - 1 && r < WIN_MAX_ROWS - 1; r++) {
        for (int c = 0; c < inner_width && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = win->text[r + 1][c];
            win->fg[r][c]   = win->fg[r + 1][c];
            win->bg[r][c]   = win->bg[r + 1][c];
        }
    }

    // கடைசி வரியை காலியாக்குதல்
    int last_r = inner_height - 1;
    if (last_r < WIN_MAX_ROWS) {
        for (int c = 0; c < inner_width && c < WIN_MAX_COLS; c++) {
            win->text[last_r][c] = ' ';
            win->fg[last_r][c]   = 7;
            win->bg[last_r][c]   = 0;
        }
    }
}

// 2. Responsive Border & Scrollback Drawing Function
void window_draw(VirtualScreen *scr, FloatingWindow *win) {
    int start_r = 1;                  
    int end_r   = scr->rows - 12;      
    int start_c = 0;                  
    int end_c   = scr->cols - 1;      

    if (end_r <= start_r + 2) end_r = scr->rows - 2;
    if (end_c <= start_c + 2) end_c = scr->cols - 1;

    // மூலைகள் (Corners)
    screen_put_char_color(scr, start_r, start_c, '+', COLOR_GREEN);
    screen_put_char_color(scr, start_r, end_c,   '+', COLOR_GREEN);
    screen_put_char_color(scr, end_r, start_c,   '+', COLOR_GREEN);
    screen_put_char_color(scr, end_r, end_c,     '+', COLOR_GREEN);

    // பார்டர் கோடுகள் (Borders)
    for (int c = start_c + 1; c < end_c; c++) {
        screen_put_char_color(scr, start_r, c, '=', COLOR_GREEN); 
        screen_put_char_color(scr, end_r, c, '-', COLOR_GREEN);   
    }

    for (int r = start_r + 1; r < end_r; r++) {
        screen_put_char_color(scr, r, start_c, '|', COLOR_GREEN);
        screen_put_char_color(scr, r, end_c,   '|', COLOR_GREEN);
    }

    // 🔥 SCROLLBACK RENDERER: பயனர் ஸ்க்ரோல் செய்திருந்தால் பழைய மெமரியைக் காட்டுதல்
    int inner_w = (end_c - start_c - 1);
    int inner_h = (end_r - start_r - 1);
    
    for (int r = 0; r < inner_h && r < WIN_MAX_ROWS; r++) {
        int history_index = -1; // -1 என்றால் தற்போதைய ஸ்க்ரீன் (Live buffer) என்று அர்த்தம்
        int current_row = r;

        if (win->scroll_offset > 0) {
            if (r < win->scroll_offset) {
                int offset_back = win->scroll_offset - r;
                if (offset_back <= win->hist_count) {
                    // Ring Buffer-ல் இருந்து சரியான வரியைக் கணக்கிட்டு எடுக்கும் மாயாஜாலம்!
                    history_index = (win->hist_head - offset_back + SCROLLBACK_HISTORY_MAX) % SCROLLBACK_HISTORY_MAX;
                }
            } else {
                current_row = r - win->scroll_offset;
            }
        }

        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            char ch; int fg;
            if (history_index != -1) {
                ch = win->hist_text[history_index][c];
                fg = win->hist_fg[history_index][c];
            } else {
                ch = win->text[current_row][c];
                fg = win->fg[current_row][c];
            }
            screen_put_char_color(scr, start_r + 1 + r, start_c + 1 + c, ch, fg);
        }
    }

    // Title Badge
    char badge[128];
    if (win->scroll_offset > 0) {
        snprintf(badge, sizeof(badge), "  %s [SCROLL: %d]  ", win->title, win->scroll_offset); 
    } else {
        snprintf(badge, sizeof(badge), "  %s  ", win->title); 
    }
    
    int badge_len = strlen(badge);
    int title_pos = start_c + 3; 

    for (int i = 0; i < badge_len && (title_pos + i) < end_c; i++) {
        // ஸ்க்ரோல் மோடில் இருந்தால் Title நிறம் மஞ்சளாக மாறும் (Visual Alert)
        int t_color = (win->scroll_offset > 0) ? 3 : COLOR_GREEN; 
        screen_put_char_color(scr, start_r, title_pos + i, badge[i], t_color);
    }
}

// 3. நிறங்களோடு எழுத்துக்களை வாங்கும் மாஸ்டர் பங்க்ஷன்
void window_put_char(VirtualScreen *scr, FloatingWindow *win, char ch, int fg, int bg) {
    (void)scr; 
    int inner_width = win->width - 2;
    int inner_height = win->height - 2;

    // 🔥 AUTO-SCROLL TO BOTTOM: புதிய அவுட்புட் வந்தால் தானாகவே அடிமட்டத்திற்கு வந்துவிடும்!
    win->scroll_offset = 0; 

    if (ch == '\r') {
        win->cur_c = 0;
        return;
    }
    
    if (ch == '\n') {
        win->cur_r++;
        if (win->cur_r >= inner_height) {
            window_scroll_up(win);
            win->cur_r = inner_height - 1;
        }
        return;
    }

    if (ch == '\t') {
        int next_tab = (win->cur_c + 8) & ~7; 
        while (win->cur_c < next_tab && win->cur_c < inner_width) {
            if (win->cur_r < WIN_MAX_ROWS && win->cur_c < WIN_MAX_COLS) {
                win->text[win->cur_r][win->cur_c] = ' ';
                win->fg[win->cur_r][win->cur_c] = fg;
                win->bg[win->cur_r][win->cur_c] = bg;
            }
            win->cur_c++;
        }
        if (win->cur_c >= inner_width) {
            win->cur_c = 0;
            win->cur_r++;
            if (win->cur_r >= inner_height) {
                window_scroll_up(win);
                win->cur_r = inner_height - 1;
            }
        }
        return;
    }

    if (ch == '\b' || ch == 127) {
        if (win->cur_c > 0) {
            win->cur_c--;
            if (win->cur_r < WIN_MAX_ROWS && win->cur_c < WIN_MAX_COLS) {
                win->text[win->cur_r][win->cur_c] = ' ';
                win->fg[win->cur_r][win->cur_c] = 7;
                win->bg[win->cur_r][win->cur_c] = 0;
            }
        }
        return;
    }

    if (ch >= 32 && ch <= 126) {
        if (win->cur_r < WIN_MAX_ROWS && win->cur_c < WIN_MAX_COLS) {
            win->text[win->cur_r][win->cur_c] = ch;
            win->fg[win->cur_r][win->cur_c] = fg; 
            win->bg[win->cur_r][win->cur_c] = bg;
        }

        win->cur_c++;
        if (win->cur_c >= inner_width) {
            win->cur_c = 0;
            win->cur_r++;
            if (win->cur_r >= inner_height) {
                window_scroll_up(win);
                win->cur_r = inner_height - 1;
            }
        }
    }
}

// ========================================================
// 4. SCROLL CONTROL FUNCTIONS (பயனர் கட்டுப்பாடு)
// ========================================================
void window_scroll_view_up(FloatingWindow *win) {
    if (win && win->scroll_offset < win->hist_count) {
        win->scroll_offset++; // ஒவ்வொரு வரியாக மேலே செல்லும்
    }
}

void window_scroll_view_down(FloatingWindow *win) {
    if (win && win->scroll_offset > 0) {
        win->scroll_offset--; // கீழே இறங்கும்
    }
}

void window_scroll_view_reset(FloatingWindow *win) {
    if (win) {
        win->scroll_offset = 0;
    }
}

// ========================================================
// 5. CLEANUP (Zero Memory Leak Engine)
// ========================================================
void window_destroy(FloatingWindow *win) {
    if (!win) return;
    
    // Malloc செய்த அனைத்தையும் தடையின்றி க்ளீன் செய்யும் இடம்
    for (int i = 0; i < SCROLLBACK_HISTORY_MAX; i++) {
        free(win->hist_text[i]);
        free(win->hist_fg[i]);
        free(win->hist_bg[i]);
    }
    free(win->hist_text);
    free(win->hist_fg);
    free(win->hist_bg);
    
    free(win);
}
