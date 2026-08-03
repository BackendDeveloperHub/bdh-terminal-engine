// include/engine/input.h - BDH Keyboard & Mouse Input Header
#ifndef BDH_INPUT_H
#define BDH_INPUT_H

#include "engine/clipboard.h"
#include "engine/mouse.h"  // <-- மவுஸ் ஈவென்ட் ஸ்ட்ரக்ட்சர் ஹெட்டரை இணைத்துள்ளோம்

// கீபோர்டு அழுத்தும்போது என்ஜின் என்ன செய்ய வேண்டும் என்ற நிலைகள்
typedef enum {
    INPUT_ACTION_NORMAL,      // சாதாரண எழுத்து (Shell-க்கு அனுப்ப வேண்டும்)
    INPUT_ACTION_EXIT,        // Ctrl + Q (Engine-ல் இருந்து வெளியேற)
    INPUT_ACTION_SWITCH_WIN,  // Ctrl + A (அடுத்த விண்டோவுக்கு மாற)
    INPUT_ACTION_PASTE,       // Ctrl + P (Clipboard-ல் இருந்து Paste செய்ய)
    INPUT_ACTION_COPY,        // Ctrl + Y (Clipboard-க்கு Copy செய்ய)
    INPUT_ACTION_OPEN_BROWSER // Ctrl + B (GUI/CLI Browser திறக்க)
} InputAction;

// --- ஃபங்ஷன் டிக்ளரேஷன்கள் ---

// மவுஸ் உள்ளீட்டைப் பகுப்பாய்வு செய்யும் பங்க்ஷன் (SGR 1006 Mode)
int mouse_parse_sgr(const char *buf, MouseEvent *event);

// கீபோர்டு உள்ளீட்டைப் பகுப்பாய்வு செய்யும் பங்க்ஷன்
InputAction input_parse_key(char key, Clipboard *cb, const char *default_copy_text);

#endif // BDH_INPUT_H
