// src/main.c
/*#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <string.h>
#include "engine/pty.h"
#include "engine/screen.h"
#include "ui/panes.h"

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

void render_screen_to_console(VirtualScreen *scr) {
    printf("\033[2J\033[H"); // திரையை க்ளீன் செய்து ரெண்டர் செய்ய
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
    int master_fd;
    char *shell_argv[] = {"/bin/bash", NULL};

    enable_raw_mode();

    // 1. PTY Bash Shell-ஐ Spawn செய்கிறோம் (Phase 1)
    pid_t child_pid = pty_spawn(shell_argv, &master_fd);
    if (child_pid == -1) {
        return EXIT_FAILURE;
    }

    // 2. Virtual Screen மற்றும் மிதக்கும் விண்டோவை உருவாக்குகிறோம் (Phase 3)
    VirtualScreen *scr = screen_create(24, 80);
    FloatingWindow *win = window_create(1, 2, 4, 70, 18, "[ 1: Bash - BDH Floating Terminal ]", 0);
    window_draw(scr, win);
    render_screen_to_console(scr);

    fd_set read_fds;
    char buffer[1024];
    ssize_t nread;

    // 3. Main Event Loop - PTY மற்றும் UI இணைப்பு (Phase 4)
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(master_fd, &read_fds);

        if (select(master_fd + 1, &read_fds, NULL, NULL, NULL) == -1) break;

        // --- Keyboard -> PTY Bash ---
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            nread = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (nread > 0) {
                if (buffer[0] == 3) break; // Ctrl-C அடித்தால் Engine-ல் இருந்து வெளியேற
                write(master_fd, buffer, nread);
            }
        }

        // --- PTY Bash Output -> Floating Window Buffer -> Screen ---
        if (FD_ISSET(master_fd, &read_fds)) {
            nread = read(master_fd, buffer, sizeof(buffer));
            if (nread <= 0) break;

            // Bash அனுப்பும் ஒவ்வொரு எழுத்தையும் விண்டோவுக்குள் எழுதுகிறோம்!
            for (int i = 0; i < nread; i++) {
                window_put_char(scr, win, buffer[i]);
            }

            // திரையில் மாற்றத்தை ரெண்டர் செய்கிறோம்
            render_screen_to_console(scr);
        }
    }

    window_destroy(win);
    screen_destroy(scr);
    close(master_fd);
    return EXIT_SUCCESS;
}*/// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <string.h>
#include "engine/pty.h"
#include "engine/screen.h"
#include "ui/panes.h"
#include "engine/parser.h" // <-- புதிதாகச் சேர்க்கப்பட்டுள்ளது

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

void render_screen_to_console(VirtualScreen *scr) {
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
    int master_fd;
    char *shell_argv[] = {"/bin/bash", NULL};

    enable_raw_mode();

    pid_t child_pid = pty_spawn(shell_argv, &master_fd);
    if (child_pid == -1) {
        return EXIT_FAILURE;
    }

    VirtualScreen *scr = screen_create(24, 80);
    FloatingWindow *win = window_create(1, 2, 4, 70, 18, "[ 1: Bash - BDH Floating Terminal ]", 0);
    AnsiParser *parser = parser_create(); // <-- Parser உருவாக்குதல்

    window_draw(scr, win);
    render_screen_to_console(scr);

    fd_set read_fds;
    char buffer[1024];
    ssize_t nread;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(master_fd, &read_fds);

        if (select(master_fd + 1, &read_fds, NULL, NULL, NULL) == -1) break;

        // --- Keyboard -> PTY Bash ---
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            nread = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (nread > 0) {
                if (buffer[0] == 3) break; // Ctrl-C
                write(master_fd, buffer, nread);
            }
        }

        // --- PTY Bash Output -> ANSI Parser -> Window -> Screen ---
        if (FD_ISSET(master_fd, &read_fds)) {
            nread = read(master_fd, buffer, sizeof(buffer));
            if (nread <= 0) break;

            // ஒவ்வொரு எழுத்தையும் Parser-க்கு அனுப்புகிறோம்!
            for (int i = 0; i < nread; i++) {
                parser_feed_char(parser, scr, win, buffer[i]); // <-- Filtered character feed
            }

            render_screen_to_console(scr);
        }
    }

    parser_destroy(parser); // <-- Parser memory free
    window_destroy(win);
    screen_destroy(scr);
    close(master_fd);
    return EXIT_SUCCESS;
}


