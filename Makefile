# Makefile
CC = gcc
CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE
LDFLAGS = -lutil

#SRCS = src/main.c src/engine/pty.c src/engine/screen.c src/ui/panes.c src/engine/parser.c
SRCS = src/main.c src/engine/pty.c src/engine/screen.c src/ui/panes.c src/engine/parser.c src/engine/clipboard.c


all:
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o bdh-engine
