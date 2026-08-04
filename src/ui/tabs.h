// src/ui/tabs.h - BDH Terminal Engine Tab Bar UI & Overlay Manager
#ifndef BDH_TABS_H
#define BDH_TABS_H

#include "engine/screen.h"

#define MAX_TABS 20      // 6-லிருந்து 20 அல்லது 50 டேப்களுக்கு உயர்த்திக்கொள்ளலாம்
#define TAB_TITLE_LEN 32

typedef struct {
    int id;
    char title[TAB_TITLE_LEN];
    int is_active;
    int is_alive;
} TabItem;

typedef struct {
    TabItem tabs[MAX_TABS];
    int count;
    int active_idx;
} TabBar;

// 1. Existing Tab Bar Functions
TabBar* tabs_create(void);
int tabs_add(TabBar *bar, const char *title);
void tabs_set_active(TabBar *bar, int index);
void tabs_draw(VirtualScreen *scr, TabBar *bar, int row);
void tabs_destroy(TabBar *bar);

// 2. --- ADDED: Tab Overlay Dashboard Function (git status style overview) ---
void render_tab_overlay(VirtualScreen *scr, TabBar *bar);

#endif // BDH_TABS_H
