// src/engine/scanner.h
#ifndef BDH_SCANNER_H
#define BDH_SCANNER_H

#include "engine/screen.h"
#include "engine/clipboard.h"

#define MAX_DETECTED_TOKENS 10

typedef enum {
    TOKEN_TYPE_URL,
    TOKEN_TYPE_IP,
    TOKEN_TYPE_UUID,
    TOKEN_TYPE_PATH
} TokenType;

typedef struct {
    int id;               // [1], [2], [3] போன்ற எண்கள்
    char text[256];       // கண்டுபிடிக்கப்பட்ட URL அல்லது IP அல்லது UUID
    TokenType type;
    int row;
    int col;
} DetectedToken;

typedef struct {
    DetectedToken tokens[MAX_DETECTED_TOKENS];
    int count;
    int is_scanning_mode; // Ctrl + K அழுத்தும்போது 1 ஆகும்
} TokenScanner;

// Scanner பங்க்ஷன்கள்
TokenScanner* scanner_create(void);
int scanner_scan_screen(TokenScanner *scanner, VirtualScreen *scr);
int scanner_copy_by_id(TokenScanner *scanner, int token_id, Clipboard *cb);
void scanner_destroy(TokenScanner *scanner);

#endif // BDH_SCANNER_H
