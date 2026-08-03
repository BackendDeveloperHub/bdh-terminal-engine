// src/main.c - BDH Pure Linux CLI Multiplexer Engine (Production-Ready)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>      // <-- Fix 1: Signal Handling
#include <sys/wait.h>    // <-- Fix 2: waitpid for Zombies
#include <errno.h>
#include "engine/pty.h"
#include "engine/screen.h"
#include "ui/panes.h"
#include "engine/parser.h"
#include "engine/clipboard.h"
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
    int is_alive;        // <-- Session உசுரோட இருக்கானு பார்க்க Flag
} TerminalSession;

// --- FIX 1: Fatal Signal Handler (Raw Mode Crash Protection) ---
static void fatal_signal_handler(int signo) {
    // கிராஷ் ஆனாலும் யூசரின் லினக்ஸ் டெர்மினலை Canonical Mode-க்கு மாற்றிவிடுவோம்!
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

    sigaction(SIGSEGV, &sa, NULL); // Segfault
    sigaction(SIGTERM, &sa, NULL); // Kill
    sigaction(SIGINT,  &sa, NULL); // Ctrl + C
    sigaction(SIGABRT, &sa, NULL); // Abort
}

int main(int argc, char *argv[]) {
    // --- FIX 1: புரோகிராம் எப்படி வெளியேறினாலும் Canonical Mode-க்கு திரும்ப atexit ---
    atexit(terminal_disable_raw_mode);
    setup_signal_handlers();

    printf("\r\n[BDH Engine] Starting 100%% Pure CLI Mode (Production-Ready Multiplexer)...\r\n");

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
            if (errno == EINTR) continue; // சிக்னல் குறுக்கீடுகளைத் தாண்டி ஓட
            break;
        }

        // --- 1. Keyboard Input ---
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            nread = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (nread > 0) {
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

                    // --- FIX 2: Configurable URL via Environment Variable ---
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

        // --- 2. Shell Output & FIX 3: Zombie Process / Hang Prevention ---
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
                // PTY Close ஆனாலோ அல்லது Shell Exit ஆனாலோ (EOF / EIO):
                else if (nread == 0 || errno == EIO) {
                    int status;
                    waitpid(sessions[i].pid, &status, WNOHANG); // Zombie ஆகாமல் தடுக்கிறோம்!
                    
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

        // எல்லா Bash செஷன்களும் Exit ஆகிவிட்டால் என்ஜினை நிறுத்துகிறோம்:
        if (active_sessions_count == 0) {
            break;
        }

        if (needs_render) {
            renderer_draw_all(scr, sessions, MAX_SESSIONS);
        }
    }

    // --- Clean Memory & FD Deallocation ---
    for (int i = 0; i < MAX_SESSIONS; i++) {
        parser_destroy(sessions[i].parser);
        window_destroy(sessions[i].win);
        if (sessions[i].is_alive) {
            close(sessions[i].master_fd);
        }
    }
    
    clipboard_destroy(engine_cb);
    screen_destroy(scr);
    printf("\r\nBDH Pure Linux CLI Multiplexer Exited Cleanly.\r\n");
    return EXIT_SUCCESS;
}
