// src/editor/edit.c - BDH Built-in Lightweight CLI Text Editor Implementation (100% Bulletproof Fixed)
#include "editor/edit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

EditorState* editor_create(void) {
    EditorState *ed = (EditorState*)malloc(sizeof(EditorState));
    if (!ed) return NULL;
    ed->cx = 0;
    ed->cy = 0;
    ed->row_offset = 0;
    ed->num_rows = 0;
    ed->row = NULL;
    ed->filename[0] = '\0';
    ed->is_dirty = 0;
    ed->is_active = 0;
    return ed;
}

// 1. ஃபைலைத் திறத்தல் (Open File):
int editor_open(EditorState *ed, const char *filename) {
    strncpy(ed->filename, filename, sizeof(ed->filename) - 1);
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        ed->row = (EditorRow*)malloc(sizeof(EditorRow));
        ed->row[0].size = 0;
        ed->row[0].chars = strdup("");
        ed->num_rows = 1;
        return 0;
    }

    char *line = NULL;
    size_t cap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &cap, fp)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            linelen--;
        
        ed->row = (EditorRow*)realloc(ed->row, sizeof(EditorRow) * (ed->num_rows + 1));
        ed->row[ed->num_rows].size = linelen;
        ed->row[ed->num_rows].chars = (char*)malloc(linelen + 1);
        memcpy(ed->row[ed->num_rows].chars, line, linelen);
        ed->row[ed->num_rows].chars[linelen] = '\0';
        ed->num_rows++;
    }
    free(line);
    fclose(fp);
    if (ed->num_rows == 0) {
        ed->row = (EditorRow*)malloc(sizeof(EditorRow));
        ed->row[0].size = 0;
        ed->row[0].chars = strdup("");
        ed->num_rows = 1;
    }
    return 1;
}

// 2. ஃபைலை சேமித்தல் (Save File - Ctrl + S):
int editor_save(EditorState *ed) {
    if (strlen(ed->filename) == 0) return 0;
    FILE *fp = fopen(ed->filename, "w");
    if (!fp) return 0;
    for (int i = 0; i < ed->num_rows; i++) {
        fprintf(fp, "%s\n", ed->row[i].chars);
    }
    fclose(fp);
    ed->is_dirty = 0;
    return 1;
}

// 3. எழுத்துக்களைச் செருகுதல் (Insert Character):
void editor_insert_char(EditorState *ed, int c) {
    if (ed->cy >= ed->num_rows) return;
    EditorRow *row = &ed->row[ed->cy];
    if (ed->cx > row->size) ed->cx = row->size;
    row->chars = (char*)realloc(row->chars, row->size + 2);
    memmove(&row->chars[ed->cx + 1], &row->chars[ed->cx], row->size - ed->cx + 1);
    row->chars[ed->cx] = c;
    row->size++;
    ed->cx++;
    ed->is_dirty = 1;
}

// 4. எழுத்துக்களை நீக்குதல் (Backspace):
void editor_delete_char(EditorState *ed) {
    if (ed->cy >= ed->num_rows || (ed->cx == 0 && ed->cy == 0)) return;
    EditorRow *row = &ed->row[ed->cy];
    if (ed->cx > row->size) ed->cx = row->size;
    if (ed->cx > 0) {
        memmove(&row->chars[ed->cx - 1], &row->chars[ed->cx], row->size - ed->cx + 1);
        row->size--;
        ed->cx--;
        ed->is_dirty = 1;
    }
}

// 5. புதிய வரி செருகுதல் (Enter Key):
void editor_insert_newline(EditorState *ed) {
    ed->row = (EditorRow*)realloc(ed->row, sizeof(EditorRow) * (ed->num_rows + 1));
    memmove(&ed->row[ed->cy + 2], &ed->row[ed->cy + 1], sizeof(EditorRow) * (ed->num_rows - ed->cy - 1));
    
    EditorRow *cur = &ed->row[ed->cy];
    int remain = cur->size - ed->cx;
    
    ed->row[ed->cy + 1].size = remain;
    ed->row[ed->cy + 1].chars = (char*)malloc(remain + 1);
    memcpy(ed->row[ed->cy + 1].chars, &cur->chars[ed->cx], remain);
    ed->row[ed->cy + 1].chars[remain] = '\0';
    
    cur->size = ed->cx;
    cur->chars[ed->cx] = '\0';
    
    ed->num_rows++;
    ed->cy++;
    ed->cx = 0;
    ed->is_dirty = 1;
}

