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

// 4. வழக்கமான Tab Bar-ஐ திரையில் வரைதல் (Top Header)
void tabs_draw(VirtualScreen *scr, TabBar *bar, int row) {
    (void)scr;
    if (!bar) return;
    
    // Top Bar-ல் டேப்களின் பெயர்களை வரிசையாகக் காட்டுதல்
    printf("\033[%d;1H\033[1;32m", row + 1); // Green Header Style
    for (int i = 0; i < bar->count; i++) {
        if (bar->tabs[i].is_active) {
            printf(" [%d: %s*] ", i + 1, bar->tabs[i].title);
        } else {
            printf("  %d: %s  ", i + 1, bar->tabs[i].title);
        }
    }
    printf("\033[0m"); // Reset color
    fflush(stdout);
}

// 5. --- UPDATED: Footer Active Sessions Manager Box Renderer ---
void render_tab_overlay(VirtualScreen *scr, TabBar *bar) {
    if (!bar || !scr) return;

    int box_top = scr->rows - 6; // திரையின் அடிபாகத்தில் பாக்ஸ் வர
    int box_left = 2;
    int box_w = scr->cols - 4;

    if (box_top < 10) box_top = 10; // பாதுகாப்பு அரண்

    // 1. பாக்ஸின் தலைப்பு பார்டர் (Top Border + Title)
    printf("\033[%d;%dH\033[1;32m+--- [ BDH Active Sessions Manager ] ---------------------------------+\033[0m", box_top, box_left);

    // 2. டேப்களின் நிலையை வரிசையாக (Wrapped List) காட்டுதல்
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
        
        col += strlen(bar->tabs[i].title) + 12;
        if (col > box_w - 20) { 
            col = box_left + 2; 
            row++; 
        }
    }
    
    // 3. பாக்ஸின் கீழ் பார்டர் (Bottom Border)
    printf("\033[%d;%dH\033[1;32m+--------------------------------------------------------------------+\033[0m", box_top + 4, box_left);
    fflush(stdout);
}

// 6. மெமரியை அழித்தல்
void tabs_destroy(TabBar *bar) {
    if (bar) {
        free(bar);
    }
}
