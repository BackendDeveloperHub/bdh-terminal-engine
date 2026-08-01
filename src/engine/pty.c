// src/engine/pty.c (மாற்றியமைக்கப்பட்ட கோடு)
#define _XOPEN_SOURCE 600
#include "pty.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef __linux__
#include <pty.h>
#elif defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__)
#include <util.h>
#endif

// Parent-ல் இருக்கும் environment variables-ஐ ஆக்சஸ் பண்ண
extern char **environ; // <--- இதைச் சேர்க்கவும்

pid_t pty_spawn(char *const argv[], int *master_fd) {
    int slave_fd;
    char slave_name[1024];
    pid_t pid;

    if (openpty(master_fd, &slave_fd, slave_name, NULL, NULL) == -1) {
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
        if (execvpe(argv[0], argv, environ) == -1) { // <--- execvpe ஆக மாற்றவும், environ-ஐ पास பண்ணவும்
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
