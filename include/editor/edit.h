// include/editor/edit.h - BDH Built-in Custom CLI Text Editor Header
#ifndef BDH_EDIT_H
#define BDH_EDIT_H

#include "engine/screen.h"

typedef struct {
    int size;
    char *chars;
} EditorRow;

typedef struct {
    int cx, cy;         // கர்சரின் தற்போதைய X, Y இடம்
    int row_offset;     // Vertical Scroll Offset
    int num_rows;       // ஃபைலில் உள்ள மொத்த வரிகள்
    EditorRow *row;     // வரிகளைச் சேமிக்கும் Dynamic Array
    char filename[256]; // ஃபைலின் பெயர்
    int is_dirty;       // மாற்றங்கள் செய்யப்பட்டுள்ளதா? (0 = Saved, 1 = Modified)
    int is_active;      // Editor Mode ஆன்/ஆஃப் நிலை (0 = PTY Shell Mode, 1 = Editor Mode)
} EditorState;

EditorState* editor_create(void);
int editor_open(EditorState *ed, const char *filename);
int editor_save(EditorState *ed);
void editor_insert_char(EditorState *ed, int c);
void editor_delete_char(EditorState *ed);
void editor_insert_newline(EditorState *ed);
void editor_handle_key(EditorState *ed, const char *buf, int len);
void editor_draw(EditorState *ed, VirtualScreen *scr, int max_rows, int max_cols);
void editor_destroy(EditorState *ed);

#endif // BDH_EDIT_H
