// src/engine/input.c
#include "input.h"
#include <string.h>

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

    // மற்ற எல்லா எழுத்துக்களும் சாதாரண Shell உள்ளீடுகள்
    return INPUT_ACTION_NORMAL;
}
