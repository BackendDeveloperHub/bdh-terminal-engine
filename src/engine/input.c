// src/engine/input.c - BDH Keyboard & Mouse Input Processing Module
#include "input.h"
#include <stdio.h>
#include <string.h>

// மவுஸ் சீக்வன்ஸ் (எ.கா: "\033[<0;15;1M") பிரித்தெடுக்கும் ஃபங்ஷன் (SGR 1006 Mode)
int mouse_parse_sgr(const char *buf, MouseEvent *event) {
    if (!buf || !event) {
        return 0;
    }

    if (buf[0] != '\033' || buf[1] != '[' || buf[2] != '<') {
        return 0; // இது மவுஸ் ஈவென்ட் இல்லை (சாதாரண கீபோர்டு உள்ளீடு)
    }

    int btn, col, row;
    char type;
    if (sscanf(buf + 3, "%d;%d;%d%c", &btn, &col, &row, &type) == 4) {
        event->button = (MouseButton)btn;
        event->col = col - 1; // Terminal 1-indexed, C array 0-indexed
        event->row = row - 1;
        event->is_release = (type == 'm') ? 1 : 0;
        return 1;
    }
    return 0;
}

InputAction input_parse_key(char key, Clipboard *cb, const char *default_copy_text) {
    // Ctrl + Q (ASCII 17) -> Exit Engine
    if (key == 17) {
        return INPUT_ACTION_EXIT;
    }
    // Ctrl + A (ASCII 1) -> Switch Window
    else if (key == 1) {
        return INPUT_ACTION_SWITCH_WIN;
    }
    // Ctrl + P (ASCII 16) -> Paste from Clipboard
    else if (key == 16) {
        return INPUT_ACTION_PASTE;
    }
    // Ctrl + Y (ASCII 25) -> Copy / Yank to Clipboard
    else if (key == 25) {
        if (cb && default_copy_text) {
            clipboard_set(cb, default_copy_text, strlen(default_copy_text));
        }
        return INPUT_ACTION_COPY;
    }
    // Ctrl + B (ASCII 2) -> Open GUI Browser
    else if (key == 2) {
        return INPUT_ACTION_OPEN_BROWSER;
    }

    // மற்ற எல்லா எழுத்துக்களும் சாதாரண Shell உள்ளீடுகள்
    return INPUT_ACTION_NORMAL;
}
