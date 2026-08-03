// src/main.c - BDH Pure Linux CLI Multiplexer Engine (100% Robust 6-Tab Production Build)
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
#include "ui/wm.h"             // UI: Window Manager Module
#include "ui/tabs.h"           // UI: Top Tab Bar Module
#include "ui/statusbar.h"      // UI: Bottom Status Bar Module
#include "engine/parser.h"
#include "engine/clipboard.h"
#include "engine/scanner.h"    // Smart Token Scanner (Ctrl+K)
#include "engine/cursor.h"
#include "engine/input.h"
#include "engine/renderer.h"
#include "engine/terminal.h"

// --- 6 Concurrent Bash Sessions ---
#define MAX_SESSIONS 6

typedef struct {
    int id;
    int master_fd;
    pid_t pid;
    FloatingWindow *win;
    AnsiParser *parser;
    int is_alive;
} TerminalSession;

static void fatal_signal_handler(int signo) {
    write(STDOUT_FILENO, "\033[?1049l", 8); // Restore Normal Terminal
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

    sigaction(SIGSEGV, &sa, NULL); // Signal 11 (Segfault Protection)
    sigaction(SIGTERM, &sa, NULL); // Kill
    sigaction(SIGINT,  &sa, NULL); // Ctrl + C
    sigaction(SIGABRT, &sa, NULL); // Abort
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    atexit(terminal_disable_raw_mode);
    setup_signal_handlers();

    printf("\r\n[BDH Engine] Starting 100%% Pure CLI Mode (6-Tab Multiplexer + Full UI)...\r\n");

    char *user_shell = getenv("SHELL");
    if (!user_shell || strlen(user_shell) == 0) {
        user_shell = "/bin/bash";
    }
    char *shell_argv[] = {user_shell, NULL};

    terminal_enable_raw_mode();

    // Alternate Screen Buffer-ஐ ஆன் செய்கிறோம் (tmux/vim style):
    write(STDOUT_FILENO, "\033[?1049h\033[H", 11);

    struct winsize ws = {0};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    }
    
    int scr_rows = (ws.ws_row > 0) ? ws.ws_row : 38;
    int scr_cols = (ws.ws_col > 0) ? ws.ws_col : 135;

    int pty_rows = scr_rows - 2;
    int pty_cols = scr_cols - 2;

    VirtualScreen *scr = screen_create(scr_rows, scr_cols);
    
    TerminalSession sessions[MAX_SESSIONS];
    memset(sessions, 0, sizeof(sessions)); // Stack Memory Garbage Clearing
    
    int active_idx = 0;

    Clipboard *engine_cb = clipboard_create();
    const char *default_cmd = "echo BDH CLI 6-Tab Multiplexer Active!\n";
    clipboard_set(engine_cb, default_cmd, strlen(default_cmd));

    TokenScanner *token_scanner = scanner_create();

    TabBar *tab_bar = tabs_create();
    StatusBar *status_bar = statusbar_create();
    statusbar_set_mode(status_bar, "NORMAL");
    statusbar_set_text(status_bar, "[ BDH Linux Multiplexer ]", "Ctrl+A: Tab | Ctrl+B: Browser | Ctrl+K: Scan");

    char *tab_names[MAX_SESSIONS] = {
        "BASH-1", "BASH-2", "BASH-3", "BASH-4", "BASH-5", "BASH-6"
    };

    // --- 6 Sessions & Tabs Safe Creation (100% Robust Loop with Fallback) ---
    for (int i = 0; i < MAX_SESSIONS; i++) {
        sessions[i].id = i;
        sessions[i].master_fd = -1;
        sessions[i].is_alive = 0;
        sessions[i].win = NULL;
        sessions[i].parser = NULL;

        sessions[i].pid = pty_spawn(shell_argv, &sessions[i].master_fd, pty_rows, pty_cols);
        
        if (sessions[i].pid < 0 || sessions[i].master_fd < 0) {
            // Fallback to standard bash if custom shell path fails
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
            tabs_add(tab_bar, tab_names[i]);
        }
    }

    renderer_draw_all(scr, sessions, MAX_SESSIONS);
    if (tab_bar) tabs_draw(scr, tab_bar, 0);
    if (status_bar) statusbar_draw(scr, status_bar, scr_rows - 1);

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
            if (sessions[i].is_alive && sessions[i].master_fd >= 0) {
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
                    statusbar_set_mode(status_bar, "NORMAL");
                    continue;
                }

                // =================================================================
                // --- SAFE INTERCEPTOR 1: Ctrl+A (ASCII 1) -> TAB SWITCH ---
                // =================================================================
                if (buffer[0] == 1) { 
                    if (active_idx >= 0 && active_idx < MAX_SESSIONS && sessions[active_idx].win != NULL) {
                        sessions[active_idx].win->z_index = 0;
                        sessions[active_idx].win->is_active = 0;
                        snprintf(sessions[active_idx].win->title, 63, "[ TAB %d/%d : %s ]", 
                                 active_idx + 1, MAX_SESSIONS, tab_names[active_idx]);
                    }

                    int next_idx = (active_idx + 1) % MAX_SESSIONS;
                    int attempts = 0;
                    while (attempts < MAX_SESSIONS && (!sessions[next_idx].is_alive || sessions[next_idx].win == NULL)) {
                        next_idx = (next_idx + 1) % MAX_SESSIONS;
                        attempts++;
                    }
                    active_idx = next_idx;

                    if (active_idx >= 0 && active_idx < MAX_SESSIONS && sessions[active_idx].win != NULL) {
                        sessions[active_idx].win->z_index = 1;
                        sessions[active_idx].win->is_active = 1;
                        snprintf(sessions[active_idx].win->title, 63, "[ TAB %d/%d : %s (ACTIVE) * ]", 
                                 active_idx + 1, MAX_SESSIONS, tab_names[active_idx]);
                    }

                    if (tab_bar) tabs_set_active(tab_bar, active_idx);

                    write(STDOUT_FILENO, "\033[?7l", 5);
                    renderer_draw_all(scr, sessions, MAX_SESSIONS);
                    if (tab_bar) tabs_draw(scr, tab_bar, 0);
                    if (status_bar) statusbar_draw(scr, status_bar, scr_rows - 1);
                    write(STDOUT_FILENO, "\033[?7h", 5);
                    continue; 
                }

                // =================================================================
                // --- SAFE INTERCEPTOR 2: Ctrl+B (ASCII 2) -> OPEN BROWSER ---
                // =================================================================
                if (buffer[0] == 2) {
                    const char *target_url = getenv("BDH_URL");
                    if (!target_url || strlen(target_url) == 0) {
                        target_url = "https://github.com/BackendDeveloperHub";
                    }
                    
                    char browser_cmd[512];
                    snprintf(browser_cmd, sizeof(browser_cmd), "links %s\n", target_url);
                    
                    if (sessions[active_idx].is_alive && sessions[active_idx].master_fd >= 0) {
                        write(sessions[active_idx].master_fd, browser_cmd, strlen(browser_cmd));
                    }
                    continue;
                }

                // =================================================================
                // --- SAFE INTERCEPTOR 3: Ctrl+K (ASCII 11) -> TOKEN SCANNER ---
                // =================================================================
                if (buffer[0] == 11) { 
                    int count = scanner_scan_screen(token_scanner, scr);
                    if (count > 0) {
                        token_scanner->is_scanning_mode = 1;
                        statusbar_set_mode(status_bar, "SCANNER");
                        printf("\r\n\033[1;33m[BDH Scanner] Found %d tokens! Press 1-%d to copy, or any other key to cancel:\033[0m\r\n", count, count);
                        for (int t = 0; t < count; t++) {
                            printf("  \033[1;36m[%d]\033[0m %s\r\n", token_scanner->tokens[t].id, token_scanner->tokens[t].text);
                        }
                    } else {
                        printf("\r\n\033[1;31m[BDH Scanner] No URLs, IPs, UUIDs, or Paths found on screen.\033[0m\r\n");
                    }
                    continue;
                }

                // --- Normal Keyboard Input ---
                InputAction action = input_parse_key(buffer[0], engine_cb, "ls -la /home\n");

                switch (action) {
                    case INPUT_ACTION_EXIT:
                        engine_running = 0;
                        break;

                    case INPUT_ACTION_PASTE: {
                        const char *paste_data = clipboard_get(engine_cb);
                        if (strlen(paste_data) > 0 && sessions[active_idx].is_alive && sessions[active_idx].master_fd >= 0) {
                            write(sessions[active_idx].master_fd, paste_data, strlen(paste_data));
                        }
                        break;
                    }

                    case INPUT_ACTION_NORMAL:
                    default:
                        if (sessions[active_idx].is_alive && sessions[active_idx].master_fd >= 0) {
                            write(sessions[active_idx].master_fd, buffer, nread);
                        }
                        break;
                }
            }
        }

        // --- 2. Shell Output & Safe Background Filtering ---
        int needs_render = 0;
        int active_sessions_count = 0;

        for (int i = 0; i < MAX_SESSIONS; i++) {
            if (!sessions[i].is_alive || sessions[i].master_fd < 0) continue;
            active_sessions_count++;

            if (FD_ISSET(sessions[i].master_fd, &read_fds)) {
                nread = read(sessions[i].master_fd, buffer, sizeof(buffer));
                
                if (nread > 0) {
                    if (i == active_idx && sessions[i].win && sessions[i].win->is_active && sessions[i].parser) {
                        for (int k = 0; k < nread; k++) {
                            parser_feed_char(sessions[i].parser, scr, sessions[i].win, buffer[k]);
                        }
                        needs_render = 1;
                    }
                } 
                else if (nread == 0 || errno == EIO) {
                    int status;
                    waitpid(sessions[i].pid, &status, WNOHANG);
                    
                    sessions[i].is_alive = 0;
                    close(sessions[i].master_fd);
                    sessions[i].master_fd = -1;
                    if (sessions[i].win) {
                        sessions[i].win->is_active = 0;
                    }
                    
                    if (i == active_idx && sessions[i].parser && sessions[i].win) {
                        char exit_msg[] = "\r\n[Session Exited - Switch tab to continue]\r\n";
                        for (size_t k = 0; k < strlen(exit_msg); k++) {
                            parser_feed_char(sessions[i].parser, scr, sessions[i].win, exit_msg[k]);
                        }
                        needs_render = 1;
                    }
                }
            }
        }

        if (active_sessions_count == 0) {
            break;
        }

        // --- UI Rendering Block ---
        if (needs_render) {
            write(STDOUT_FILENO, "\033[?7l", 5); 
            renderer_draw_all(scr, sessions, MAX_SESSIONS);
            if (tab_bar) tabs_draw(scr, tab_bar, 0);
            if (status_bar) statusbar_draw(scr, status_bar, scr_rows - 1);
            write(STDOUT_FILENO, "\033[?7h", 5); 
        }
    }

    // --- Clean Memory & Deallocation ---
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

    write(STDOUT_FILENO, "\033[?1049l", 8);

    printf("\r\nBDH Pure Linux CLI Multiplexer Exited Cleanly.\r\n");
    return EXIT_SUCCESS;
}
