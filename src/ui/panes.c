// src/ui/panes.c - BDH Pure Linux CLI Multiplexer Windows & Panes (256-Color & Custom Color Support)
#include "panes.h"
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

    // ஆரம்பத்தில் விண்டோ மெமரியை காலியாக்குதல் (Spaces & Default White Color)
    for (int r = 0; r < WIN_MAX_ROWS; r++) {
        for (int c = 0; c < WIN_MAX_COLS; c++) {
            win->text[r][c] = ' ';
            win->fg[r][c] = 7; // Default White
            win->bg[r][c] = 0; // Default Black
        }
    }
    return win;
}

// 1. விண்டோ மெமரியை ஸ்க்ரோல் செய்யும் போது நிறங்களையும் சேர்த்து நகர்த்துவது:
void window_scroll_up(FloatingWindow *win) {
    int inner_height = win->height - 2;
    int inner_width  = win->width - 2;

    for (int r = 0; r < inner_height - 1 && r < WIN_MAX_ROWS - 1; r++) {
        for (int c = 0; c < inner_width && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = win->text[r + 1][c];
            win->fg[r][c]   = win->fg[r + 1][c];
            win->bg[r][c]   = win->bg[r + 1][c];
        }
    }

    int last_r = inner_height - 1;
    if (last_r < WIN_MAX_ROWS) {
        for (int c = 0; c < inner_width && c < WIN_MAX_COLS; c++) {
            win->text[last_r][c] = ' ';
            win->fg[last_r][c]   = 7;
            win->bg[last_r][c]   = 0;
        }
    }
}

// 2. Responsive Border & Color Drawing Function
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

    // 🔥 4. விண்டோவின் உள் எழுத்துக்கள் மற்றும் அவற்றின் உண்மையான நிறங்களை (Foreground Colors) பிரிண்ட் செய்வது!
    int inner_w = (end_c - start_c - 1);
    int inner_h = (end_r - start_r - 1);
    for (int r = 0; r < inner_h && r < WIN_MAX_ROWS; r++) {
        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            char ch = win->text[r][c];
            int fg = win->fg[r][c];
            screen_put_char_color(scr, start_r + 1 + r, start_c + 1 + c, ch, fg);
        }
    }

    // 5. Title Badge
    char badge[128];
    snprintf(badge, sizeof(badge), "  %s  ", win->title); 
    int badge_len = strlen(badge);
    int title_pos = start_c + 3; 

    for (int i = 0; i < badge_len && (title_pos + i) < end_c; i++) {
        screen_put_char_color(scr, start_r, title_pos + i, badge[i], COLOR_GREEN);
    }
}

// 3. 🔥 நிறங்களோடு எழுத்துக்களை வாங்கும் மாஸ்டர் பங்க்ஷன் (Parser-லிருந்து கலர் வரும்)
void window_put_char(VirtualScreen *scr, FloatingWindow *win, char ch, int fg, int bg) {
    (void)scr; 
    int inner_width = win->width - 2;
    int inner_height = win->height - 2;

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
            win->fg[win->cur_r][win->cur_c] = fg; // 🔥 உண்மையான கலர் மெமரியில் பதிவாகிறது!
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

void window_destroy(FloatingWindow *win) {
    free(win);
}
