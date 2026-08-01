# Makefile
CC = gcc
CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE

# For openpty support (linux)
LDFLAGS = -lutil

SRCS = src/main.c src/engine/pty.c

all:
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o bdh-engine
