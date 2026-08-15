// src/engine/screen.h - BDH Virtual Screen Buffer Header (Delta Rendering Ready)
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

// ஒட்டுமொத்த திரையின் மெமரி அமைப்பு (Double Buffered for Delta Rendering)
typedef struct {
    int cols;
    int rows;
    ScreenCell **grid;     // 🔥 Back Buffer: நாம் புதிதாக வரையும் எழுத்துக்கள் இதில் சேரும்
    ScreenCell **old_grid; // 🔥 Front Buffer: டெர்மினலில் தற்போது என்ன இருக்கிறது என்ற டேட்டா
} VirtualScreen;

// Screen-ஐ உருவாக்கும் மற்றும் அழிக்கும் பங்க்ஷன்கள்
VirtualScreen* screen_create(int rows, int cols);
void screen_destroy(VirtualScreen *screen);
void screen_clear(VirtualScreen *screen);

// 🔥 புதிய ஃபங்ஷன்: மொத்த ஸ்கிரீனையும் கட்டாயமாக ரெஃப்ரெஷ் செய்ய (Force Redraw)
void screen_force_redraw(VirtualScreen *screen);

void screen_put_char(VirtualScreen *screen, int row, int col, char ch);

// செல் (Cell) மெமரியில் எழுத்து மற்றும் நிறத்தை (Foreground Color) ஒன்றாகப் பதிவு செய்யும் பங்க்ஷன்
void screen_put_char_color(VirtualScreen *screen, int row, int col, char ch, int fg_color);

#endif // BDH_SCREEN_H
