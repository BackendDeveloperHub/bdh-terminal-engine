// include/engine/mouse.h
#ifndef BDH_MOUSE_H
#define BDH_MOUSE_H

typedef enum {
    MOUSE_BTN_LEFT = 0,
    MOUSE_BTN_MIDDLE = 1,
    MOUSE_BTN_RIGHT = 2,
    MOUSE_BTN_RELEASE = 3,
    MOUSE_WHEEL_UP = 64,
    MOUSE_WHEEL_DOWN = 65
} MouseButton;

typedef struct {
    MouseButton button;
    int col;       // 0 to 219 (For 220 cols)
    int row;       // 0 to 49  (For 50 rows)
    int is_release;// 1 = released, 0 = pressed
} MouseEvent;

#endif // BDH_MOUSE_H
