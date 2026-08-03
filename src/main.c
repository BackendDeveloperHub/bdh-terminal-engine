// src/main.c - BDH Pure Linux CLI Multiplexer Engine (Production-Ready + Token Scanner)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "engine/pty.h"
#include "engine/screen.h"
#include "ui/panes.h"
#include "engine/parser.h"
#include "engine/clipboard.h"
#include "engine/scanner.h"    // <-- NEW: Token Scanner Module
#include "engine/cursor.h"
#include "engine/input.h"
#include "engine/renderer.h"
#include "engine/terminal.h"

#define MAX_SESSIONS 2

typedef struct {
    int id;
    int master_fd;
    pid_t pid;
    FloatingWindow *win;
    AnsiParser *parser;
    int is_alive;
} TerminalSession;

static void fatal_signal_handler(int signo) {
    terminal_disable_raw_mode();
    char msg[128];
    int len = snprintf(msg, sizeof(msg), "\r\n[BDH Engine] Fatal error: Caught signal %d. Terminal state restored cleanly.\r\n", signo);
    write(STDOUT_FILENO, msg, len);
    _exit(EXIT_FAILURE);
}

static void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = fatal_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
}

int main(int argc, char *argv[]) {
    atexit(terminal_disable_raw_mode);
    setup_signal_handlers();

    printf("\r\n[BDH Engine] Starting 100%% Pure CLI Mode (With Smart Token Scanner)...\r\n");

    char *shell_argv[] = {"/bin/bash", NULL};
    terminal_enable_raw_mode();

    struct winsize ws = {0};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    }
    
    int scr_rows = (ws.ws_row > 0) ? ws.ws_row : 38;
    int scr_cols = (ws.ws_col > 0) ? ws.ws_col : 135;

    int pty_rows = scr_rows - 2;
    int pty_cols = scr_cols - 2;

    printf("\r\n[BDH Engine] Detected Screen Size: %d cols x %d rows (PTY Inner: %d x %d)\r\n", 
           scr_cols, scr_rows, pty_cols, pty_rows);

    VirtualScreen *scr = screen_create(scr_rows, scr_cols);
    TerminalSession sessions[MAX_SESSIONS];
    int active_idx = 0;

    Clipboard *engine_cb = clipboard_create();
    const char *default_cmd = "echo BDH CLI Multiplexer Active!\n";
    clipboard_set(engine_cb, default_cmd, strlen(default_cmd));

    // --- NEW: Token Scanner-ஐ உருவாக்குகிறோம் ---
    TokenScanner *token_scanner = scanner_create();

    // --- Session 0 (Primary Window) ---
    sessions[0].id = 0;
    sessions[0].pid = pty_spawn(shell_argv, &sessions[0].master_fd, pty_rows, pty_cols);
    sessions[0].win = window_create(0, 0, 0, scr_cols, scr_rows, "[ TAB 1/2 : PRIMARY BASH (ACTIVE) * ]", 1);
    sessions[0].parser = parser_create();
    sessions[0].is_alive = 1;

    // --- Session 1 (Secondary Window) ---
    sessions[1].id = 1;
    sessions[1].pid = pty_spawn(shell_argv, &sessions[1].master_fd, pty_rows, pty_cols);
    sessions[1].win = window_create(1, 0, 0, scr_cols, scr_rows, "[ TAB 2/2 : SECONDARY BASH ]", 0);
    sessions[1].parser = parser_create();
    sessions[1].is_alive = 1;

    renderer_draw_all(scr, sessions, MAX_SESSIONS);

    fd_set read_fds;
    char buffer[16384]; // 16 KB Anti-Glitch Buffer
    ssize_t nread;
    int max_fd = 0;
    int engine_running = 1;

    while (engine_running) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        max_fd = STDIN_FILENO;

        for (int i = 0; i < MAX_SESSIONS; i++) {
            if (sessions[i].is_alive) {
                FD_SET(sessions[i].master_fd, &read_fds);
                if (sessions[i].master_fd > max_fd) {
                    max_fd = sessions[i].master_fd;
                }
            }
        }

        struct timeval tv = {0, 10000}; // 10ms
        if (select(max_fd + 1, &read_fds, NULL, NULL, &tv) == -1) {
            if (errno == EINTR) continue;
            break;
        }

        // --- 1. Keyboard Input ---
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            nread = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (nread > 0) {
                
                // =========================================================
                // --- NEW FEATURE 1: Scanning Mode Active (Number Copy) ---
                // =========================================================
                if (token_scanner->is_scanning_mode) {
                    if (buffer[0] >= '1' && buffer[0] <= '9') {
                        int token_id = buffer[0] - '0';
                        if (scanner_copy_by_id(token_scanner, token_id, engine_cb)) {
                            printf("\r\n\033[1;32m[BDH Scanner] Token [%d] copied to BDH & Host OS Clipboard! 🚀\033[0m\r\n", token_id);
                        } else {
                            printf("\r\n\033[1;31m[BDH Scanner] Invalid Token ID [%d]\033[0m\r\n", token_id);
                        }
                    } else {
                        printf("\r\n[BDH Scanner] Scan cancelled.\r\n");
                    }
                    token_scanner->is_scanning_mode = 0;
                    continue; // ஷெல்லுக்கு இந்த கீயை அனுப்பாமல் தடுக்கிறோம்
                }

                // =========================================================
                // --- NEW FEATURE 2: Trigger Token Scanner (Ctrl + K) ---
                // =========================================================
                if (buffer[0] == 11) { // Ctrl + K in ASCII is 11 (0x0B)
                    int count = scanner_scan_screen(token_scanner, scr);
                    if (count > 0) {
                        token_scanner->is_scanning_mode = 1;
                        printf("\r\n\033[1;33m[BDH Scanner] Found %d tokens! Press 1-%d to copy, or any other key to cancel:\033[0m\r\n", count, count);
                        for (int t = 0; t < count; t++) {
                            printf("  \033[1;36m[%d]\033[0m %s\r\n", token_scanner->tokens[t].id, token_scanner->tokens[t].text);
                        }
                    } else {
                        printf("\r\n\033[1;31m[BDH Scanner] No URLs, IPs, UUIDs, or Paths found on screen.\033[0m\r\n");
                    }
                    continue;
                }

                // --- Normal Keyboard Input Logic ---
                InputAction action = input_parse_key(buffer[0], engine_cb, "ls -la /home\n");

                switch (action) {
                    case INPUT_ACTION_EXIT:
                        engine_running = 0;
                        break;

                    case INPUT_ACTION_SWITCH_WIN:
                        sessions[active_idx].win->z_index = 0;
                        sessions[active_idx].win->is_active = 0;
                        strncpy(sessions[active_idx].win->title, 
                                active_idx == 0 ? "[ TAB 1/2 : PRIMARY BASH ]" : "[ TAB 2/2 : SECONDARY BASH ]", 63);

                        active_idx = 1 - active_idx;

                        sessions[active_idx].win->z_index = 1;
                        sessions[active_idx].win->is_active = 1;
                        strncpy(sessions[active_idx].win->title, 
                                active_idx == 0 ? "[ TAB 1/2 : PRIMARY BASH (ACTIVE) * ]" : "[ TAB 2/2 : SECONDARY BASH (ACTIVE) * ]", 63);

                        renderer_draw_all(scr, sessions, MAX_SESSIONS);
                        break;

                    case INPUT_ACTION_OPEN_BROWSER: {
                        const char *target_url = getenv("BDH_URL");
                        if (!target_url || strlen(target_url) == 0) {
                            target_url = "https://github.com/BackendDeveloperHub";
                        }
                        
                        char browser_cmd[512];
                        snprintf(browser_cmd, sizeof(browser_cmd), "links %s\n", target_url);
                        
                        if (sessions[active_idx].is_alive) {
                            write(sessions[active_idx].master_fd, browser_cmd, strlen(browser_cmd));
                        }
                        break;
                    }

                    case INPUT_ACTION_PASTE: {
                        const char *paste_data = clipboard_get(engine_cb);
                        if (strlen(paste_data) > 0 && sessions[active_idx].is_alive) {
                            write(sessions[active_idx].master_fd, paste_data, strlen(paste_data));
                        }
                        break;
                    }

                    case INPUT_ACTION_COPY:
                        break;

                    case INPUT_ACTION_NORMAL:
                    default:
                        if (sessions[active_idx].is_alive) {
                            write(sessions[active_idx].master_fd, buffer, nread);
                        }
                        break;
                }
            }
        }

        // --- 2. Shell Output & Zombie Prevention ---
        int needs_render = 0;
        int active_sessions_count = 0;

        for (int i = 0; i < MAX_SESSIONS; i++) {
            if (!sessions[i].is_alive) continue;
            active_sessions_count++;

            if (FD_ISSET(sessions[i].master_fd, &read_fds)) {
                nread = read(sessions[i].master_fd, buffer, sizeof(buffer));
                
                if (nread > 0) {
                    for (int k = 0; k < nread; k++) {
                        parser_feed_char(sessions[i].parser, scr, sessions[i].win, buffer[k]);
                    }
                    needs_render = 1;
                } 
                else if (nread == 0 || errno == EIO) {
                    int status;
                    waitpid(sessions[i].pid, &status, WNOHANG);
                    
                    sessions[i].is_alive = 0;
                    close(sessions[i].master_fd);
                    sessions[i].win->is_active = 0;
                    
                    char exit_msg[] = "\r\n[Session Exited - Press Ctrl+B to switch or exit engine]\r\n";
                    for (size_t k = 0; k < strlen(exit_msg); k++) {
                        parser_feed_char(sessions[i].parser, scr, sessions[i].win, exit_msg[k]);
                    }
                    needs_render = 1;
                }
            }
        }

        if (active_sessions_count == 0) {
            break;
        }

        if (needs_render) {
            renderer_draw_all(scr, sessions, MAX_SESSIONS);
        }
    }

    // --- Clean Memory & Deallocation ---
    for (int i = 0; i < MAX_SESSIONS; i++) {
        parser_destroy(sessions[i].parser);
        window_destroy(sessions[i].win);
        if (sessions[i].is_alive) {
            close(sessions[i].master_fd);
        }
    }
    
    scanner_destroy(token_scanner); // <-- NEW: Scanner Cleanup
    clipboard_destroy(engine_cb);
    screen_destroy(scr);
    printf("\r\nBDH Pure Linux CLI Multiplexer Exited Cleanly.\r\n");
    return EXIT_SUCCESS;
}
