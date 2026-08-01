// src/main.c
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
