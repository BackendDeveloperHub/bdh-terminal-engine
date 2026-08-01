/*// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>
#include "engine/pty.h"

struct termios orig_termios; // Parent terminal's original settings

// RAW mode-ஐ நீக்க
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// RAW mode-ஐ எனேபிள் பண்ண (Forward keys to shell)
void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode); // ensure restore on exit
    struct termios raw = orig_termios;
    cfmakeraw(&raw); // sets raw settings
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(int argc, char *argv[]) {
    printf("BDH Terminal Engine Phase 1 Starting...\n");
    printf("Press Ctrl-D (in blank line) to exit shell, or Ctrl-C (twice) to exit engine.\n\n\r");

    int master_fd;
    char *shell_argv[] = {"/bin/bash", NULL}; // default shell

    enable_raw_mode(); // switch parent TTY to RAW mode

    // 1. Spawn PTY and Shell
    pid_t child_pid = pty_spawn(shell_argv, &master_fd);
    if (child_pid == -1) {
        fprintf(stderr, "[Error] PTY spawn failed.\n");
        return EXIT_FAILURE;
    }

    // 2. I/O Loop using select() (Non-blocking multiplexing)
    fd_set read_fds;
    char buffer[1024];
    ssize_t nread;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds); // user input (keyboard)
        FD_SET(master_fd, &read_fds);     // shell output (pty master)

        if (select(master_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("[Error] select failed");
            break;
        }

        // --- Keyboard (Stdin) -> Shell (PTY Master) ---
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            nread = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (nread > 0) {
                // simple check for engine exit (Ctrl-C Ctrl-C)
                if (buffer[0] == 3) {
                    static int ctrl_c_count = 0;
                    ctrl_c_count++;
                    if (ctrl_c_count >= 2) break;
                    printf("\r\n[BDH Engine] Ctrl-C twice to exit engine.\n\r");
                } else {
                    // Send keyboard bytes to PTY Master (-> Shell Stdin)
                    write(master_fd, buffer, nread);
                }
            }
        }

        // --- Shell (PTY Master) -> Screen (Stdout) ---
        if (FD_ISSET(master_fd, &read_fds)) {
            nread = read(master_fd, buffer, sizeof(buffer));
            if (nread <= 0) break; // shell exited
            // Write shell output to Parent TTY Stdout (-> user screen)
            write(STDOUT_FILENO, buffer, nread);
        }
    }

    printf("\r\nBDH Terminal Engine Exiting.\n");
    close(master_fd);
    return EXIT_SUCCESS;
}

*/

// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine/screen.h"

// Virtual Screen-ல் ஒரு விண்டோ பார்டர் மற்றும் டெக்ஸ்ட் எழுதும் டெஸ்ட் பங்க்ஷன்
void draw_test_window(VirtualScreen *scr) {
    // 1. Top Bar (Browser style Tab) எழுதுதல்
    const char *top_bar = " [ 1: Bash ] [ 2: nvim main.c * ] [ + New ] ";
    for (int i = 0; i < strlen(top_bar) && i < scr->cols; i++) {
        screen_put_char(scr, 0, i, top_bar[i]);
    }

    // 2. விண்டோ பார்டர்களுக்கான எல்லைகள் (Row 2 முதல் Row 10 வரை, Col 5 முதல் Col 50 வரை)
    int start_row = 2, end_row = 10;
    int start_col = 5, end_col = 50;

    // மூலைகள் (Corners)
    screen_put_char(scr, start_row, start_col, '+');
    screen_put_char(scr, start_row, end_col, '+');
    screen_put_char(scr, end_row, start_col, '+');
    screen_put_char(scr, end_row, end_col, '+');

    // மேல் மற்றும் கீழ் கோடுகள் (Horizontal lines)
    for (int c = start_col + 1; c < end_col; c++) {
        screen_put_char(scr, start_row, c, '-');
        screen_put_char(scr, end_row, c, '-');
    }

    // இடது மற்றும் வலது கோடுகள் (Vertical lines)
    for (int r = start_row + 1; r < end_row; r++) {
        screen_put_char(scr, r, start_col, '|');
        screen_put_char(scr, r, end_col, '|');
    }

    // 3. விண்டோ உள்ளே ஒரு செய்தி (Window Title & Text)
    const char *title = " BDH TERMINAL - FLOATING WINDOW ";
    for (int i = 0; i < strlen(title); i++) {
        screen_put_char(scr, start_row + 2, start_col + 4 + i, title[i]);
    }

    const char *msg = "Powered by தமிழி (Tamizhi)";
    for (int i = 0; i < strlen(msg); i++) {
        screen_put_char(scr, start_row + 4, start_col + 8 + i, msg[i]);
    }
}

// 2D Buffer-ல் உள்ளதை அப்படியே மானிட்டரில் பிரிண்ட் செய்ய
void render_screen_to_console(VirtualScreen *scr) {
    printf("\033[2J\033[H"); // பழைய திரையை Clear செய்து Cursor-ஐ ஆரம்பத்திற்கு கொண்டுவர

    for (int r = 0; r < scr->rows; r++) {
        for (int c = 0; c < scr->cols; c++) {
            putchar(scr->grid[r][c].ch);
        }
        putchar('\n');
    }
}

int main() {
    printf("BDH Terminal Engine Phase 2 - Buffer Test...\n");

    // 15 வரிகள், 65 காலம்கள் கொண்ட Virtual Screen உருவாக்க
    VirtualScreen *scr = screen_create(15, 65);

    // பார்டர் மற்றும் டெக்ஸ்ட் வரைய
    draw_test_window(scr);

    // மானிட்டரில் ரெண்டர் செய்ய
    render_screen_to_console(scr);

    // மெமரியை விடுவிக்க
    screen_destroy(scr);

    return 0;
}


