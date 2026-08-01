// src/engine/clipboard.h
#ifndef BDH_CLIPBOARD_H
#define BDH_CLIPBOARD_H

#include <stddef.h>

// கிளிப்போர்டு நினைவக அமைப்பு
typedef struct {
    char *text;     // சேமிக்கப்பட்ட டெக்ஸ்ட் (Dynamic Buffer)
    size_t length;  // டெக்ஸ்டின் நீளம்
} Clipboard;

// கிளிப்போர்டு பங்க்ஷன்கள்
Clipboard* clipboard_create(void);
void clipboard_set(Clipboard *cb, const char *text, size_t len);
const char* clipboard_get(Clipboard *cb);
void clipboard_clear(Clipboard *cb);
void clipboard_destroy(Clipboard *cb);

// Bonus: Host OS Clipboard-க்கு அனுப்ப (OSC 52 Protocol)
void clipboard_send_osc52(const char *text);

#endif // BDH_CLIPBOARD_H
