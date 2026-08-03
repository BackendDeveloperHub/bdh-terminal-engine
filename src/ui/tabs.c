// src/ui/tabs.c - BDH Terminal Engine Tab Bar UI Implementation
#include "ui/tabs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// புதிய TabBar மெமரியை உருவாக்குதல்
TabBar* tabs_create(void) {
    TabBar *bar = (TabBar*)malloc(sizeof(TabBar));
    if (!bar) return NULL;

    bar->count = 0;
    bar->active_idx = 0;

    for (int i = 0; i < MAX_TABS; i++) {
        bar->tabs[i].id = i;
        bar->tabs[i].is_active = 0;
        bar->tabs[i].is_alive = 0;
        memset(bar->tabs[i].title, 0, TAB_TITLE_LEN);
    }

    return bar;
}

// புதிய Tab-ஐ சேர்ப்பது
int tabs_add(TabBar *bar, const char *title) {
    if (!bar || bar->count >= MAX_TABS) return -1;

    int idx = bar->count;
    bar->tabs[idx].id = idx;
    bar->tabs[idx].is_alive = 1;
    bar->tabs[idx].is_active = (idx == 0) ? 1 : 0; // முதல் Tab எப்போதும் Active

    if (title && strlen(title) > 0) {
        strncpy(bar->tabs[idx].title, title, TAB_TITLE_LEN - 1);
        bar->tabs[idx].title[TAB_TITLE_LEN - 1] = '\0';
    } else {
        snprintf(bar->tabs[idx].title, TAB_TITLE_LEN, "bash-%d", idx + 1);
    }

    bar->count++;
    return idx;
}

// குறிப்பிட்ட Tab-ஐ Active ஆக மாற்றுவது
void tabs_set_active(TabBar *bar, int index) {
    if (!bar || index < 0 || index >= bar->count) return;

    for (int i = 0; i < bar->count; i++) {
        bar->tabs[i].is_active = (i == index) ? 1 : 0;
    }
    bar->active_idx = index;
}

// VirtualScreen-ல் Tab Bar-ஐ வரைவது (tmux-style horizontal bar)
void tabs_draw(VirtualScreen *scr, TabBar *bar, int row) {
    if (!scr || !bar || row < 0 || row >= scr->rows) return;

    char bar_buffer[512] = "";
    int offset = 0;

    // லோகோ / பிராண்ட் பெயர்:
    offset += snprintf(bar_buffer + offset, sizeof(bar_buffer) - offset, "[ BDH Linux ] ");

    // ஒவ்வொரு Tab-ன் டைட்டிலையும் அழகாக இணைப்பது:
    for (int i = 0; i < bar->count; i++) {
        if (!bar->tabs[i].is_alive) continue;

        if (bar->tabs[i].is_active) {
            // Active Tab-க்கு ஒரு ஸ்டார் (*) மற்றும் ஸ்பெஷல் பிராக்கெட்டுகள்:
            offset += snprintf(bar_buffer + offset, sizeof(bar_buffer) - offset,
                               "*[ %d:%s* ]  ", i + 1, bar->tabs[i].title);
        } else {
            // Normal Background Tab:
            offset += snprintf(bar_buffer + offset, sizeof(bar_buffer) - offset,
                               "[ %d:%s ]  ", i + 1, bar->tabs[i].title);
        }
    }

    // இந்த முழு Tab Bar வரியையும் VirtualScreen-ன் குறிப்பிட்ட row-ல் எழுதுகிறோம்:
    int len = strlen(bar_buffer);
    for (int c = 0; c < scr->cols && c < len; c++) {
        // குறிப்பு: VirtualScreen-ன் cell structure-க்கு ஏற்ப scr->cells[row][c].ch = bar_buffer[c]; என்று மாற்றி அமைக்கலாம்
        screen_put_char(scr, row, c, bar_buffer[c]);
    }
}

// மெமரியை முழுமையாக அழித்தல்
void tabs_destroy(TabBar *bar) {
    if (bar) {
        free(bar);
    }
}
