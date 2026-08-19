// src/main.c - BDH Pure Linux CLI Multiplexer Engine (12 Default / 18 Max Cap)
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
#include "engine/cursor.h"
#include "engine/input.h"
#include "engine/renderer.h"
#include "engine/terminal.h"
#include "engine/session.h"

#ifndef MAX_SESSIONS
#define MAX_SESSIONS 18
#endif

#define DEFAULT_SESSIONS 12

static void fatal_signal_handler(int signo) {
    write(STDOUT_FILENO, "\033[?1049l\033[?7h", 13);
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

// 🔥 NEW: BDH Active Sessions Manager UI Logic
static void update_session_manager_ui(FloatingWindow *mgr_win, TerminalSession sessions[], char *tab_names[], int active_idx) {
    if (!mgr_win) return;
    int inner_w = mgr_win->width - 2;
    int inner_h = mgr_win->height - 2;
    
    // Clear background
    for (int r = 0; r < inner_h; r++) {
        for (int c = 0; c < inner_w; c++) {
            mgr_win->text[r][c] = ' ';
        }
    }
    
    int r = 0;
    int c = 2; // Left margin
    int col_width = inner_w / 3; // 3 Columns
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        char buf[64];
        if (i == active_idx) {
            snprintf(buf, sizeof(buf), "[%d: %s (ACTIVE)]", i + 1, tab_names[i]);
        } else if (sessions[i].is_alive) {
            snprintf(buf, sizeof(buf), "%d: %s (RUNNING)", i + 1, tab_names[i]);
        } else {
            snprintf(buf, sizeof(buf), "%d: %s (DEAD)", i + 1, tab_names[i]);
        }
        
        int len = strlen(buf);
        for(int k = 0; k < len && (c + k) < inner_w; k++) {
            mgr_win->text[r][c + k] = buf[k];
        }
        
        c += col_width;
        if (c + 15 > inner_w) { 
            r++;
            c = 2;
            if (r >= inner_h) break;
        }
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    atexit(terminal_disable_raw_mode);
    setup_signal_handlers();

    setenv("TERM", "xterm-256color", 1);

    printf("\r\n[BDH Engine] Starting Responsive Custom Mode (Bash Edition - 12 Default / 18 Max Sessions)...\r\n");

    char *user_shell = getenv("SHELL");
    if (!user_shell || strlen(user_shell) == 0) {
        user_shell = "/bin/bash"; 
    }
    char *shell_argv[] = {user_shell, NULL};

    terminal_enable_raw_mode();
    write(STDOUT_FILENO, "\033[?1049h\033[H\033[?7l", 16);

    struct winsize ws = {0};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    }
    
    int scr_rows = (ws.ws_row > 10) ? ws.ws_row : 24;
    int scr_cols = (ws.ws_col > 20) ? ws.ws_col : 80;

    // 🔥 FIX: Terminal Multiplexer-ஐ -12 ஆக குறைத்துள்ளோம்!
    int pty_rows = scr_rows - 12; 
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
    const char *default_cmd = "echo BDH Bash Multiplexer Active!\n";
    clipboard_set(engine_cb, default_cmd, strlen(default_cmd));

    TabBar *tab_bar = tabs_create();
    StatusBar *status_bar = statusbar_create();
    
    // 🔥 NEW: BDH Active Sessions Manager Box (Height: 10, Positioned below Terminal)
    FloatingWindow *session_mgr_win = window_create(99, 0, scr_rows - 11, scr_cols, 10, "[ BDH Active Sessions Manager ]", 0);

    statusbar_set_mode(status_bar, "NORMAL");
    statusbar_set_text(status_bar, "[ BDH Linux Multiplexer ]", "Ctrl+A: Tab | Ctrl+B: Browser");

    char *tab_names[MAX_SESSIONS];
    char tab_name_buffers[MAX_SESSIONS][32];

    for (int i = 0; i < MAX_SESSIONS; i++) {
        snprintf(tab_name_buffers[i], sizeof(tab_name_buffers[i]), "BASH-%d", i + 1);
        tab_names[i] = tab_name_buffers[i];
    }

    sessions_init_all(sessions, shell_argv, pty_rows, pty_cols, scr_cols, scr_rows, tab_bar, tab_names);

    struct winsize ws_pty;
    ws_pty.ws_row = pty_rows;
    ws_pty.ws_col = pty_cols;
    ws_pty.ws_xpixel = 0;
    ws_pty.ws_ypixel = 0;

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].is_alive && sessions[i].master_fd >= 0) {
            ioctl(sessions[i].master_fd, TIOCSWINSZ, &ws_pty);
        }
    }

    // Initial Render
    update_session_manager_ui(session_mgr_win, sessions, tab_names, active_idx);
    screen_clear(scr);
    if (active_idx >= 0 && active_idx < MAX_SESSIONS && sessions[active_idx].win) {
        window_draw(scr, sessions[active_idx].win);
    }
    if (session_mgr_win) window_draw(scr, session_mgr_win); // Render Manager Box
    if (status_bar) statusbar_draw(scr, status_bar, scr_rows - 1);
    if (tab_bar) render_tab_overlay(scr, tab_bar); 
    renderer_draw_all(scr, sessions, MAX_SESSIONS);

    fd_set read_fds;
    char buffer[16384];
    ssize_t nread;
    int max_fd = 0;
    int engine_running = 1;
    int needs_render = 0;

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

        needs_render = 0;

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            nread = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (nread > 0) {
                buffer[nread] = '\0'; 

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
                            needs_render = 1;
                        }
                    }
                    continue; 
                }

                if (buffer[0] == 1 && nread == 1) { // Ctrl+A
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
                        needs_render = 1;
                    }
                    continue; 
                }

                if (buffer[0] == 2 && nread == 1) { // Ctrl+B
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

                if (buffer[0] == 17 && nread == 1) { // Ctrl+Q
                    engine_running = 0;
                    break;
                }

                if (sessions[active_idx].is_alive && sessions[active_idx].master_fd >= 0) {
                    write(sessions[active_idx].master_fd, buffer, nread);
                }
            }
        }

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
                    }
                    needs_render = 1; // Re-render to show session as DEAD in manager
                }
            }
        }

        if (active_sessions_count == 0) {
            break;
        }

render_check:
        if (needs_render) {
            update_session_manager_ui(session_mgr_win, sessions, tab_names, active_idx);
            
            screen_clear(scr);
            
            if (active_idx >= 0 && active_idx < MAX_SESSIONS && sessions[active_idx].win) {
                window_draw(scr, sessions[active_idx].win);
            }

            if (session_mgr_win) window_draw(scr, session_mgr_win); // Render Manager Box
            if (status_bar) statusbar_draw(scr, status_bar, scr_rows - 1);
            if (tab_bar) render_tab_overlay(scr, tab_bar); 
            
            renderer_draw_all(scr, sessions, MAX_SESSIONS);
        }
    }

    if (session_mgr_win) window_destroy(session_mgr_win);
    sessions_cleanup_all(sessions, tab_bar, status_bar, engine_cb, scr);

    write(STDOUT_FILENO, "\033[?1049l\033[?7h", 13);
    printf("\r\nBDH Pure Linux CLI Multiplexer Exited Cleanly.\r\n");
    return EXIT_SUCCESS;
}
