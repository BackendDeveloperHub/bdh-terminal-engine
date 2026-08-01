// src/engine/pty.c
#define _XOPEN_SOURCE 600 // required for openpty setup
#include "pty.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef __linux__
#include <pty.h> // compile with -lutil on older systems
#elif defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__)
#include <util.h> // compile with -lutil on older systems
#endif

pid_t pty_spawn(char *const argv[], int *master_fd) {
    int slave_fd;
    char slave_name[1024];
    pid_t pid;

    // 1. Open Master/Slave PTY pair
    if (openpty(master_fd, &slave_fd, slave_name, NULL, NULL) == -1) {
        perror("[Error] openpty failed");
        return -1;
    }

    // 2. Fork the process
    pid = fork();
    if (pid == -1) {
        perror("[Error] fork failed");
        return -1;
    }

    if (pid == 0) { // Child process (Bash Shell)

        close(*master_fd); // Child doesn't need Master end

        // 3. Establish as Controlling Terminal
        if (setsid() == -1) perror("[Error] setsid failed");

        if (ioctl(slave_fd, TIOCSCTTY, NULL) == -1) perror("[Error] TIOCSCTTY failed");

        // 4. Redirect stdin, stdout, stderr to Slave PTY
        dup2(slave_fd, STDIN_FILENO);
        dup2(slave_fd, STDOUT_FILENO);
        dup2(slave_fd, STDERR_FILENO);

        if (slave_fd > STDERR_FILENO) close(slave_fd); // close slave FD after dup

        // 5. Execute the Shell
        printf("[Phase 1] Executing Shell: %s\n", argv[0]);
        if (execvp(argv[0], argv) == -1) {
            perror("[Error] execvp failed");
            _exit(EXIT_FAILURE); // kill child if exec fails
        }

    } else { // Parent process (Terminal Engine)

        close(slave_fd); // Parent doesn't need Slave end
        printf("[Phase 1] Parent Engine attached to Master PTY FD: %d\n", *master_fd);
        printf("[Phase 1] Spawning shell (PID: %d)...\n", pid);

        return pid; // Return child PID
    }

    return -1; // Should not reach
}
