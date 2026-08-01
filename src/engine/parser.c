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
            if (ch == '\033') { // Escape Byte (0x1B) வந்திருக்கிறது
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
            // எ.கா: 'm' (color), 'h'/'l' (mode like ?2004h), 'J'/'K' (clear), 'A'..'D' (cursor)
            if (ch >= 0x40 && ch <= 0x7E) {
                parser->state = STATE_NORMAL; // குறியீடு முடிந்தது! மீண்டும் Normal State-க்கு திரும்பு
            }
            // அதுவரை நடுவில் வரும் எண்கள் ('2','0','0','4'), '?' போன்றவற்றை புறக்கணிக்கவும் (Ignore)
            break;
    }
}

void parser_destroy(AnsiParser *parser) {
    free(parser);
}
