// src/engine/mouse.c - BDH Terminal Engine SGR Mouse Event Parser
#include "engine/mouse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// SGR Mouse sequence parser: "\033[<btn;col;rowM" (Press) அல்லது "\033[<btn;col;rowm" (Release)
int mouse_parse_sgr(const char *buf, MouseEvent *event) {
    if (!buf || !event) return 0;

    // "\033[<" என்று தொடங்குவதை உறுதி செய்ய வேண்டும்
    if (strncmp(buf, "\033[<", 3) != 0) {
        return 0;
    }

    int btn = 0, col = 0, row = 0;
    char type = 0;

    // sscanf மூலம் எண்களையும் கடைசி எழுத்தையும் (M அல்லது m) துல்லியமாகப் பிரித்தெடுத்தல்
    int parsed = sscanf(buf + 3, "%d;%d;%d%c", &btn, &col, &row, &type);
    if (parsed != 4) {
        return 0;
    }

    // SGR Protocol 1-based (1,1)-ல் தொடங்கும்; நமது Engine 0-based (0,0)-ல் இயங்கும்
    event->button = (MouseButton)btn;
    event->col = (col > 0) ? (col - 1) : 0;
    event->row = (row > 0) ? (row - 1) : 0;
    event->is_release = (type == 'm') ? 1 : 0;

    return 1;
}

