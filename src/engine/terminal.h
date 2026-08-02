// src/engine/terminal.h
#ifndef BDH_TERMINAL_H
#define BDH_TERMINAL_H

#include <termios.h>

// டெர்மினல் ரா மோட் (Raw Mode) மற்றும் அமைப்புகளை நிர்வகித்தல்
void terminal_enable_raw_mode(struct termios *orig_termios);
void terminal_disable_raw_mode(struct termios *orig_termios);

#endif // BDH_TERMINAL_H
