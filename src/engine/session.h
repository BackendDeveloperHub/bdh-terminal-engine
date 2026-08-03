// include/engine/session.h - BDH Terminal Session Management Header (Fixed)
#ifndef SESSION_H
#define SESSION_H

#include <sys/types.h>
#include "ui/panes.h"
#include "ui/tabs.h"          // Added for TabBar
#include "ui/statusbar.h"     // Added for StatusBar
#include "engine/parser.h"
#include "engine/scanner.h"   // Added for TokenScanner
#include "engine/clipboard.h" // Added for Clipboard
#include "engine/screen.h"    // Added for VirtualScreen

#define MAX_SESSIONS 6

typedef struct {
    int id;
    int master_fd;
    pid_t pid;
    FloatingWindow *win;
    AnsiParser *parser;
    int is_alive;
} TerminalSession;

// Session Management Functions:
void sessions_init_all(TerminalSession sessions[MAX_SESSIONS], char *shell_argv[], int pty_rows, int pty_cols, int scr_cols, int scr_rows, TabBar *tab_bar, char *tab_names[]);
void sessions_cleanup_all(TerminalSession sessions[MAX_SESSIONS], TabBar *tab_bar, StatusBar *status_bar, TokenScanner *token_scanner, Clipboard *engine_cb, VirtualScreen *scr);

#endif
