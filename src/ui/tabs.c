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
void tabs_draw(VirtualScreen *scr, TabBar *bar, int row) {
    (void)scr;
    (void)bar;
    (void)row;
    // Top Tab Bar display disabled as requested.
}

// --- 🔥 HELPER FUNCTIONS: VirtualScreen மெமரியில் நேரடியாக எழுத ---
static void draw_str(VirtualScreen *scr, int r, int c, const char *str, int color) {
    if (r < 0 || r >= scr->rows || !scr->grid || !scr->grid[r]) return;
    for (int i = 0; str[i] != '\0' && (c + i) < scr->cols; i++) {
        scr->grid[r][c + i].ch = str[i];
        scr->grid[r][c + i].fg_color = color;
    }
}

static void fill_row(VirtualScreen *scr, int r, int c_start, int c_end, char ch, int color) {
    if (r < 0 || r >= scr->rows || !scr->grid || !scr->grid[r]) return;
    for (int c = c_start; c <= c_end && c < scr->cols; c++) {
        scr->grid[r][c].ch = ch;
        scr->grid[r][c].fg_color = color;
    }
}
// -------------------------------------------------------------------

// 5. --- UPDATED: Full-Screen Footer Active Sessions Manager Box (-12 to -1 Complete Cover) ---
void render_tab_overlay(VirtualScreen *scr, TabBar *bar) {
    if (!bar || !scr) return;

    // ANSI 1-based Row-ஐ 0-based Array Index ஆக மாற்றுகிறோம்
    int box_top_idx = scr->rows - 13; 
    int box_bottom_idx = scr->rows - 2; 

    if (box_top_idx < 0) box_top_idx = 0;

    // --- 1. Dynamic Full-Width Top Border (+=== [ BDH Active Sessions Manager ] ===+) ---
    char title_buf[128];
    snprintf(title_buf, sizeof(title_buf), "+== [ BDH Active Sessions Manager ] ");
    
    // color = 2 (பச்சை நிறம்)
    draw_str(scr, box_top_idx, 0, title_buf, 2);
    int used_len = strlen(title_buf);
    
    fill_row(scr, box_top_idx, used_len, scr->cols - 2, '=', 2);
    draw_str(scr, box_top_idx, scr->cols - 1, "+", 2);

    // --- 2. 🔥 GHOST TEXT FIX: பாக்ஸின் உள்ளே இருக்கும் எல்லா வரிகளையும் சுத்தமாக அழித்தல் ---
    for (int r = box_top_idx + 1; r < box_bottom_idx; r++) { 
        draw_str(scr, r, 0, "|", 2);                      // Left Border
        fill_row(scr, r, 1, scr->cols - 2, ' ', 0);       // Clear inside
        draw_str(scr, r, scr->cols - 1, "|", 2);          // Right Border
    }

    // --- 3. டேப்களின் நிலையை வரிசையாக (Full-Screen Wrapped List - 18 Max safe) காட்டுதல் ---
    int r_idx = box_top_idx + 1;
    int c_idx = 3; // இடது பார்டரிலிருந்து 2 ஸ்பேஸ் தள்ளி அழகாகத் தொடங்கும்
    
    for (int i = 0; i < bar->count; i++) {
        char buf[128];
        int color = 0; // Default Terminal Color
        
        if (bar->tabs[i].is_active) {
            snprintf(buf, sizeof(buf), "[%d: %s (RUNNING)]", i + 1, bar->tabs[i].title);
            color = 2; // Active tab-ஐ Green-ல் ஒளிரச் செய்தல்
        } else {
            snprintf(buf, sizeof(buf), "%d: %s", i + 1, bar->tabs[i].title);
        }
        
        draw_str(scr, r_idx, c_idx, buf, color);
        
        c_idx += strlen(bar->tabs[i].title) + 16;
        
        // வலதுபுற எல்லை வந்ததும் அடுத்த வரிக்குச் செல்ல
        if (c_idx > scr->cols - 20 && r_idx < box_bottom_idx - 1) { 
            c_idx = 3; 
            r_idx++; 
        }
    }
    
    // --- 4. 🔥 Dynamic Full-Width Bottom Border (+---+ across entire screen at FOOTER -1) ---
    draw_str(scr, box_bottom_idx, 0, "+", 2);
    fill_row(scr, box_bottom_idx, 1, scr->cols - 2, '-', 2);
    draw_str(scr, box_bottom_idx, scr->cols - 1, "+", 2);
}

// 6. மெமரியை அழித்தல்
void tabs_destroy(TabBar *bar) {
    if (bar) {
        free(bar);
    }
}
