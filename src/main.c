// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <string.h>
#include "engine/pty.h"
#include "engine/screen.h"
#include "ui/panes.h"
#include "engine/parser.h"
#include "engine/clipboard.h" // <-- 1. Clipboard Header இணைக்கப்பட்டுள்ளது!

#define MAX_SESSIONS 2

typedef struct {
    int id;
    int master_fd;
    pid_t pid;
    FloatingWindow *win;
    AnsiParser *parser;
} TerminalSession;

struct termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    cfmakeraw(&raw);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void render_all_sessions(VirtualScreen *scr, TerminalSession sessions[], int count) {
    screen_clear(scr);
    for (int i = 0; i < count; i++) {
        if (sessions[i].win->z_index == 0) {
            window_draw(scr, sessions[i].win);
        }
    }
    for (int i = 0; i < count; i++) {
        if (sessions[i].win->z_index == 1) {
            window_draw(scr, sessions[i].win);
        }
    }

    printf("\033[2J\033[H");
    for (int r = 0; r < scr->rows; r++) {
        for (int c = 0; c < scr->cols; c++) {
            putchar(scr->grid[r][c].ch);
        }
        putchar('\r');
        putchar('\n');
    }
    fflush(stdout);
}

int main() {
    char *shell_argv[] = {"/bin/bash", NULL};
    enable_raw_mode();

    VirtualScreen *scr = screen_create(24, 80);
    TerminalSession sessions[MAX_SESSIONS];
    int active_idx = 0;

    // --- 2. Engine Clipboard உருவாக்குதல் & Default Command Copy செய்தல் ---
    Clipboard *engine_cb = clipboard_create();
    const char *default_cmd = "echo BDH Clipboard Success!\n";
    clipboard_set(engine_cb, default_cmd, strlen(default_cmd));

    // --- Session 0 (Primary Window) ---
    sessions[0].id = 0;
    sessions[0].pid = pty_spawn(shell_argv, &sessions[0].master_fd);
    sessions[0].win = window_create(1, 1, 2, 60, 15, "[ 1: Bash - Primary (ACTIVE) ]", 1);
    sessions[0].parser = parser_create();

    // --- Session 1 (Secondary Window) ---
    sessions[1].id = 1;
    sessions[1].pid = pty_spawn(shell_argv, &sessions[1].master_fd);
    sessions[1].win = window_create(2, 6, 15, 60, 15, "[ 2: Bash - Secondary ]", 0);
    sessions[1].parser = parser_create();

    render_all_sessions(scr, sessions, MAX_SESSIONS);

    fd_set read_fds;
    char buffer[1024];
    ssize_t nread;
    int max_fd = 0;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        max_fd = STDIN_FILENO;

        for (int i = 0; i < MAX_SESSIONS; i++) {
            FD_SET(sessions[i].master_fd, &read_fds);
            if (sessions[i].master_fd > max_fd) {
                max_fd = sessions[i].master_fd;
            }
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) break;

        // --- Keyboard Input (Stdin) ---
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            nread = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (nread > 0) {
                // Ctrl + Q (ASCII 17) = Exit Engine
                if (buffer[0] == 17) {
                    break;
                }
                // Ctrl + A (ASCII 1) = Switch Windows
                else if (buffer[0] == 1) {
                    sessions[active_idx].win->z_index = 0;
                    sessions[active_idx].win->is_active = 0;
                    strncpy(sessions[active_idx].win->title, 
                            active_idx == 0 ? "[ 1: Bash - Primary ]" : "[ 2: Bash - Secondary ]", 63);

                    active_idx = 1 - active_idx;

                    sessions[active_idx].win->z_index = 1;
                    sessions[active_idx].win->is_active = 1;
                    strncpy(sessions[active_idx].win->title, 
                            active_idx == 0 ? "[ 1: Bash - Primary (ACTIVE) ]" : "[ 2: Bash - Secondary (ACTIVE) ]", 63);

                    render_all_sessions(scr, sessions, MAX_SESSIONS);
                }
                // Ctrl + P (ASCII 16) = Paste from Clipboard! <-- 3. PASTE HOTKEY
                else if (buffer[0] == 16) {
                    const char *paste_data = clipboard_get(engine_cb);
                    if (strlen(paste_data) > 0) {
                        write(sessions[active_idx].master_fd, paste_data, strlen(paste_data));
                    }
                }
                // Ctrl + Y (ASCII 25) = Yank/Copy a new command! <-- 4. COPY HOTKEY
                else if (buffer[0] == 25) {
                    const char *new_cmd = "ls -la /home\n";
                    clipboard_set(engine_cb, new_cmd, strlen(new_cmd));
                }
                // மற்ற எல்லா எழுத்துக்களையும் Active Shell-க்கு அனுப்பு
                else {
                    write(sessions[active_idx].master_fd, buffer, nread);
                }
            }
        }

        // --- Shell Output (PTY Master FDs) ---
        int needs_render = 0;
        for (int i = 0; i < MAX_SESSIONS; i++) {
            if (FD_ISSET(sessions[i].master_fd, &read_fds)) {
                nread = read(sessions[i].master_fd, buffer, sizeof(buffer));
                if (nread > 0) {
                    for (int k = 0; k < nread; k++) {
                        parser_feed_char(sessions[i].parser, scr, sessions[i].win, buffer[k]);
                    }
                    needs_render = 1;
                }
            }
        }

        if (needs_render) {
            render_all_sessions(scr, sessions, MAX_SESSIONS);
        }
    }

    // --- Cleanup Memory & FDs ---
    for (int i = 0; i < MAX_SESSIONS; i++) {
        parser_destroy(sessions[i].parser);
        window_destroy(sessions[i].win);
        close(sessions[i].master_fd);
    }
    clipboard_destroy(engine_cb); // <-- 5. Clipboard Memory Cleanup
    screen_destroy(scr);
    printf("\r\nBDH Terminal Engine Exited Cleanly.\n\r");
    return EXIT_SUCCESS;
}
