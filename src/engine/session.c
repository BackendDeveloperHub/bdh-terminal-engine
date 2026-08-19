// src/engine/session.c - BDH Terminal Session Management Implementation (Zero-Warning Clean Build)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "engine/session.h"
#include "engine/pty.h"
#include "ui/wm.h"
#include "ui/tabs.h"
#include "ui/statusbar.h"
// 🔥 FIX: engine/scanner.h நீக்கப்பட்டுவிட்டது!
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

        // --- Clean Array Title Buffer ---
        char win_title[64];
        snprintf(win_title, sizeof(win_title), "[ TAB %d/%d : %s %s ]", 
                 i + 1, MAX_SESSIONS, tab_names[i], (i == 0) ? "(ACTIVE) *" : "");
                 
        // 🔥 THE ARCHITECT FIX: விண்டோ உயரத்தை (scr_rows - 12) ஆகக் குறைத்துள்ளோம்!
        sessions[i].win = window_create(i, 0, 1, scr_cols, scr_rows - 12, win_title, (i == 0) ? 1 : 0);
        sessions[i].parser = parser_create();
        
        if (sessions[i].win != NULL && sessions[i].parser != NULL) {
            sessions[i].is_alive = 1;
            if (tab_bar) {
                tabs_add(tab_bar, tab_names[i]);
            }
        }
    }
}

// 🔥 FIX: TokenScanner ஆர்கியுமெண்ட் நீக்கப்பட்டுவிட்டது!
void sessions_cleanup_all(TerminalSession sessions[MAX_SESSIONS], TabBar *tab_bar, StatusBar *status_bar, Clipboard *engine_cb, VirtualScreen *scr) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].parser) parser_destroy(sessions[i].parser);
        if (sessions[i].win) {
            window_destroy(sessions[i].win); // --- FIX: No free(win->title) needed for char[64] array! ---
        }
        if (sessions[i].is_alive && sessions[i].master_fd >= 0) {
            close(sessions[i].master_fd);
        }
    }
    
    if (tab_bar) tabs_destroy(tab_bar);
    if (status_bar) statusbar_destroy(status_bar);
    // 🔥 FIX: scanner_destroy நீக்கப்பட்டுவிட்டது!
    if (engine_cb) clipboard_destroy(engine_cb);
    if (scr) screen_destroy(scr);
}
