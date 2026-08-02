// src/engine/pty.c
#define _XOPEN_SOURCE 600
#include "pty.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h> // <-- 1. struct winsize-க்காக சேர்க்கப்பட்டுள்ளது

#ifdef __linux__
#include <pty.h>
#elif defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__)
#include <util.h>
#endif

// Parent-ல் இருக்கும் environment variables-ஐ ஆக்சஸ் பண்ண
extern char **environ;

pid_t pty_spawn(char *const argv[], int *master_fd) {
    int slave_fd;
    char slave_name[1024];
    pid_t pid;

    // 2. விண்டோவின் உள் அளவை (58 columns, 13 rows) PTY-க்குச் சொல்கிறோம்:
    struct winsize ws = {
        .ws_row = 13,
        .ws_col = 58,
        .ws_xpixel = 0,
        .ws_ypixel = 0
    };

    // 3. openpty-ன் கடைசி ஆர்குமெண்டாக &ws கொடுக்கவும்:
    if (openpty(master_fd, &slave_fd, slave_name, NULL, &ws) == -1) {
        perror("[Error] openpty failed");
        return -1;
    }

    pid = fork();
    if (pid == -1) {
        perror("[Error] fork failed");
        return -1;
    }

    if (pid == 0) { // Child process (Bash Shell)
        close(*master_fd);

        if (setsid() == -1) perror("[Error] setsid failed");
        if (ioctl(slave_fd, TIOCSCTTY, NULL) == -1) perror("[Error] TIOCSCTTY failed");

        dup2(slave_fd, STDIN_FILENO);
        dup2(slave_fd, STDOUT_FILENO);
        dup2(slave_fd, STDERR_FILENO);

        if (slave_fd > STDERR_FILENO) close(slave_fd);

        // Execute the Shell with environment explicitly passed
        printf("[Phase 1] Executing Shell with explicit env: %s\n", argv[0]);
        // 'execvpe' enables path search and environment passing
        if (execvpe(argv[0], argv, environ) == -1) {
            perror("[Error] execvpe failed");
            _exit(EXIT_FAILURE);
        }

    } else { // Parent process
        close(slave_fd);
        printf("[Phase 1] Parent Engine attached to Master PTY FD: %d\n", *master_fd);
        printf("[Phase 1] Spawning shell (PID: %d)...\n", pid);
        return pid;
    }

    return -1; // Should not reach
}
