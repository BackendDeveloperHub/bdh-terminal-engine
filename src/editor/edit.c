// src/editor/edit.c - BDH Built-in Lightweight CLI Text Editor Implementation
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
        // புதிய ஃபைல் என்றால் ஒரு காலி வரியை உருவாக்குதல்
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

// 7. 🔥 Virtual Screen-ல் எடிட்டரை வரைதல் (Zero Glitch Footer Safe Render):
void editor_draw(EditorState *ed, VirtualScreen *scr, int max_rows, int max_cols) {
    if (!ed || !scr) return;

    // 1. Top Bar ([ BDH Built-in Editor : filename ]):
    char header[256];
    snprintf(header, sizeof(header), "--- [ BDH Edit : %s %s ] --- (Ctrl+S: Save | Ctrl+X: Exit)", 
             strlen(ed->filename) > 0 ? ed->filename : "Untitled", ed->is_dirty ? "[+]" : "");
    for (int c = 0; c < max_cols && c < scr->cols; c++) {
        scr->buffer[0][c].ch = (c < (int)strlen(header)) ? header[c] : ' ';
    }

    // 2. Text Buffer-ஐ நடுவில் அச்சிடுதல் (கீழே 12 வரிகள் Footer-க்காக ஒதுக்கப்பட்டுள்ளது):
    int draw_rows = max_rows - 13;
    for (int r = 1; r <= draw_rows; r++) {
        int file_row = ed->row_offset + r - 1;
        for (int c = 0; c < max_cols && c < scr->cols; c++) {
            if (file_row < ed->num_rows && c < ed->row[file_row].size) {
                scr->buffer[r][c].ch = ed->row[file_row].chars[c];
            } else {
                scr->buffer[r][c].ch = ' ';
            }
        }
    }
}

void editor_destroy(EditorState *ed) {
    if (!ed) return;
    for (int i = 0; i < ed->num_rows; i++) {
        free(ed->row[i].chars);
    }
    free(ed->row);
    free(ed);
}
