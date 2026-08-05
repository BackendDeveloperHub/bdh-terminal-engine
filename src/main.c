// src/main.c - BDH Pure Linux CLI Multiplexer Engine (50x220 Production Build - 12 Default / 18 Max Cap)
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
#include "ui/wm.h"
#include "ui/tabs.h"
#include "ui/statusbar.h"
#include "engine/parser.h"
#include "engine/clipboard.h"
#include "engine/scanner.h"
#include "engine/cursor.h"
#include "engine/input.h"
#include "engine/renderer.h"
#include "engine/terminal.h"
#include "engine/session.h"

// --- 🔥 SESSION LIMIT CONFIGURATION (12 Default / 18 Maximum Cap) ---
#ifndef MAX_SESSIONS
#define MAX_SESSIONS 18
#endif

#define DEFAULT_SESSIONS 12

static void fatal_signal_handler(int signo) {
    write(STDOUT_FILENO, "\033[?1049l", 8);
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
    (void)argc;
    (void)argv;

    atexit(terminal_disable_raw_mode);
    setup_signal_handlers();

    printf("\r\n[BDH Engine] Starting 50x220 Custom Mode (12 Default / 18 Max Sessions)...\r\n");

    char *user_shell = getenv("SHELL");
    if (!user_shell || strlen(user_shell) == 0) {
        user_shell = "/bin/bash";
    }
    char *shell_argv[] = {user_shell, NULL};

    terminal_enable_raw_mode();
    write(STDOUT_FILENO, "\033[?1049h\033[H", 11);

    struct winsize ws = {0};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    }
    
    // --- FORCE 50 Rows x 220 Columns Layout ---
    int scr_rows = 50;   
    int scr_cols = 220;  

    int pty_rows = scr_rows - 2;
    int pty_cols = scr_cols - 2;

    VirtualScreen *scr = screen_create(scr_rows, scr_cols);
    if (!scr) {
        terminal_disable_raw_mode();
        printf("[Error] Failed to allocate VirtualScreen!\r\n");
        return EXIT_FAILURE;
    }
    
    TerminalSession sessions[MAX_SESSIONS];
    int active_idx = 0;

    Clipboard *engine_cb = clipboard_create();
    const char *default_cmd = "echo BDH 50x220 Multiplexer Active!\n";
    clipboard_set(engine_cb, default_cmd, strlen(default_cmd));

    TokenScanner *token_scanner = scanner_create();
    TabBar *tab_bar = tabs_create();
    StatusBar *status_bar = statusbar_create();
    
    statusbar_set_mode(status_bar, "NORMAL");
    statusbar_set_text(status_bar, "[ BDH Linux Multiplexer ]", "Click/Ctrl+A: Tab | Ctrl+B: Browser | Ctrl+K: Scan");

    // --- DYNAMIC TAB NAMES GENERATOR (Capped at MAX_SESSIONS: 18) ---
    char *tab_names[MAX_SESSIONS];
    char tab_name_buffers[MAX_SESSIONS][32];

    for (int i = 0; i < MAX_SESSIONS; i++) {
        snprintf(tab_name_buffers[i], sizeof(tab_name_buffers[i]), "BASH-%d", i + 1);
        tab_names[i] = tab_name_buffers[i];
    }

    // 12 முதல் 18 செஷன்களை கச்சிతంగా இயக்கும் Session Initializer
    sessions_init_all(sessions, shell_argv, pty_rows, pty_cols, scr_cols, scr_rows, tab_bar, tab_names);

    // Initial Draw (Top Tab Bar commented out for clean Row 0 badge display)
    renderer_draw_all(scr, sessions, MAX_SESSIONS);
    // if (tab_bar) tabs_draw(scr, tab_bar, 0);
    if (status_bar) statusbar_draw(scr, status_bar, scr_rows - 1);
    if (tab_bar) render_tab_overlay(scr, tab_bar); // Permanent Footer Box

    fd_set read_fds;
    char buffer[16384];
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

        struct timeval tv = {0, 10000};
        if (select(max_fd + 1, &read_fds, NULL, NULL, &tv) == -1) {
            if (errno == EINTR) continue;
            break;
        }

        // --- 1. Keyboard & Mouse Input (BULLETPROOF FIXED) ---
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            nread = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (nread > 0) {
                buffer[nread] = '\0'; // கட்டாயம் Null-Terminate செய்ய வேண்டும்!

                // --- SCANNER MODE CHECK ---
                if (token_scanner && token_scanner->is_scanning_mode) {
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
                    if (status_bar) statusbar_set_mode(status_bar, "NORMAL");
                    continue;
                }

                // --- MOUSE INTERCEPTOR ---
                MouseEvent mouse;
                if (strncmp(buffer, "\033[<", 3) == 0 && mouse_parse_sgr(buffer, &mouse)) {
                    if (mouse.row == 0 && mouse.button == MOUSE_BTN_LEFT && !mouse.is_release) {
                        int tab_width = scr_cols / MAX_SESSIONS;
                        if (tab_width < 1) tab_width = 1;

                        int clicked_tab = mouse.col / tab_width; 
                        
                        if (clicked_tab >= 0 && clicked_tab < MAX_SESSIONS && 
                            sessions[clicked_tab].is_alive && sessions[clicked_tab].win != NULL) {
                            
                            sessions[active_idx].win->z_index = 0;
                            sessions[active_idx].win->is_active = 0;
                            snprintf(sessions[active_idx].win->title, sizeof(sessions[active_idx].win->title), 
                                     "[ TAB %d/%d : %s ]", active_idx + 1, MAX_SESSIONS, tab_names[active_idx]);

                            active_idx = clicked_tab;
                            sessions[active_idx].win->z_index = 1;
                            sessions[active_idx].win->is_active = 1;
                            snprintf(sessions[active_idx].win->title, sizeof(sessions[active_idx].win->title), 
                                     "[ TAB %d/%d : %s (ACTIVE) * ]", active_idx + 1, MAX_SESSIONS, tab_names[active_idx]);

                            if (tab_bar) tabs_set_active(tab_bar, active_idx);

                            write(STDOUT_FILENO, "\033[?7l", 5);
                            renderer_draw_all(scr, sessions, MAX_SESSIONS);
                            // if (tab_bar) tabs_draw(scr, tab_bar, 0);
                            if (status_bar) statusbar_draw(scr, status_bar, scr_rows - 1);
                            if (tab_bar) render_tab_overlay(scr, tab_bar);
                            write(STDOUT_FILENO, "\033[?7h", 5);
                        }
                    }
                    continue; 
                }

                // --- SHORTCUT 1: Ctrl+A (Tab Switch) ---
                if (buffer[0] == 1 && nread == 1) { 
                    if (active_idx >= 0 && active_idx < MAX_SESSIONS && sessions[active_idx].win != NULL) {
                        sessions[active_idx].win->z_index = 0;
                        sessions[active_idx].win->is_active = 0;
                        snprintf(sessions[active_idx].win->title, sizeof(sessions[active_idx].win->title), 
                                 "[ TAB %d/%d : %s ]", active_idx + 1, MAX_SESSIONS, tab_names[active_idx]);
                    }

                    int next_idx = (active_idx + 1) % MAX_SESSIONS;
                    int attempts = 0;
                    while (attempts < MAX_SESSIONS && (!sessions[next_idx].is_alive || sessions[next_idx].win == NULL)) {
                        next_idx = (next_idx + 1) % MAX_SESSIONS;
                        attempts++;
                    }

                    if (sessions[next_idx].is_alive && sessions[next_idx].win != NULL) {
                        active_idx = next_idx;
                        sessions[active_idx].win->z_index = 1;
                        sessions[active_idx].win->is_active = 1;
                        snprintf(sessions[active_idx].win->title, sizeof(sessions[active_idx].win->title), 
                                 "[ TAB %d/%d : %s (ACTIVE) * ]", active_idx + 1, MAX_SESSIONS, tab_names[active_idx]);

                        if (tab_bar) tabs_set_active(tab_bar, active_idx);

                        write(STDOUT_FILENO, "\033[?7l", 5);
                        renderer_draw_all(scr, sessions, MAX_SESSIONS);
                        // if (tab_bar) tabs_draw(scr, tab_bar, 0);
                        if (status_bar) statusbar_draw(scr, status_bar, scr_rows - 1);
                        if (tab_bar) render_tab_overlay(scr, tab_bar);
                        write(STDOUT_FILENO, "\033[?7h", 5);
                    }
                    continue; 
                }

                // --- SHORTCUT 2: Ctrl+B (Browser) ---
                if (buffer[0] == 2 && nread == 1) {
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

                // --- SHORTCUT 3: Ctrl+K (Token Scanner) ---
                if (buffer[0] == 11 && nread == 1) { 
                    int count = scanner_scan_screen(token_scanner, scr);
                    if (count > 0) {
                        token_scanner->is_scanning_mode = 1;
                        if (status_bar) statusbar_set_mode(status_bar, "SCANNER");
                        printf("\r\n\033[1;33m[BDH Scanner] Found %d tokens! Press 1-%d to copy, or any other key to cancel:\033[0m\r\n", count, count);
                        for (int t = 0; t < count; t++) {
                            printf("  \033[1;36m[%d]\033[0m %s\r\n", token_scanner->tokens[t].id, token_scanner->tokens[t].text);
                        }
                    } else {
                        printf("\r\n\033[1;31m[BDH Scanner] No URLs, IPs, UUIDs, or Paths found on screen.\033[0m\r\n");
                    }
                    continue;
                }

                // --- SHORTCUT 4: Ctrl+Q (Exit Engine Cleanly) ---
                if (buffer[0] == 17 && nread == 1) {
                    engine_running = 0;
                    break;
                }

                // --- DIRECT PTY PASSTHROUGH ---
                if (sessions[active_idx].is_alive && sessions[active_idx].master_fd >= 0) {
                    write(sessions[active_idx].master_fd, buffer, nread);
                }
            }
        }

        // --- 2. Shell Output & Concurrent Sessions Parsing ---
        int needs_render = 0;
        int active_sessions_count = 0;

        for (int i = 0; i < MAX_SESSIONS; i++) {
            if (!sessions[i].is_alive || sessions[i].master_fd < 0) continue;
            active_sessions_count++;

            if (FD_ISSET(sessions[i].master_fd, &read_fds)) {
                nread = read(sessions[i].master_fd, buffer, sizeof(buffer));
                
                if (nread > 0) {
                    if (sessions[i].win != NULL && sessions[i].parser != NULL) {
                        for (int k = 0; k < nread; k++) {
                            parser_feed_char(sessions[i].parser, scr, sessions[i].win, buffer[k]);
                        }
                        if (i == active_idx) {
                            needs_render = 1;
                        }
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

        // --- RENDER LOOP WITH PERMANENT FOOTER BOX ---
        if (needs_render) {
            write(STDOUT_FILENO, "\033[?7l", 5); 
            renderer_draw_all(scr, sessions, MAX_SESSIONS);
            // if (tab_bar) tabs_draw(scr, tab_bar, 0); // Top Tab Bar disabled
            if (status_bar) statusbar_draw(scr, status_bar, scr_rows - 1);
            if (tab_bar) render_tab_overlay(scr, tab_bar); // Always rendered at footer
            write(STDOUT_FILENO, "\033[?7h", 5); 
        }
    }

    sessions_cleanup_all(sessions, tab_bar, status_bar, token_scanner, engine_cb, scr);

    write(STDOUT_FILENO, "\033[?1049l", 8);
    printf("\r\nBDH Pure Linux CLI Multiplexer Exited Cleanly.\r\n");
    return EXIT_SUCCESS;
}
