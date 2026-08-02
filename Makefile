# Makefile
CC = gcc
PKGS = gtk+-3.0 webkit2gtk-4.1

CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE $(shell pkg-config --cflags $(PKGS) 2>/dev/null)
LDFLAGS = -lutil $(shell pkg-config --libs $(PKGS) 2>/dev/null)

SRCS = src/main.c \
       src/engine/pty.c \
       src/engine/screen.c \
       src/ui/panes.c \
       src/engine/parser.c \
       src/engine/clipboard.c \
       src/engine/cursor.c \
       src/engine/input.c \
       src/engine/renderer.c \
       src/engine/terminal.c \
       src/engine/browser.c

TARGET = bdh-engine

all: $(TARGET)

# தேவைப்படும் GTK மற்றும் WebKit லைப்ரரிகளை ஆட்டோமேட்டிக்காக செக் செய்து இன்ஸ்டால் செய்ய:
deps:
	@echo "Checking and installing required system packages..."
	sudo pacman -S --needed gtk3 webkit2gtk

$(TARGET): deps $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o $(TARGET)
	@echo "BDH Engine built successfully with GUI Browser! 🚀"

clean:
	rm -f $(TARGET) *.o
	@echo "Cleaned old builds successfully!"

.PHONY: all deps clean
