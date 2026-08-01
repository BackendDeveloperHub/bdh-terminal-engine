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

#define MAX_SESSIONS 2

// ஒவ்வொரு விண்டோவுக்கும் தனித்தனி PTY, Window மற்றும் Parser அமைப்பு
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

// Z-Index அடிப்படையில் பின்னால் இருப்பதை முதலில் வரைந்து, முன்னால் இருப்பதை மேலே வரைதல்
void render_all_sessions(VirtualScreen *scr, TerminalSession sessions[], int count) {
    // 1. முதலில் Z-Index 0 (பின்னால் இருக்கும் விண்டோ) வரையலாம்
    for (int i = 0; i < count; i++) {
        if (sessions[i].win->z_index == 0) {
            window_draw(scr, sessions[i].win);
        }
    }
    // 2. அடுத்து Z-Index 1 (Active விண்டோ - முன்னால் மிதப்பது) வரையலாம்
    for (int i = 0; i < count; i++) {
        if (sessions[i].win->z_index == 1) {
            window_draw(scr, sessions[i].win);
        }
    }

    // 3. திரையில் ரெண்டர் செய்தல்
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
    int active_idx = 0; // ஆரம்பத்தில் 1வது விண்டோ Active

    // --- Session 0 (Primary Window) உருவாக்குதல் ---
    sessions[0].id = 0;
    sessions[0].pid = pty_spawn(shell_argv, &sessions[0].master_fd);
    sessions[0].win = window_create(1, 1, 2, 60, 15, "[ 1: Bash - Primary (ACTIVE) ]", 1);
    sessions[0].parser = parser_create();

    // --- Session 1 (Secondary Window) உருவாக்குதல் ---
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

        // --- 1. Keyboard Input (Stdin) ---
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            nread = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (nread > 0) {
                // Ctrl + Q (ASCII 17) = Engine-ல் இருந்து வெளியேற
                if (buffer[0] == 17) {
                    break;
                }
                // Ctrl + A (ASCII 1) = Window Switch Hotkey!
                else if (buffer[0] == 1) {
                    // பழைய Active விண்டோவை பின்னால் அனுப்பு (z_index = 0)
                    sessions[active_idx].win->z_index = 0;
                    sessions[active_idx].win->is_active = 0;
                    strncpy(sessions[active_idx].win->title, 
                            active_idx == 0 ? "[ 1: Bash - Primary ]" : "[ 2: Bash - Secondary ]", 63);

                    // அடுத்த விண்டோவுக்கு Active-ஐ மாற்று
                    active_idx = 1 - active_idx;

                    // புதிய Active விண்டோவை முன்னால் கொண்டு வா (z_index = 1)
                    sessions[active_idx].win->z_index = 1;
                    sessions[active_idx].win->is_active = 1;
                    strncpy(sessions[active_idx].win->title, 
                            active_idx == 0 ? "[ 1: Bash - Primary (ACTIVE) ]" : "[ 2: Bash - Secondary (ACTIVE) ]", 63);

                    render_all_sessions(scr, sessions, MAX_SESSIONS);
                } 
                // மற்ற எல்லா எழுத்துக்களையும் Active விண்டோவின் Shell-க்கு மட்டும் அனுப்பு
                else {
                    write(sessions[active_idx].master_fd, buffer, nread);
                }
            }
        }

        // --- 2. Shell Output (PTY Master FDs) ---
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
    screen_destroy(scr);
    printf("\r\nBDH Terminal Engine Exited Cleanly.\n\r");
    return EXIT_SUCCESS;
}
