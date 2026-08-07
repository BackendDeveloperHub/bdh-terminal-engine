// src/editor/main.c - Standalone BDH Edit Executable (bdh-edit filename.tz)
#include "editor/edit.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>

static struct termios orig_termios;

static void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    write(STDOUT_FILENO, "\033[?1049l\033[?25h", 13); // Alternate screen-ல் இருந்து வெளியேறுதல் & கர்சரைக் காட்டுதல்
}

static void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    write(STDOUT_FILENO, "\033[?1049h\033[H", 11); // Alternate screen-க்குள் நுழைதல்
}

int main(int argc, char *argv[]) {
    // 1. ஃபைல் பெயர் கொடுக்கவில்லை என்றால் டீஃபால்ட் பெயர், கொடுத்தால் அந்தப் பெயர் (எ.கா: tamil.tz)
    const char *filename = (argc > 1) ? argv[1] : "untitled.txt";

    EditorState *ed = editor_create();
    if (!ed) {
        fprintf(stderr, "Fatal: Failed to allocate EditorState\r\n");
        return EXIT_FAILURE;
    }

    // 2. ஃபைலைத் திறத்தல் (இல்லையென்றால் புதிய ஃபைல் உருவாக்குவதற்காக மெமரி ரெடி ஆகும்):
    editor_open(ed, filename);
    ed->is_active = 1;

    enable_raw_mode();

    char buf[16];
    ssize_t nread;
    int running = 1;

    struct winsize ws;
    int rows = 24, cols = 80;

    while (running) {
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) {
            rows = ws.ws_row;
            cols = ws.ws_col;
        }

        // 🔥 திரையை வரைதல்:
        write(STDOUT_FILENO, "\033[?25l", 6); // வரையும்போது கர்சரை மறைத்தல்
        editor_draw(ed, NULL, rows, cols);
        write(STDOUT_FILENO, "\033[?25h", 6); // வரைந்த பின் கர்சரைக் காட்டுதல்

        // 🔥 கீபோர்டு உள்ளீடு (Keyboard Input):
        nread = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (nread <= 0) continue;
        buf[nread] = '\0';

        // --- SHORTCUTS ---
        if (buf[0] == 19 && nread == 1) { // Ctrl + S = Save File
            editor_save(ed);
            continue;
        }
        if (buf[0] == 24 && nread == 1) { // Ctrl + X = Exit Editor
            running = 0;
            break;
        }

        // Arrow keys, typing, backspace அனைத்தும் உள்ளே செல்லும்:
        editor_handle_key(ed, buf, nread);
    }

    editor_destroy(ed);
    return EXIT_SUCCESS;
}
