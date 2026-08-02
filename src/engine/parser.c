// src/engine/parser.c
#include "parser.h"
#include <stdlib.h>

AnsiParser* parser_create() {
    AnsiParser *parser = (AnsiParser*)malloc(sizeof(AnsiParser));
    parser->state = STATE_NORMAL;
    return parser;
}

void parser_feed_char(AnsiParser *parser, VirtualScreen *scr, FloatingWindow *win, char ch) {
    switch (parser->state) {
        case STATE_NORMAL:
            if (ch == '\033') { // Escape Byte (0x1B)
                parser->state = STATE_ESC;
            } else {
                window_put_char(scr, win, ch); // சாதாரண எழுத்து - விண்டோவில் எழுது
            }
            break;

        case STATE_ESC:
            if (ch == '[') {
                parser->state = STATE_CSI; // Control Sequence ஆரம்பம் (\033[)
            } else {
                parser->state = STATE_NORMAL;
                if (ch != '\033') {
                    window_put_char(scr, win, ch);
                }
            }
            break;

        case STATE_CSI:
            // CSI குறியீடுகள் எப்போதும் 0x40 ('@') முதல் 0x7E ('~') வரையிலான எழுத்துக்களில் முடியும்
            if (ch >= 0x40 && ch <= 0x7E) {
                // --- புதிய CSI எஸ்கேப் கோடு கையாளுதல் (Handlers) ---
                if (ch == 'K') {
                    // \033[K -> கர்சர் இருக்கும் இடத்திலிருந்து வரியின் கடைசி வரை அழித்தல் (Erase to End of Line)
                    int abs_r = win->x + 1 + win->cur_r;
                    for (int c = win->cur_c; c < win->width - 2; c++) {
                        scr->grid[abs_r][win->y + 1 + c].ch = ' ';
                    }
                } 
                else if (ch == 'J') {
                    // \033[2J -> விண்டோவை முழுமையாகத் துடைத்தல் (Clear Display)
                    for (int r = 0; r < win->height - 2; r++) {
                        for (int c = 0; c < win->width - 2; c++) {
                            scr->grid[win->x + 1 + r][win->y + 1 + c].ch = ' ';
                        }
                    }
                    win->cur_r = 0;
                    win->cur_c = 0;
                }

                parser->state = STATE_NORMAL; // குறியீடு முடிந்தது! மீண்டும் Normal State-க்கு திரும்பு
            }
            break;
    }
}

void parser_destroy(AnsiParser *parser) {
    free(parser);
}
