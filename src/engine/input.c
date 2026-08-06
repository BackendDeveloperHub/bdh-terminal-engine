// src/engine/input.c - BDH Keyboard & Mouse Input Processing Module (Duplicate mouse_parse_sgr Removed!)
#include "input.h"
#include "engine/mouse.h" // <-- mouse_parse_sgr() ஃபங்ஷனுக்காக சேர்க்கப்பட்டுள்ளது
#include <stdio.h>
#include <string.h>

// குறிப்பு: mouse_parse_sgr() ஃபங்ஷன் src/engine/mouse.c ஃபைலில் உள்ளதால் இங்கிருந்து நீக்கப்பட்டுவிட்டது!

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
