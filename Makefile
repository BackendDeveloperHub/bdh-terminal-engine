# Makefile
CC = gcc
CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE
LDFLAGS = -lutil

#SRCS = src/main.c src/engine/pty.c src/engine/screen.c src/ui/panes.c src/engine/parser.c
#SRCS = src/main.c src/engine/pty.c src/engine/screen.c src/ui/panes.c src/engine/parser.c src/engine/clipboard.c

#SRCS = src/main.c src/engine/pty.c src/engine/screen.c src/ui/panes.c src/engine/parser.c src/engine/clipboard.c src/engine/cursor.c
#SRCS = src/main.c src/engine/pty.c src/engine/screen.c src/ui/panes.c src/engine/parser.c src/engine/clipboard.c src/engine/cursor.c src/engine/input.c
#SRCS = src/main.c src/engine/pty.c src/engine/screen.c src/ui/panes.c src/engine/parser.c src/engine/clipboard.c src/engine/cursor.c src/engine/input.c src/engine/renderer.c
SRCS = src/main.c src/engine/pty.c src/engine/screen.c src/ui/panes.c src/engine/parser.c src/engine/clipboard.c src/engine/cursor.c src/engine/input.c src/engine/renderer.c src/engine/terminal.c


all:
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o bdh-engine
