// src/engine/scanner.c
#include "engine/scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// புதிய Scanner மெமரியை உருவாக்குதல்
TokenScanner* scanner_create(void) {
    TokenScanner *scanner = (TokenScanner*)malloc(sizeof(TokenScanner));
    if (!scanner) return NULL;
    
    scanner->count = 0;
    scanner->is_scanning_mode = 0;
    return scanner;
}

// Token-க்கு தகுதியான எழுத்துக்களா என சரிபார்க்கும் Helper Function
static int is_token_char(char c) {
    return isalnum((unsigned char)c) || 
           c == '.' || c == '/' || c == '-' || c == '_' || 
           c == ':' || c == '?' || c == '=' || c == '&' || 
           c == '%' || c == '~';
}

// UUID ஃபார்மட் (8-4-4-4-12) சரியாக உள்ளதா என சரிபார்க்க
static int is_uuid_pattern(const char *str) {
    if (strlen(str) < 36) return 0;
    for (int i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (str[i] != '-') return 0;
        } else if (!isxdigit((unsigned char)str[i])) {
            return 0;
        }
    }
    return 1;
}

// ஒரு வரியில் உள்ள வார்த்தைகளில் இருந்து Token-ஐ கண்டறிந்து சேமித்தல்
static void detect_and_add_token(TokenScanner *scanner, const char *word, int row, int col) {
    if (scanner->count >= MAX_DETECTED_TOKENS || !word || strlen(word) < 4) return;

    TokenType type;
    int valid = 0;

    // 1. URL Detection (http:// or https://)
    if (strncmp(word, "http://", 7) == 0 || strncmp(word, "https://", 8) == 0) {
        type = TOKEN_TYPE_URL;
        valid = 1;
    }
    // 2. IP Address Detection (Simple prefix check for common IPs)
    else if (strncmp(word, "192.168.", 8) == 0 || strncmp(word, "10.", 3) == 0 || 
             strncmp(word, "172.", 4) == 0 || strncmp(word, "127.0.", 6) == 0) {
        type = TOKEN_TYPE_IP;
        valid = 1;
    }
    // 3. UUID Detection (36 chars standard format)
    else if (strlen(word) >= 36 && is_uuid_pattern(word)) {
        type = TOKEN_TYPE_UUID;
        valid = 1;
    }
    // 4. Linux Path Detection (/home, /var, /etc, /tmp)
    else if (word[0] == '/' && (strncmp(word, "/home", 5) == 0 || strncmp(word, "/var", 4) == 0 ||
                                strncmp(word, "/etc", 4) == 0 || strncmp(word, "/tmp", 4) == 0)) {
        type = TOKEN_TYPE_PATH;
        valid = 1;
    }

    // தகுதியான Token என்றால் Scanner-ல் சேர்க்கிறோம்:
    if (valid) {
        DetectedToken *tok = &scanner->tokens[scanner->count];
        tok->id = scanner->count + 1; // [1], [2], [3]...
        strncpy(tok->text, word, sizeof(tok->text) - 1);
        tok->text[sizeof(tok->text) - 1] = '\0';
        tok->type = type;
        tok->row = row;
        tok->col = col;
        scanner->count++;
    }
}

// ஸ்கிரீன் முழுவதையும் ஸ்கேன் செய்து Token-களை கண்டறியும் மெயின் பங்க்ஷன்
int scanner_scan_screen(TokenScanner *scanner, VirtualScreen *scr) {
    if (!scanner || !scr) return 0;

    scanner->count = 0; // பழைய Token-களை கிளியர் செய்கிறோம்
    char word_buf[256];
    int word_idx = 0;

    // ஸ்கிரீனின் ஒவ்வொரு வரியையும் படிக்கிறோம்
    for (int r = 0; r < scr->rows && scanner->count < MAX_DETECTED_TOKENS; r++) {
        word_idx = 0;
        int start_col = 0;

        for (int c = 0; c < scr->cols; c++) {
            // குறிப்பு: VirtualScreen-ன் character structure-க்கு ஏற்ப scr->cells[r][c].ch என்று மாற்றிக்கொள்ளலாம்
            char ch = ' '; // Placeholder: VirtualScreen-ல் இருந்து எழுத்தை எடுக்கும் இடம்

            if (is_token_char(ch)) {
                if (word_idx == 0) start_col = c;
                if (word_idx < (int)sizeof(word_buf) - 1) {
                    word_buf[word_idx++] = ch;
                }
            } else {
                if (word_idx > 0) {
                    word_buf[word_idx] = '\0';
                    detect_and_add_token(scanner, word_buf, r, start_col);
                    word_idx = 0;
                }
            }
        }

        // வரியின் கடைசியில் வார்த்தை முடிந்தால் செக் செய்ய:
        if (word_idx > 0) {
            word_buf[word_idx] = '\0';
            detect_and_add_token(scanner, word_buf, r, start_col);
        }
    }

    return scanner->count;
}

// யூசர் நம்பரை (1, 2, 3...) அழுத்தியதும் அந்த Token-ஐ Clipboard-ல் காப்பி செய்ய:
int scanner_copy_by_id(TokenScanner *scanner, int token_id, Clipboard *cb) {
    if (!scanner || !cb || token_id < 1 || token_id > scanner->count) {
        return 0; // Invalid ID
    }

    // ID-க்குரிய Token-ஐ எடுத்து கிளிப்போர்டில் சேர்க்கிறோம்:
    DetectedToken *target = &scanner->tokens[token_id - 1];
    clipboard_set(cb, target->text, strlen(target->text));
    
    return 1; // Success!
}

// மெமரியை முழுமையாக அழித்தல்
void scanner_destroy(TokenScanner *scanner) {
    if (scanner) {
        free(scanner);
    }
}
