// src/engine/pty.h
#ifndef BDH_PTY_H
#define BDH_PTY_H

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

// PTY-ஐ உருவாக்கி, புதிய Shell-ஐ Spawn செய்யும் பங்க்ஷன்
// input: argv[] (எந்த shell run பண்ணனும்), master_fd (engine control FD)
// return: Child Process ID (PID)
pid_t pty_spawn(char *const argv[], int *master_fd);

#endif // BDH_PTY_H
