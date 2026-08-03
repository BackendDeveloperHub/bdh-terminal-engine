// src/ui/tabs.h - BDH Terminal Engine Tab Bar UI
#ifndef BDH_TABS_H
#define BDH_TABS_H

#include "engine/screen.h"

#define MAX_TABS 6
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

// Tab Bar பங்க்ஷன்கள்
TabBar* tabs_create(void);
int tabs_add(TabBar *bar, const char *title);
void tabs_set_active(TabBar *bar, int index);
void tabs_draw(VirtualScreen *scr, TabBar *bar, int row);
void tabs_destroy(TabBar *bar);

#endif // BDH_TABS_H
