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

# sudo கட்டளை நீக்கப்பட்டு, C கோடுகள் மட்டுமே கம்பைல் ஆகும்:
$(TARGET): $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o $(TARGET)
	@echo "BDH Pure Linux CLI Multiplexer Engine built successfully! 🚀"

clean:
	rm -f $(TARGET) *.o
	@echo "Cleaned old builds successfully!"

.PHONY: all clean
