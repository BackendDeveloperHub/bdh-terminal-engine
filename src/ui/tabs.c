// src/ui/tabs.c - BDH Terminal Engine Tab Bar UI & Overlay Manager Implementation
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

// 4. வழக்கமான Tab Bar-ஐ திரையில் வரைதல்
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

// 5. --- ADDED: Tab Overlay Dashboard Renderer (Git-Status / Manager Style) ---
void render_tab_overlay(VirtualScreen *scr, TabBar *bar) {
    (void)scr;
    if (!bar) return;

    int box_top = 5;
    int box_w = DEFAULT_COLS - 1;

    // 1. Active Tab லேபிளை சிவப்பு நிறத்தில் காட்டுதல் (\033[31m)
    printf("\033[%d;2H\033[31m[ BDH Active Sessions Overview ]\033[0m", box_top - 1);

    // 2. அனைத்து டேப்களின் பட்டியலை மல்டி-லைனில் (Wrapped) பிரிண்ட் செய்தல்
    int row = box_top + 2;
    int col = 2;
    
    for (int i = 0; i < bar->count; i++) {
        char buf[128];
        if (bar->tabs[i].is_active) {
            // Active tab-ஐ Bold Green-ல் Highlight செய்ய: \033[1;32m
            snprintf(buf, sizeof(buf), "\033[1;32m[%d- %s (ACTIVE)]\033[0m", i + 1, bar->tabs[i].title);
        } else {
            snprintf(buf, sizeof(buf), "%d- %s", i + 1, bar->tabs[i].title);
        }
        
        // டெர்மினல் கரஸ்பாண்டிங் பொசிஷனில் பிரிண்ட் செய்தல்
        printf("\033[%d;%dH%s", row, col, buf);
        
        col += strlen(bar->tabs[i].title) + 10;
        if (col > box_w - 20) { 
            col = 2; 
            row++; 
        }
    }
    fflush(stdout);
}

// 6. மெமரியை அழித்தல்
void tabs_destroy(TabBar *bar) {
    if (bar) {
        free(bar);
    }
}
