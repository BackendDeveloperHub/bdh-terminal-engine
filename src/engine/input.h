// src/engine/input.h
#ifndef BDH_INPUT_H
#define BDH_INPUT_H

#include "engine/clipboard.h"

// கீபோர்டு அழுத்தும்போது என்ஜின் என்ன செய்ய வேண்டும் என்ற நிலைகள்
typedef enum {
    INPUT_ACTION_NORMAL,      // சாதாரண எழுத்து (Shell-க்கு அனுப்ப வேண்டும்)
    INPUT_ACTION_EXIT,        // Ctrl + Q (Engine-ல் இருந்து வெளியேற)
    INPUT_ACTION_SWITCH_WIN,  // Ctrl + A (அடுத்த விண்டோவுக்கு மாற)
    INPUT_ACTION_PASTE,       // Ctrl + P (Clipboard-ல் இருந்து Paste செய்ய)
    INPUT_ACTION_COPY ,
    INPUT_ACTION_OPEN_BROWSER
// Ctrl + Y (Clipboard-க்கு Copy செய்ய)
} InputAction;

// கீபோர்டு உள்ளீட்டைப் பகுப்பாய்வு செய்யும் பங்க்ஷன்
InputAction input_parse_key(char key, Clipboard *cb, const char *default_copy_text);

#endif // BDH_INPUT_H
