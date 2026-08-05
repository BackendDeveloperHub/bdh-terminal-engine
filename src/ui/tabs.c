// src/ui/tabs.c - BDH Terminal Engine Tab Bar UI & Footer Session Manager Implementation
#include "tabs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 1. TabBar-ஐ உருவாக்கும் பங்க்ஷன்
TabBar* tabs_create(void) {
    TabBar *bar = (TabBar*)malloc(sizeof(TabBar));
    if (!bar) return NULL;
    
    bar->count = 0;
    bar->active_idx = 0;
    for (int i = 0; i < MAX_TABS; i++) {
        bar->tabs[i].id = i;
        bar->tabs[i].title[0] = '\0';
        bar->tabs[i].is_active = 0;
        bar->tabs[i].is_alive = 0;
    }
    return bar;
}

// 2. புதிய டேப்பைச் சேர்த்தல்
int tabs_add(TabBar *bar, const char *title) {
    if (!bar || bar->count >= MAX_TABS) return -1;
    
    int idx = bar->count;
    bar->tabs[idx].id = idx;
    strncpy(bar->tabs[idx].title, title, TAB_TITLE_LEN - 1);
    bar->tabs[idx].title[TAB_TITLE_LEN - 1] = '\0';
    bar->tabs[idx].is_alive = 1;
    
    if (idx == 0) {
        bar->tabs[idx].is_active = 1;
        bar->active_idx = 0;
    } else {
        bar->tabs[idx].is_active = 0;
    }
    
    bar->count++;
    return idx;
}

// 3. Active டேப்பை மாற்றுதல்
void tabs_set_active(TabBar *bar, int index) {
    if (!bar || index < 0 || index >= bar->count) return;
    
    for (int i = 0; i < bar->count; i++) {
        bar->tabs[i].is_active = (i == index) ? 1 : 0;
    }
    bar->active_idx = index;
}

// 4. 🔥 TOP TAB BAR REMOVED COMPLETE FIX (No-Op Function)
// இந்த பங்க்ஷன் காலி செய்யப்பட்டுள்ளது; எனவே உச்சியில் "1: BASH-1 2: BASH-2" இனி வரவே வராது!
void tabs_draw(VirtualScreen *scr, TabBar *bar, int row) {
    (void)scr;
    (void)bar;
    (void)row;
    // Top Tab Bar display disabled as requested.
}

// 5. --- UPDATED: Full-Screen Footer Active Sessions Manager Box ---
void render_tab_overlay(VirtualScreen *scr, TabBar *bar) {
    if (!bar || !scr) return;

    int box_top = scr->rows - 5; // திரையின் அடிபாகத்தில் பாக்ஸ் வர
    int box_left = 1;            // 🔥 Column 1-ல் இருந்து முழு அகலத்திற்கும் தொடங்கும் (0 Margin)
    int box_w = scr->cols;       // 🔥 திரையின் முழு அகலம் (Full Screen Cover)

    if (box_top < 10) box_top = 10; // பாதுகாப்பு அரண்
    if (box_w < 40) box_w = 40;

    // --- 1. Dynamic Full-Width Top Border (+=== [ BDH Active Sessions Manager ] ===+) ---
    const char *title = "[ BDH Active Sessions Manager ]";
    int title_len = strlen(title);
    
    printf("\033[%d;%dH\033[1;32m+== %s ", box_top, box_left, title);
    int used_len = 4 + title_len + 1;
    for (int c = used_len; c < box_w - 1; c++) {
        putchar('=');
    }
    printf("+\033[0m");

    // --- 2. டேப்களின் நிலையை வரிசையாக (Full-Screen Wrapped List) காட்டுதல் ---
    int row = box_top + 1;
    int col = box_left + 2;
    
    for (int i = 0; i < bar->count; i++) {
        char buf[128];
        if (bar->tabs[i].is_active) {
            // Active tab-ஐ Green-ல் ஒளிரச் செய்தல்
            snprintf(buf, sizeof(buf), "\033[1;32m[%d: %s (RUNNING)]\033[0m", i + 1, bar->tabs[i].title);
        } else {
            snprintf(buf, sizeof(buf), "%d: %s", i + 1, bar->tabs[i].title);
        }
        
        printf("\033[%d;%dH%s", row, col, buf);
        
        col += strlen(bar->tabs[i].title) + 14;
        // வலதுபுற எல்லை வந்ததும் அடுத்த வரிக்குச் செல்ல (Responsive Wrap)
        if (col > box_w - 18 && row < box_top + 3) { 
            col = box_left + 2; 
            row++; 
        }
    }
    
    // --- 3. Dynamic Full-Width Bottom Border (+---+ across entire screen) ---
    printf("\033[%d;%dH\033[1;32m+", box_top + 4, box_left);
    for (int c = 1; c < box_w - 1; c++) {
        putchar('-');
    }
    printf("+\033[0m");
    fflush(stdout);
}

// 6. மெமரியை அழித்தல்
void tabs_destroy(TabBar *bar) {
    if (bar) {
        free(bar);
    }
}
