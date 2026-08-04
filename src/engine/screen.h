// src/engine/screen.h - BDH Virtual Screen Buffer Header (Updated)
#ifndef BDH_SCREEN_H
#define BDH_SCREEN_H

#define DEFAULT_COLS 80
#define DEFAULT_ROWS 24

// திரையில் உள்ள ஒவ்வொரு எழுத்துக்கும் (Character + Color) ஒரு Cell
typedef struct {
    char ch;
    unsigned char fg_color;
    unsigned char bg_color;
} ScreenCell;

// ஒட்டுமொத்த திரையின் மெமரி அமைப்பு
typedef struct {
    int cols;
    int rows;
    ScreenCell **grid; // 2D Array [rows][cols]
} VirtualScreen;

// Screen-ஐ உருவாக்கும் மற்றும் அழிக்கும் பங்க்ஷன்கள்
VirtualScreen* screen_create(int rows, int cols);
void screen_destroy(VirtualScreen *screen);
void screen_clear(VirtualScreen *screen);
void screen_put_char(VirtualScreen *screen, int row, int col, char ch);

// --- ADDED: செல் (Cell) மெமரியில் எழுத்து மற்றும் நிறத்தை (Foreground Color) ஒன்றாகப் பதிவு செய்யும் பங்க்ஷன் ---
void screen_put_char_color(VirtualScreen *screen, int row, int col, char ch, int fg_color);

#endif // BDH_SCREEN_H
