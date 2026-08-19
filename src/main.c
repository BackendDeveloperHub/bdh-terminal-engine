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

// 🔥 Direct Native Overlay Rendering
static void draw_session_manager_direct(TerminalSession sessions[], char *tab_names[], int active_idx, int scr_cols, int scr_rows) {
    int start_row = scr_rows - 11; // Status bar is at scr_rows - 1, Manager takes 11 rows
    if (start_row < 2) return; 
    
    char out[8192];
    int len = 0;
    
    // Move cursor to start_row
    len += snprintf(out + len, sizeof(out) - len, "\033[%d;1H", start_row);
    
    // Top border
    len += snprintf(out + len, sizeof(out) - len, "\033[1;36m+== [ BDH Active Sessions Manager ] ");
    int title_len = 36;
    for(int i = title_len; i < scr_cols - 1; i++) out[len++] = '=';
    out[len++] = '+';
    len += snprintf(out + len, sizeof(out) - len, "\033[0m\r\n");
    
    // 3 columns layout for 9 rows
    int col_width = (scr_cols - 4) / 3;
    if (col_width < 15) col_width = 15;
    
    int session_idx = 0;
    for (int r = 0; r < 9; r++) {
        len += snprintf(out + len, sizeof(out) - len, "\033[1;36m|\033[0m ");
        int chars_printed_in_row = 1; 
        
        for (int c = 0; c < 3; c++) {
            int text_len = 0;
            if (session_idx < MAX_SESSIONS) {
                char dummy[128];
                if (session_idx == active_idx) {
                    text_len = snprintf(dummy, sizeof(dummy), "[%d: %s (ACTIVE)]", session_idx + 1, tab_names[session_idx]);
                    len += snprintf(out + len, sizeof(out) - len, "\033[1;32m%s\033[0m", dummy);
                } else if (sessions[session_idx].is_alive) {
                    text_len = snprintf(dummy, sizeof(dummy), "%d: %s (RUNNING)", session_idx + 1, tab_names[session_idx]);
                    len += snprintf(out + len, sizeof(out) - len, "\033[1;33m%s\033[0m", dummy);
                } else {
                    text_len = snprintf(dummy, sizeof(dummy), "%d: %s (DEAD)", session_idx + 1, tab_names[session_idx]);
                    len += snprintf(out + len, sizeof(out) - len, "\033[1;31m%s\033[0m", dummy);
                }
                session_idx++;
            }
            
            int padding = col_width - text_len;
            if (padding < 0) padding = 0;
            if (c == 2) {
                int current_col_pos = chars_printed_in_row + text_len;
                padding = (scr_cols - 2) - current_col_pos; 
                if (padding < 0) padding = 0;
            }
            
            for(int p = 0; p < padding; p++) out[len++] = ' ';
            chars_printed_in_row += text_len + padding;
        }
        len += snprintf(out + len, sizeof(out) - len, "\033[1;36m|\033[0m\r\n");
    }
    
    // Bottom border
    len += snprintf(out + len, sizeof(out) - len, "\033[1;36m+");
    for(int i = 1; i < scr_cols - 1; i++) out[len++] = '=';
    out[len++] = '+';
    len += snprintf(out + len, sizeof(out) - len, "\033[0m"); 
    
    write(STDOUT_FILENO, out, len);
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

    int pty_rows = scr_rows - 13; 
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

    screen_clear(scr);
    if (active_idx >= 0 && active_idx < MAX_SESSIONS && sessions[active_idx].win) {
        window_draw(scr, sessions[active_idx].win);
    }
    if (status_bar) statusbar_draw(scr, status_bar, scr_rows - 1);
    if (tab_bar) render_tab_overlay(scr, tab_bar); 
    renderer_draw_all(scr, sessions, MAX_SESSIONS);
    draw_session_manager_direct(sessions, tab_names, active_idx, scr_cols, scr_rows);

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

                        needs_render = 1;
                    }
                    continue; 
                }

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

                if (buffer[0] == 17 && nread == 1) {
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
                        needs_render = 1;
                    }
                }
            }
        }

        if (active_sessions_count == 0) {
            break;
        }

render_check:
        if (needs_render) {
            screen_clear(scr);
            
            if (active_idx >= 0 && active_idx < MAX_SESSIONS && sessions[active_idx].win) {
                window_draw(scr, sessions[active_idx].win);
            }

            if (status_bar) statusbar_draw(scr, status_bar, scr_rows - 1);
            if (tab_bar) render_tab_overlay(scr, tab_bar); 
            
            renderer_draw_all(scr, sessions, MAX_SESSIONS);

            // Draw Manager UI Directly!
            draw_session_manager_direct(sessions, tab_names, active_idx, scr_cols, scr_rows);

            // 🔥 THE ARCHITECT FIX: RESTORE CURSOR POSITION!
            // கர்சரை மீண்டும் கமாண்ட் பிராம்ப்ட்-ல் (Active Terminal) வைக்கிறோம்.
            if (active_idx >= 0 && active_idx < MAX_SESSIONS && sessions[active_idx].win) {
                int cursor_r = sessions[active_idx].win->y + 2 + sessions[active_idx].win->cur_r;
                int cursor_c = sessions[active_idx].win->x + 2 + sessions[active_idx].win->cur_c;
                char cursor_restore[32];
                // \033[%d;%dH -> கர்சரை குறிப்பிட்ட இடத்திற்கு நகர்த்தும் ANSI கோட்
                int cr_len = snprintf(cursor_restore, sizeof(cursor_restore), "\033[%d;%dH", cursor_r, cursor_c);
                write(STDOUT_FILENO, cursor_restore, cr_len);
            }
        }
    }

    sessions_cleanup_all(sessions, tab_bar, status_bar, engine_cb, scr);

    write(STDOUT_FILENO, "\033[?1049l\033[?7h", 13);
    printf("\r\nBDH Pure Linux CLI Multiplexer Exited Cleanly.\r\n");
    return EXIT_SUCCESS;
}
