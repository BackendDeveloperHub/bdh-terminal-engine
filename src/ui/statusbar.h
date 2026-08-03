// src/ui/statusbar.h - BDH Terminal Engine Status Bar UI
#ifndef BDH_STATUSBAR_H
#define BDH_STATUSBAR_H

#include "engine/screen.h"

#define STATUS_LEN 128

typedef struct {
    char mode[32];        // எ.கா: "NORMAL", "PREFIX", "SCANNER"
    char left_text[STATUS_LEN];
    char right_text[STATUS_LEN];
} StatusBar;

// Status Bar பங்க்ஷன்கள்
StatusBar* statusbar_create(void);
void statusbar_set_mode(StatusBar *bar, const char *mode_str);
void statusbar_set_text(StatusBar *bar, const char *left, const char *right);
void statusbar_draw(VirtualScreen *scr, StatusBar *bar, int row);
void statusbar_destroy(StatusBar *bar);

#endif // BDH_STATUSBAR_H
