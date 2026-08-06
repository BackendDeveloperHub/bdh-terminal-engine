// src/ui/panes.c - BDH Pure Linux CLI Multiplexer Windows & Panes (Clean Top Row 0 + Title Badge Kept!)
#include "panes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// நிறங்களுக்கான குறியீடுகள்
#define COLOR_GREEN  2   // ANSI Green (32 / Bright Green)
#define COLOR_RESET  0   // Default Terminal Color

FloatingWindow* window_create(int id, int x, int y, int width, int height, const char* title, int z_index) {
    FloatingWindow *win = (FloatingWindow*)malloc(sizeof(FloatingWindow));
    win->id = id;
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->z_index = z_index;
    
    // z_index == 1 ஆக இருந்தால் மட்டுமே Active
    win->is_active = (z_index == 1) ? 1 : 0;
    
    win->cur_r = 0;
    win->cur_c = 0;
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->title[sizeof(win->title) - 1] = '\0';

    // ஆரம்பத்தில் விண்டோ மெமரியை காலியாக்குதல் (Spaces)
    for (int r = 0; r < WIN_MAX_ROWS; r++) {
        for (int c = 0; c < WIN_MAX_COLS; c++) {
            win->text[r][c] = ' ';
        }
    }
    return win;
}

// 1. விண்டோ மெமரியை (win->text) ஒரு வரி மேலே நகர்த்தும் பங்க்ஷன் (Scroll Up):
void window_scroll_up(FloatingWindow *win) {
    int inner_height = win->height - 2;
    int inner_width  = win->width - 2;

    for (int r = 0; r < inner_height - 1 && r < WIN_MAX_ROWS - 1; r++) {
        for (int c = 0; c < inner_width && c < WIN_MAX_COLS; c++) {
            win->text[r][c] = win->text[r + 1][c];
        }
    }

    int last_r = inner_height - 1;
    if (last_r < WIN_MAX_ROWS) {
        for (int c = 0; c < inner_width && c < WIN_MAX_COLS; c++) {
            win->text[last_r][c] = ' ';
        }
    }
}

// 2. Responsive Border Draw Function (Row 0 Border + Full Width + Title Badge Kept!)
void window_draw(VirtualScreen *scr, FloatingWindow *win) {
    // --- FULL WIDTH & SAFE VERTICAL MARGIN CALCULATION ---
    int start_r = 1;                  // 🔥 Row 0-லேயே தொடங்கும்! (உச்சியில் எந்த காலி வரியும் இருக்காது)
    int end_r   = scr->rows - 11;      // கீழே உள்ள [ BDH Active Sessions Manager ] பாக்ஸுக்கு மேலே சரியாக முடியும்
    int start_c = 0;                  // இடதுபுறம் முழு அகலம் (0 Margin)
    int end_c   = scr->cols - 1;      // வலதுபுறம் முழு அகலம் (0 Margin)

    // பாதுகாப்பு அரண் (Safety Check):
    if (end_r <= start_r + 2) end_r = scr->rows - 2;
    if (end_c <= start_c + 2) end_c = scr->cols - 1;

    // 1. மூலைகள் (Clean Crisp Corners - GREEN)
    screen_put_char_color(scr, start_r, start_c, '+', COLOR_GREEN);
    screen_put_char_color(scr, start_r, end_c,   '+', COLOR_GREEN);
    screen_put_char_color(scr, end_r, start_c,   '+', COLOR_GREEN);
    screen_put_char_color(scr, end_r, end_c,     '+', COLOR_GREEN);

    // 2. மேல் & கீழ் பார்டர் கோடுகள் (Top Bar 'Bold =' & Bottom Bar '-' - GREEN)
    for (int c = start_c + 1; c < end_c; c++) {
        screen_put_char_color(scr, start_r, c, '=', COLOR_GREEN); // மேல் பார்டர் தடிமனாக (=)
        screen_put_char_color(scr, end_r, c, '-', COLOR_GREEN);   // கீழ் பார்டர் நேர்த்தியாக (-)
    }

    // 3. பக்கவாட்டு கோடுகள் (Vertical Side Borders - GREEN)
    for (int r = start_r + 1; r < end_r; r++) {
        screen_put_char_color(scr, r, start_c, '|', COLOR_GREEN);
        screen_put_char_color(scr, r, end_c,   '|', COLOR_GREEN);
    }

    // 4. விண்டோவின் உள் எழுத்துக்கள் (Window Text Content - Default Color)
    int inner_w = (end_c - start_c - 1);
    int inner_h = (end_r - start_r - 1);
    for (int r = 0; r < inner_h && r < WIN_MAX_ROWS; r++) {
        for (int c = 0; c < inner_w && c < WIN_MAX_COLS; c++) {
            screen_put_char(scr, start_r + 1 + r, start_c + 1 + c, win->text[r][c]);
        }
    }

    // 5. 🔥 நீங்கள் கேட்ட [ TAB 1/50 : BASH-1 (ACTIVE) * ] TITLE BADGE மாஸாக மீண்டும் சேர்க்கப்பட்டுள்ளது!
    char badge[128];
    snprintf(badge, sizeof(badge), "  %s  ", win->title); 
    int badge_len = strlen(badge);
    int title_pos = start_c + 3; // இடதுபுறம் 3 ஸ்பேஸ் தள்ளி அழகாகத் தொடங்கும்

    for (int i = 0; i < badge_len && (title_pos + i) < end_c; i++) {
        screen_put_char_color(scr, start_r, title_pos + i, badge[i], COLOR_GREEN);
    }
}

// 3. முழுமையான window_put_char (Scrolling + Backspace + Tab '\t' ஆதரவுடன்)
void window_put_char(VirtualScreen *scr, FloatingWindow *win, char ch) {
    (void)scr; 
    int inner_width = win->width - 2;
    int inner_height = win->height - 2;

    // Carriage Return (\r - ASCII 13)
    if (ch == '\r') {
        win->cur_c = 0;
        return;
    }
    
    // Newline / Line Feed (\n - ASCII 10)
    if (ch == '\n') {
        win->cur_r++;
        if (win->cur_r >= inner_height) {
            window_scroll_up(win);
            win->cur_r = inner_height - 1;
        }
        return;
    }

    // Tab (\t - ASCII 9) ஆதரவு ('ls' கமாண்டுக்கு மிக முக்கியம்!)
    if (ch == '\t') {
        int next_tab = (win->cur_c + 8) & ~7; // 8-Space Tab Stop
        while (win->cur_c < next_tab && win->cur_c < inner_width) {
            if (win->cur_r < WIN_MAX_ROWS && win->cur_c < WIN_MAX_COLS) {
                win->text[win->cur_r][win->cur_c] = ' ';
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

    // Backspace ஆதரவு (\b அல்லது 127)
    if (ch == '\b' || ch == 127) {
        if (win->cur_c > 0) {
            win->cur_c--;
            if (win->cur_r < WIN_MAX_ROWS && win->cur_c < WIN_MAX_COLS) {
                win->text[win->cur_r][win->cur_c] = ' ';
            }
        }
        return;
    }

    // சாதாரண எழுத்துக்கள் (Printable ASCII 32..126)
    if (ch >= 32 && ch <= 126) {
        if (win->cur_r < WIN_MAX_ROWS && win->cur_c < WIN_MAX_COLS) {
            win->text[win->cur_r][win->cur_c] = ch;
        }

        win->cur_c++;
        // Auto-wrap (வரியின் முடிவை அடைந்தால் அடுத்த வரிக்குச் செல்ல)
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
