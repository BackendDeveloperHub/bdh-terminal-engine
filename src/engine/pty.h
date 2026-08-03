// include/engine/pty.h - BDH PTY Module Header
#ifndef PTY_H
#define PTY_H

#include <sys/types.h>

pid_t pty_spawn(char *const argv[], int *master_fd, int rows, int cols);
int pty_resize(int master_fd, int rows, int cols); // <-- புதிய Resize ஃபங்ஷன் டிக்ளரேஷன்

#endif
