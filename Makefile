# Makefile - BDH Pure Linux CLI Multiplexer Engine
CC = gcc
CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE -Wall -Wextra
LDFLAGS = -lutil

# எல்லா Engine மற்றும் UI மாட்யூல்களும் முழுமையாக சேர்க்கப்பட்டுள்ளது:
SRCS = src/main.c \
       src/engine/pty.c \
       src/engine/screen.c \
       src/ui/panes.c \
       src/ui/wm.c \
       src/ui/tabs.c \
       src/ui/statusbar.c \
       src/engine/parser.c \
       src/engine/clipboard.c \
       src/engine/scanner.c \
       src/engine/cursor.c \
       src/engine/input.c \
       src/engine/renderer.c \
       src/engine/terminal.c

TARGET = bdh-engine

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o $(TARGET)
	@echo "BDH Pure Linux CLI Multiplexer Engine built successfully! 🚀"

clean:
	rm -f $(TARGET) *.o
	@echo "Cleaned old builds successfully!"

.PHONY: all clean