// 6. விசைப்பலகை உள்ளீடுகளைக் கையாளுதல் (Key Handler):
void editor_handle_key(EditorState *ed, const char *buf, int len) {
    if (len == 0) return;
    unsigned char c = buf[0];

    // Arrow Keys (\033[A, \033[B, \033[C, \033[D):
    if (c == '\033' && len >= 3 && buf[1] == '[') {
        if (buf[2] == 'A' && ed->cy > 0) ed->cy--; // UP
        else if (buf[2] == 'B' && ed->cy < ed->num_rows - 1) ed->cy++; // DOWN
        else if (buf[2] == 'C') { // RIGHT
            if (ed->cy < ed->num_rows && ed->cx < ed->row[ed->cy].size) ed->cx++;
        }
        else if (buf[2] == 'D' && ed->cx > 0) ed->cx--; // LEFT
        
        // 🔥 BOUNDARY CLAMP FIX: வரி மாறும்போது cx அளவை வரியின் நீளத்திற்குள் கட்டுப்படுத்த வேண்டும்
        if (ed->cy < ed->num_rows && ed->cx > ed->row[ed->cy].size) {
            ed->cx = ed->row[ed->cy].size;
        }
        return;
    }

    if (c == '\r' || c == '\n') {
        editor_insert_newline(ed);
    } 
    else if (c == 127 || c == '\b' || c == 0x08) {
        editor_delete_char(ed);
    } 
    else if (c >= 32 && c <= 126) {
        editor_insert_char(ed, c);
    }
}

// 7. 🔥 Explicit Row Rendering (100% Guaranteed Display & Auto-Scroll Fixed):
void editor_draw(EditorState *ed, VirtualScreen *scr, int max_rows, int max_cols) {
    if (!ed) return;
    (void)scr;

    char buf[1024];
    int len;

    // 1. Top Title Bar (Row 1):
    len = snprintf(buf, sizeof(buf), 
             "\033[1;1H\033[7m--- [ BDH Edit : %s %s ] --- (Ctrl+S: Save | Ctrl+X: Exit) ---\033[0m\033[K", 
             strlen(ed->filename) > 0 ? ed->filename : "Untitled", 
             ed->is_dirty ? "[+]" : "");
    write(STDOUT_FILENO, buf, len);

    // 2. Text Buffer-ஐ நடுவில் அச்சிடுதல்:
    int draw_rows = max_rows - 3;
    if (draw_rows < 5) draw_rows = 15;

    // Auto-Scroll Logic:
    if (ed->cy < ed->row_offset) {
        ed->row_offset = ed->cy;
    }
    if (ed->cy >= ed->row_offset + draw_rows) {
        ed->row_offset = ed->cy - draw_rows + 1;
    }

    for (int r = 1; r <= draw_rows; r++) {
        int file_row = ed->row_offset + r - 1;
        len = snprintf(buf, sizeof(buf), "\033[%d;1H\033[K", r + 1);
        write(STDOUT_FILENO, buf, len);

        if (file_row < ed->num_rows) {
            int sz = ed->row[file_row].size;
            if (sz > max_cols - 1) sz = max_cols - 1;
            if (sz > 0) {
                write(STDOUT_FILENO, ed->row[file_row].chars, sz);
            }
        }
    }

    // 3. Bottom Help / Status Bar (Row max_rows - 1):
    len = snprintf(buf, sizeof(buf), 
             "\033[%d;1H\033[1;33m[ BDH Edit Active ] | Arrow Keys: Move | Enter/Backspace: Edit | Ctrl+S: Save | Ctrl+X: Exit\033[0m\033[K", 
             max_rows - 1);
    write(STDOUT_FILENO, buf, len);

    // 4. கர்சரை நாம் டைப் செய்யும் சரியான இடத்தில் (cx, cy) உட்கார வைப்பது:
    int screen_cursor_r = (ed->cy - ed->row_offset) + 2;
    int screen_cursor_c = ed->cx + 1;
    len = snprintf(buf, sizeof(buf), "\033[%d;%dH", screen_cursor_r, screen_cursor_c);
    write(STDOUT_FILENO, buf, len);
}

void editor_destroy(EditorState *ed) {
    if (!ed) return;
    for (int i = 0; i < ed->num_rows; i++) {
        free(ed->row[i].chars);
    }
    free(ed->row);
    free(ed);
}
