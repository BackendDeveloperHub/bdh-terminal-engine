// src/engine/screen.c - BDH Virtual Screen Buffer Management
#include "screen.h"
#include <stdio.h>
#include <stdlib.h>

VirtualScreen* screen_create(int rows, int cols) {
    VirtualScreen *screen = (VirtualScreen*)malloc(sizeof(VirtualScreen));
    screen->rows = rows;
    screen->cols = cols;

    // 2D Array-க்காக Memory Allocation
    screen->grid = (ScreenCell**)malloc(rows * sizeof(ScreenCell*));
    for (int r = 0; r < rows; r++) {
        screen->grid[r] = (ScreenCell*)malloc(cols * sizeof(ScreenCell));
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

void screen_put_char(VirtualScreen *screen, int row, int col, char ch) {
    if (row >= 0 && row < screen->rows && col >= 0 && col < screen->cols) {
        screen->grid[row][col].ch = ch;
        screen->grid[row][col].fg_color = 7; // Default White
    }
}

// --- ADDED: செல் (Cell) மெமரியில் எழுத்து மற்றும் நிறத்தை (Foreground Color) ஒன்றாகப் பதிவு செய்யும் பங்க்ஷன் ---
void screen_put_char_color(VirtualScreen *screen, int row, int col, char ch, int fg_color) {
    if (row >= 0 && row < screen->rows && col >= 0 && col < screen->cols) {
        screen->grid[row][col].ch = ch;
        screen->grid[row][col].fg_color = fg_color; // Green (2) அல்லது வேறு நிறத்தை சேமிக்க
    }
}

void screen_destroy(VirtualScreen *screen) {
    for (int r = 0; r < screen->rows; r++) {
        free(screen->grid[r]);
    }
    free(screen->grid);
    free(screen);
}
