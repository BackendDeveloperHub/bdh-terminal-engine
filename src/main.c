// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <string.h>
#include "engine/pty.h"
#include "engine/screen.h"
#include "ui/panes.h"
#include "engine/parser.h"
#include "engine/clipboard.h"
#include "engine/cursor.h"
#include "engine/input.h"
#include "engine/renderer.h"
#include "engine/terminal.h"
#include "engine/browser.h" // <-- 1. Browser Header சேர்க்கப்பட்டுள்ளது!

#define MAX_SESSIONS 2

typedef struct {
    int id;
    int master_fd;
    pid_t pid;
    FloatingWindow *win;
    AnsiParser *parser;
} TerminalSession;

int main() {
    char *shell_argv[] = {"/bin/bash", NULL};
    
    // Terminal Module மூலம் Raw Mode ஆன் செய்யப்படுகிறது:
    terminal_enable_raw_mode();

    // 1. லேப்டாப் / Termux டெர்மினலின் உண்மையான முழு அளவை (Full Screen Size) எடுப்பது:
    struct winsize ws = {0};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    }
    
    int scr_rows = (ws.ws_row > 0) ? ws.ws_row : 38;
    int scr_cols = (ws.ws_col > 0) ? ws.ws_col : 135;

    // 2. பார்டர்கள் (2) போக உள்ளே இருக்கும் அசல் PTY அளவு (Full Width Inner Size):
    int pty_rows = scr_rows - 2;
    int pty_cols = scr_cols - 2;

    // ஸ்கிரீன் சைஸ் என்ன கண்டுபிடிக்கப்பட்டது என்பதைத் தெரிந்துகொள்ள Debug Print:
    printf("\r\n[BDH Engine] Detected Screen Size: %d cols x %d rows (PTY Inner: %d x %d)\r\n", 
           scr_cols, scr_rows, pty_cols, pty_rows);

    // 3. முழு ஸ்கிரீன் அளவுக்கு VirtualScreen உருவாக்குதல்:
    VirtualScreen *scr = screen_create(scr_rows, scr_cols);
    TerminalSession sessions[MAX_SESSIONS];
    int active_idx = 0;

    // --- Browser UI Tracker ---
    BrowserWindow *my_browser = NULL; // <-- 2. பிரவுசருக்கான Pointer

    // --- Engine Clipboard ---
    Clipboard *engine_cb = clipboard_create();
    const char *default_cmd = "echo BDH Clipboard Success!\n";
    clipboard_set(engine_cb, default_cmd, strlen(default_cmd));

    // 4. எல்லா டேப்களையும் முழு ஸ்கிரீன் அளவில் (x=0, y=0, scr_cols, scr_rows) உருவாக்குதல்!
    // --- Session 0 (Primary Window) ---
    sessions[0].id = 0;
    sessions[0].pid = pty_spawn(shell_argv, &sessions[0].master_fd, pty_rows, pty_cols);
    // இடது மேல் மூலையில் தெளிவாகத் தெரியும்படி புதிய Tab டைட்டில்:
    sessions[0].win = window_create(0, 0, 0, scr_cols, scr_rows, "[ TAB 1/2 : PRIMARY BASH (ACTIVE) * ]", 1);
    sessions[0].parser = parser_create();

    // --- Session 1 (Secondary Window) ---
    sessions[1].id = 1;
    sessions[1].pid = pty_spawn(shell_argv, &sessions[1].master_fd, pty_rows, pty_cols);
    sessions[1].win = window_create(1, 0, 0, scr_cols, scr_rows, "[ TAB 2/2 : SECONDARY BASH ]", 0);
    sessions[1].parser = parser_create();

    // Renderer Module மூலம் விண்டோக்கள் வரையப்படுகின்றன:
    renderer_draw_all(scr, sessions, MAX_SESSIONS);

    fd_set read_fds;
    char buffer[1024];
    ssize_t nread;
    int max_fd = 0;
    int engine_running = 1;

    while (engine_running) {
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

        // --- 1. Keyboard Input -> Input Module ---
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            nread = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (nread > 0) {
                InputAction action = input_parse_key(buffer[0], engine_cb, "ls -la /home\n");

                switch (action) {
                    case INPUT_ACTION_EXIT:
                        engine_running = 0;
                        break;

                    case INPUT_ACTION_SWITCH_WIN:
                        // பழைய Active விண்டோவை Inactive ஆக மாற்றுகிறோம்:
                        sessions[active_idx].win->z_index = 0;
                        sessions[active_idx].win->is_active = 0;
                        strncpy(sessions[active_idx].win->title, 
                                active_idx == 0 ? "[ TAB 1/2 : PRIMARY BASH ]" : "[ TAB 2/2 : SECONDARY BASH ]", 63);

                        active_idx = 1 - active_idx;

                        // புதிய விண்டோவை Active ஆக மாற்றுகிறோம்:
                        sessions[active_idx].win->z_index = 1;
                        sessions[active_idx].win->is_active = 1;
                        strncpy(sessions[active_idx].win->title, 
                                active_idx == 0 ? "[ TAB 1/2 : PRIMARY BASH (ACTIVE) * ]" : "[ TAB 2/2 : SECONDARY BASH (ACTIVE) * ]", 63);

                        renderer_draw_all(scr, sessions, MAX_SESSIONS);
                        break;

                    // --- 3. புதிய Browser Integration Handling ---
                    case INPUT_ACTION_OPEN_BROWSER:
                        if (my_browser == NULL) {
                            my_browser = browser_create("BDH GUI Browser", 1200, 800);
                            browser_load_url(my_browser, "https://github.com/BackendDeveloperHub");
                            gtk_widget_show_all(my_browser->window);
                        } else {
                            // ஏற்கனவே பிரவுசர் திறந்திருந்தால் அதை முன்னால் கொண்டு வருதல் (Focus)
                            gtk_window_present(GTK_WINDOW(my_browser->window));
                        }
                        break;

                    case INPUT_ACTION_PASTE: {
                        const char *paste_data = clipboard_get(engine_cb);
                        if (strlen(paste_data) > 0) {
                            write(sessions[active_idx].master_fd, paste_data, strlen(paste_data));
                        }
                        break;
                    }

                    case INPUT_ACTION_COPY:
                        break;

                    case INPUT_ACTION_NORMAL:
                    default:
                        write(sessions[active_idx].master_fd, buffer, nread);
                        break;
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
            renderer_draw_all(scr, sessions, MAX_SESSIONS);
        }
    }

    // --- Cleanup Memory & FDs ---
    for (int i = 0; i < MAX_SESSIONS; i++) {
        parser_destroy(sessions[i].parser);
        window_destroy(sessions[i].win);
        close(sessions[i].master_fd);
    }
    
    // 4. பிரவுசர் மெமரியை சுத்தம் செய்தல்:
    if (my_browser != NULL) {
        browser_destroy(my_browser);
    }
    
    clipboard_destroy(engine_cb);
    screen_destroy(scr);
    printf("\r\nBDH Terminal Engine Exited Cleanly.\n\r");
    return EXIT_SUCCESS;
}
