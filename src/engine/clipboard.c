// src/engine/clipboard.c
#include "clipboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// புதிய கிளிப்போர்டு மெமரியை உருவாக்குதல்
Clipboard* clipboard_create(void) {
    Clipboard *cb = (Clipboard*)malloc(sizeof(Clipboard));
    if (!cb) return NULL;
    
    cb->text = NULL;
    cb->length = 0;
    return cb;
}

// டெக்ஸ்டை கிளிப்போர்டில் சேமித்தல் (Copy / Yank)
void clipboard_set(Clipboard *cb, const char *text, size_t len) {
    if (!cb || !text || len == 0) return;

    // பழைய டேட்டா இருந்தால் மெமரியை விடுவித்தல்
    if (cb->text) {
        free(cb->text);
    }

    // புதிய டேட்டாவுக்கு Memory Allocation (+1 for Null Terminator)
    cb->text = (char*)malloc(len + 1);
    if (!cb->text) {
        cb->length = 0;
        return;
    }

    memcpy(cb->text, text, len);
    cb->text[len] = '\0';
    cb->length = len;
}

// கிளிப்போர்டில் உள்ள டெக்ஸ்டை எடுத்தல் (Paste)
const char* clipboard_get(Clipboard *cb) {
    if (!cb || !cb->text) {
        return "";
    }
    return cb->text;
}

// கிளிப்போர்டை காலியாக்குதல்
void clipboard_clear(Clipboard *cb) {
    if (!cb) return;
    if (cb->text) {
        free(cb->text);
        cb->text = NULL;
    }
    cb->length = 0;
}

// கிளிப்போர்டு மெமரியை முழுமையாக அழித்தல் (Cleanup)
void clipboard_destroy(Clipboard *cb) {
    if (!cb) return;
    clipboard_clear(cb);
    free(cb);
}

// --- Bonus: Advanced Terminal OSC 52 Bridge ---
// இந்த Escape Sequence-ஐ பிரிண்ட் செய்தால், Host Terminal-ன் OS Clipboard-க்கு டெக்ஸ்ட் சென்றுவிடும்!
void clipboard_send_osc52(const char *text) {
    if (!text) return;
    // குறிப்பு: முழுமையான OSC 52-க்கு Base64 encoding தேவை. 
    // இது எதிர்கால OS integration-க்கான placeholder core function.
    printf("\033]52;c;%s\a", text);
    fflush(stdout);
}
