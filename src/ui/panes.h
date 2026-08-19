// src/ui/panes.h - BDH Pure Linux CLI Multiplexer Panes Header (Scrollback Edition)
#ifndef BDH_PANES_H
#define BDH_PANES_H

#include "engine/screen.h"

// தற்போதைய ஸ்க்ரீன் அளவுகள்
#define WIN_MAX_ROWS 150  
#define WIN_MAX_COLS 300  

// 🔥 SCROLLBACK LIMIT: ஒரு விண்டோவிற்கு 1000 வரிகள் வரை பழைய அவுட்புட் சேமிக்கப்படும்!
#define SCROLLBACK_HISTORY_MAX 1000 

typedef struct {
    int id;
    int x, y;
    int width, height;
    int z_index;
    int is_active;
    char title[64];
    
    // தற்போது ஸ்க்ரீனில் தெரியும் வரிகள்
    char text[WIN_MAX_ROWS][WIN_MAX_COLS];
    unsigned char fg[WIN_MAX_ROWS][WIN_MAX_COLS]; 
    unsigned char bg[WIN_MAX_ROWS][WIN_MAX_COLS]; 
    
    // ========================================================
    // 🔥 SCROLLBACK HISTORY BUFFER (The Architect Update)
    // ========================================================
    char **hist_text;           // பழைய வரிகளின் எழுத்துக்கள்
    unsigned char **hist_fg;    // பழைய வரிகளின் Foreground நிறம்
    unsigned char **hist_bg;    // பழைய வரிகளின் Background நிறம்
    int hist_head;              // கடைசியாக எந்த வரியில் டேட்டா சேமிக்கப்பட்டது
    int hist_count;             // மொத்தம் எத்தனை வரிகள் சேமிக்கப்பட்டுள்ளன
    int scroll_offset;          // பயனர் எவ்வளவு மேலே ஸ்க்ரோல் செய்துள்ளார் (0 = Normal View)
    // ========================================================

    int cur_r, cur_c;
} FloatingWindow;

FloatingWindow* window_create(int id, int x, int y, int width, int height, const char* title, int z_index);
void window_draw(VirtualScreen *scr, FloatingWindow *win);
void window_put_char(VirtualScreen *scr, FloatingWindow *win, char ch, int fg, int bg);
void window_scroll_up(FloatingWindow *win);
void window_destroy(FloatingWindow *win);

// 🔥 புதிய Scrollback கண்ட்ரோல் ஃபங்ஷன்கள்
void window_scroll_view_up(FloatingWindow *win);
void window_scroll_view_down(FloatingWindow *win);
void window_scroll_view_reset(FloatingWindow *win);

#endif // BDH_PANES_H
