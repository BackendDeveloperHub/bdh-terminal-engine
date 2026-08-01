// src/engine/cursor.c
#include "cursor.h"
#include <stdio.h>

// திரையை வரையும்போது கர்சர் துள்ளுவதைத் தடுக்க (Hide cursor)
void cursor_hide(void) {
    printf("\033[?25l");
    fflush(stdout);
}

// கர்சரை மீண்டும் காட்ட (Show cursor)
void cursor_show(void) {
    printf("\033[?25h");
    fflush(stdout);
}

// குறிப்பிட்ட (row, col) இடத்திற்கு கர்சரை நகர்த்த (1-based terminal grid)
void cursor_move(int row, int col) {
    // ANSI escape code: \033[<row>;<col>H
    printf("\033[%d;%dH", row, col);
    fflush(stdout);
}

// Active விண்டோவின் உள்ளே சரியாக கர்சரை கொண்டு வந்து நிறுத்த
void cursor_sync_to_window(FloatingWindow *win) {
    if (!win || !win->is_active) return;

    // win->x = விண்டோ தொடங்கும் Row, +1 = மேல் பார்டர் கோடு (+---+), + win->cur_r = உள்ளே இருக்கும் வரிசை
    int target_row = win->x + 1 + win->cur_r;
    
    // win->y = விண்டோ தொடங்கும் Col, +1 = இடது பார்டர் கோடு (|), + win->cur_c = உள்ளே இருக்கும் காலம்
    int target_col = win->y + 1 + win->cur_c;

    // ANSI டெர்மினல் 1-Based Index என்பதால் இரண்டுக்கும் +1 கூட்டுகிறோம்
    cursor_move(target_row + 1, target_col + 1);
    cursor_show();
}
