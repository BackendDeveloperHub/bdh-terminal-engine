# Makefile - BDH Pure Linux CLI Multiplexer Engine
CC = gcc
CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE -Wall -Wextra
LDFLAGS = -lutil

SRCS = src/main.c \
       src/engine/pty.c \
       src/engine/screen.c \
       src/ui/panes.c \
       src/engine/parser.c \
       src/engine/clipboard.c \
       src/engine/cursor.c \
       src/engine/input.c \
       src/engine/renderer.c \
       src/engine/terminal.c

TARGET = bdh-engine

all: $(TARGET)

# CLI Browser-க்கான links பேக்கேஜை மட்டும் செக் செய்ய:
deps:
	@echo "Checking and installing required CLI browser package..."
	sudo pacman -S --needed links

$(TARGET): deps $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o $(TARGET)
	@echo "BDH Pure Linux CLI Multiplexer Engine built successfully! 🚀"

clean:
	rm -f $(TARGET) *.o
	@echo "Cleaned old builds successfully!"

.PHONY: all deps clean
