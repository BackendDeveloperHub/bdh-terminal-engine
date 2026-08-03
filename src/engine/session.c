// src/engine/session.c - BDH Terminal Session Management Implementation (Signal 11 Fixed)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "engine/session.h"
#include "engine/pty.h"
#include "ui/wm.h"            // <-- FIX: Signal 11 (Pointer Truncation) எரரைத் தடுக்க wm.h சேர்க்கப்பட்டுள்ளது!
#include "ui/tabs.h"
#include "ui/statusbar.h"
#include "engine/scanner.h"
#include "engine/clipboard.h"
#include "engine/screen.h"

void sessions_init_all(TerminalSession sessions[MAX_SESSIONS], char *shell_argv[], int pty_rows, int pty_cols, int scr_cols, int scr_rows, TabBar *tab_bar, char *tab_names[]) {
    memset(sessions, 0, sizeof(TerminalSession) * MAX_SESSIONS);

    for (int i = 0; i < MAX_SESSIONS; i++) {
        sessions[i].id = i;
        sessions[i].master_fd = -1;
        sessions[i].is_alive = 0;
        sessions[i].win = NULL;
        sessions[i].parser = NULL;

        sessions[i].pid = pty_spawn(shell_argv, &sessions[i].master_fd, pty_rows, pty_cols);
        
        if (sessions[i].pid < 0 || sessions[i].master_fd < 0) {
            sessions[i].pid = pty_spawn((char *[]){"/bin/bash", NULL}, &sessions[i].master_fd, pty_rows, pty_cols);
            if (sessions[i].pid < 0 || sessions[i].master_fd < 0) {
                continue; 
            }
        }

        char win_title[64];
        snprintf(win_title, sizeof(win_title), "[ TAB %d/%d : %s %s ]", 
                 i + 1, MAX_SESSIONS, tab_names[i], (i == 0) ? "(ACTIVE) *" : "");
                 
        sessions[i].win = window_create(i, 0, 1, scr_cols, scr_rows - 2, win_title, (i == 0) ? 1 : 0);
        sessions[i].parser = parser_create();
        
        if (sessions[i].win != NULL && sessions[i].parser != NULL) {
            sessions[i].is_alive = 1;
            if (tab_bar) {
                tabs_add(tab_bar, tab_names[i]);
            }
        }
    }
}

void sessions_cleanup_all(TerminalSession sessions[MAX_SESSIONS], TabBar *tab_bar, StatusBar *status_bar, TokenScanner *token_scanner, Clipboard *engine_cb, VirtualScreen *scr) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].parser) parser_destroy(sessions[i].parser);
        if (sessions[i].win) window_destroy(sessions[i].win);
        if (sessions[i].is_alive && sessions[i].master_fd >= 0) {
            close(sessions[i].master_fd);
        }
    }
    
    if (tab_bar) tabs_destroy(tab_bar);
    if (status_bar) statusbar_destroy(status_bar);
    if (token_scanner) scanner_destroy(token_scanner);
    if (engine_cb) clipboard_destroy(engine_cb);
    if (scr) screen_destroy(scr);
}
