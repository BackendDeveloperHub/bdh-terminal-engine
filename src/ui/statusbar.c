// src/ui/statusbar.c - BDH Terminal Engine Status Bar UI Implementation
#include "ui/statusbar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// புதிய StatusBar மெமரியை உருவாக்குதல் (Default Values)
StatusBar* statusbar_create(void) {
    StatusBar *bar = (StatusBar*)malloc(sizeof(StatusBar));
    if (!bar) return NULL;

    strncpy(bar->mode, "NORMAL", sizeof(bar->mode) - 1);
    bar->mode[sizeof(bar->mode) - 1] = '\0';

    strncpy(bar->left_text, "[ BDH Linux Multiplexer ]", STATUS_LEN - 1);
    bar->left_text[STATUS_LEN - 1] = '\0';

    strncpy(bar->right_text, "Ctrl+B: Browser | Ctrl+K: Scan | exit: Quit", STATUS_LEN - 1);
    bar->right_text[STATUS_LEN - 1] = '\0';

    return bar;
}

// என்ஜினின் Mode-ஐ மாற்றுவது (எ.கா: NORMAL -> SCANNER)
void statusbar_set_mode(StatusBar *bar, const char *mode_str) {
    if (!bar || !mode_str) return;
    strncpy(bar->mode, mode_str, sizeof(bar->mode) - 1);
    bar->mode[sizeof(bar->mode) - 1] = '\0';
}

// இடது மற்றும் வலதுபுற டெக்ஸ்டை மாற்றுவது
void statusbar_set_text(StatusBar *bar, const char *left, const char *right) {
    if (!bar) return;
    if (left) {
        strncpy(bar->left_text, left, STATUS_LEN - 1);
        bar->left_text[STATUS_LEN - 1] = '\0';
    }
    if (right) {
        strncpy(bar->right_text, right, STATUS_LEN - 1);
        bar->right_text[STATUS_LEN - 1] = '\0';
    }
}

// VirtualScreen-ன் கடைசி வரியில் (row = rows-1) Status Bar-ஐ வரைவது
void statusbar_draw(VirtualScreen *scr, StatusBar *bar, int row) {
    if (!scr || !bar || row < 0 || row >= scr->rows) return;

    char full_line[512];
    char left_part[256];
    char right_part[256];

    // இடதுபுற பகுதி: [ MODE ] Left Text
    snprintf(left_part, sizeof(left_part), "[ %s ] %s", bar->mode, bar->left_text);

    // வலதுபுற பகுதி: Right Text (Shortcuts guide)
    snprintf(right_part, sizeof(right_part), " %s ", bar->right_text);

    int left_len = (int)strlen(left_part);
    int right_len = (int)strlen(right_part);
    int cols = scr->cols;

    // இடது மற்றும் வலதுபுறத்திற்கு நடுவில் ஸ்பேஸ் விட்டு முழு வரியையும் நிரப்புதல்:
    int spaces = cols - (left_len + right_len);
    if (spaces < 0) spaces = 0;

    int offset = 0;
    offset += snprintf(full_line + offset, sizeof(full_line) - offset, "%s", left_part);
    
    for (int i = 0; i < spaces && offset < (int)sizeof(full_line) - 1; i++) {
        full_line[offset++] = ' ';
    }
    full_line[offset] = '\0';

    if (offset < (int)sizeof(full_line) - 1) {
        snprintf(full_line + offset, sizeof(full_line) - offset, "%s", right_part);
    }

    // VirtualScreen-ன் குறிப்பிட்ட வரியில் எழுதுகிறோம்:
    int total_len = (int)strlen(full_line);
    for (int c = 0; c < cols && c < total_len; c++) {
        // குறிப்பு: VirtualScreen-ன் cell structure-க்கு ஏற்ப scr->cells[row][c].ch = full_line[c]; என்று மாற்றலாம்
        screen_put_char(scr, row, c, full_line[c]);
    }
}

// மெமரியை முழுமையாக அழித்தல்
void statusbar_destroy(StatusBar *bar) {
    if (bar) {
        free(bar);
    }
}
