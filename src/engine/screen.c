// src/engine/screen.c - BDH Virtual Screen Buffer Management (Delta Rendering Ready)
#include "screen.h"
#include <stdio.h>
#include <stdlib.h>

VirtualScreen* screen_create(int rows, int cols) {
    VirtualScreen *screen = (VirtualScreen*)malloc(sizeof(VirtualScreen));
    screen->rows = rows;
    screen->cols = cols;

    // 2D Array-க்காக Memory Allocation (Front & Back Buffers)
    screen->grid = (ScreenCell**)malloc(rows * sizeof(ScreenCell*));
    screen->old_grid = (ScreenCell**)malloc(rows * sizeof(ScreenCell*));

    for (int r = 0; r < rows; r++) {
        screen->grid[r] = (ScreenCell*)malloc(cols * sizeof(ScreenCell));
        screen->old_grid[r] = (ScreenCell*)malloc(cols * sizeof(ScreenCell));
        
        // 🔥 old_grid-ஐ வெற்று (null) டேட்டாவால் நிரப்புதல் 
        // (முதல் முறை எஞ்சின் லான்ச் ஆகும்போது மொத்தமாக வரைய இது உதவும்)
        for (int c = 0; c < cols; c++) {
            screen->old_grid[r][c].ch = '\0';
            screen->old_grid[r][c].fg_color = 0;
            screen->old_grid[r][c].bg_color = 0;
        }
    }

    screen_clear(screen);
    return screen;
}

void screen_clear(VirtualScreen *screen) {
    for (int r = 0; r < screen->rows; r++) {
        for (int c = 0; c < screen->cols; c++) {
            screen->grid[r][c].ch = ' ';     // வெற்று Space
            screen->grid[r][c].fg_color = 7; // Default White
            screen->grid[r][c].bg_color = 0; // Default Black
        }
    }
}

// 🔥 புதிய ஃபங்ஷன்: மொத்த ஸ்கிரீனையும் கட்டாயமாக ரெஃப்ரெஷ் செய்ய (Force Redraw)
void screen_force_redraw(VirtualScreen *screen) {
    for (int r = 0; r < screen->rows; r++) {
        for (int c = 0; c < screen->cols; c++) {
            // old_grid-ஐ அழித்துவிட்டால், அடுத்த ரெண்டரில் மொத்த ஸ்கிரீனும் புதுசு என நினைத்து வரையப்படும்
            screen->old_grid[r][c].ch = '\0'; 
        }
    }
}

void screen_put_char(VirtualScreen *screen, int row, int col, char ch) {
    if (row >= 0 && row < screen->rows && col >= 0 && col < screen->cols) {
        screen->grid[row][col].ch = ch;
        screen->grid[row][col].fg_color = 7; // Default White
    }
}

// செல் (Cell) மெமரியில் எழுத்து மற்றும் நிறத்தை (Foreground Color) ஒன்றாகப் பதிவு செய்யும் பங்க்ஷன்
void screen_put_char_color(VirtualScreen *screen, int row, int col, char ch, int fg_color) {
    if (row >= 0 && row < screen->rows && col >= 0 && col < screen->cols) {
        screen->grid[row][col].ch = ch;
        screen->grid[row][col].fg_color = fg_color; 
    }
}

void screen_destroy(VirtualScreen *screen) {
    for (int r = 0; r < screen->rows; r++) {
        free(screen->grid[r]);
        free(screen->old_grid[r]); // 🔥 old_grid-ன் மெமரியையும் கட்டாயம் அழிக்க வேண்டும்
    }
    free(screen->grid);
    free(screen->old_grid);
    free(screen);
}
