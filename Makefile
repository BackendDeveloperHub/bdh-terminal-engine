# Makefile
#CC = gcc
#CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE

# For openpty support (linux)
#LDFLAGS = -lutil

#₹SRCS = src/main.c src/engine/pty.c
#
#all:
	#$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o bdh-engine
# Makefile
CC = gcc
CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE
LDFLAGS = -lutil

# Phase 2 Screen buffer-ஐயும் சேர்க்கிறோம்
SRCS = src/main.c src/engine/pty.c src/engine/screen.c

all:
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o bdh-engine
