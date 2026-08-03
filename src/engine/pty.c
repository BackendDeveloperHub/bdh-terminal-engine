// src/engine/pty.c - BDH Pure Linux CLI Multiplexer PTY Engine (100% Complete)
#define _XOPEN_SOURCE 600
#include "pty.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#ifdef __linux__
#include <pty.h>
#elif defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__)
#include <util.h>
#endif

extern char **environ;

pid_t pty_spawn(char *const argv[], int *master_fd, int rows, int cols) {
    int slave_fd;
    char slave_name[1024];
    pid_t pid;

    // நாம் அனுப்பும் அசல் லேப்டாப் ஸ்கிரீன் அளவை PTY-க்குச் சொல்கிறோம்:
    struct winsize ws = {
        .ws_row = rows,
        .ws_col = cols,
        .ws_xpixel = 0,
        .ws_ypixel = 0
    };

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

        if (execvpe(argv[0], argv, environ) == -1) {
            perror("[Error] execvpe failed");
            _exit(EXIT_FAILURE);
        }

    } else { // Parent process
        close(slave_fd);
        return pid;
    }

    return -1;
}

// --- ADDED: லே-அவுட் அளவு மாறும்போது PTY-க்கு புதிய அளவைத் தெரிவிக்கும் ஃபங்ஷன் ---
int pty_resize(int master_fd, int rows, int cols) {
    struct winsize ws = {
        .ws_row = rows,
        .ws_col = cols,
        .ws_xpixel = 0,
        .ws_ypixel = 0
    };
    
    if (ioctl(master_fd, TIOCSWINSZ, &ws) == -1) {
        perror("[Error] TIOCSWINSZ failed");
        return -1;
    }
    return 0;
}
