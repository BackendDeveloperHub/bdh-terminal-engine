// src/engine/cursor.h
#ifndef BDH_CURSOR_H
#define BDH_CURSOR_H

#include "ui/panes.h"

// கர்சர் கட்டுப்பாட்டு பங்க்ஷன்கள் (Hardware Cursor Control)
void cursor_hide(void);
void cursor_show(void);
void cursor_move(int row, int col);

// Active விண்டோவின் உள்-கர்சர் (cur_r, cur_c) இருக்கும் இடத்திற்கு மானிட்டர் கர்சரை நகர்த்த
void cursor_sync_to_window(FloatingWindow *win);

#endif // BDH_CURSOR_H
