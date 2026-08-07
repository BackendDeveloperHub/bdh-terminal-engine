// src/engine/cursor.c - BDH Pure Linux CLI Multiplexer Cursor Control (Blinking Block Enabled)
#include "cursor.h"
#include <stdio.h>

// 1. திரையை வரையும்போது கர்சர் துள்ளுவதைத் தடுக்க (Hide cursor)
void cursor_hide(void) {
    printf("\033[?25l");
    fflush(stdout);
}

// 2. 🔥 மாஸ் மாற்றம்: கர்சரை மீண்டும் காட்டுவதோடு, அதை 'BLINKING BLOCK (█)' ஆக மாற்றுதல்!
void cursor_show(void) {
    // \033[?25h = Show Cursor
    // \033[1 q  = VT520 Blinking Block Style (பளிச் பளிச்-என மின்னும் கட்டம்)
    printf("\033[?25h\033[1 q");
    fflush(stdout);
}

// 3. குறிப்பிட்ட (row, col) இடத்திற்கு கர்சரை நகர்த்த (1-based terminal grid)
void cursor_move(int row, int col) {
    // ANSI escape code: \033[<row>;<col>H
    printf("\033[%d;%dH", row, col);
    fflush(stdout);
}

// 4. Active விண்டோவின் உள்ளே சரியாக கர்சரை கொண்டு வந்து நிறுத்தி மிளிரச் செய்தல்
void cursor_sync_to_window(FloatingWindow *win) {
    if (!win || !win->is_active) return;

    // win->x = விண்டோ தொடங்கும் Row, +1 = மேல் பார்டர் கோடு (+---+), + win->cur_r = உள்ளே இருக்கும் வரிசை
    int target_row = win->x + 1 + win->cur_r;
    
    // win->y = விண்டோ தொடங்கும் Col, +1 = இடது பார்டர் கோடு (|), + win->cur_c = உள்ளே இருக்கும் காலம்
    int target_col = win->y + 1 + win->cur_c;

    // ANSI டெர்மினல் 1-Based Index என்பதால் இரண்டுக்கும் +1 கூட்டுகிறோம்
    cursor_move(target_row + 1, target_col + 1);
    
    // 🔥 கர்சரை அந்த இடத்தில் நிறுத்தி பளிச்சிடச் செய்கிறோம்:
    cursor_show();
}
