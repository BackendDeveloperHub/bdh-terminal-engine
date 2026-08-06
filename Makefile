# Makefile - BDH Pure Linux CLI Multiplexer Engine (Universal Edition)
# --- FIX: Termux (clang) மற்றும் Arch Linux (gcc) இரண்டிற்கும் தானாகவே பொருந்த CC ?= gcc ---
CC ?= gcc
CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE -Wall -Wextra
LDFLAGS = -lutil
PREFIX ?= /usr/local

# --- FIX: புதிய session.c மாட்யூல் இங்கு சேர்க்கப்பட்டுள்ளது ---
SRCS = src/main.c \
       src/engine/pty.c \
       src/engine/screen.c \
       src/engine/session.c \
       src/engine/mouse.c \
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
       src/engine/terminal.c \
       src/editor/edit.c 

TARGET = bdh-engine

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o $(TARGET)
	@echo "BDH Pure Linux CLI Multiplexer Engine built successfully! 🚀"

# --- Universal System Install Target (Global CLI Command) ---
install: $(TARGET)
	install -Dm755 $(TARGET) $(PREFIX)/bin/$(TARGET)
	@echo "=================================================================="
	@echo "🔥 BDH Engine installed globally to $(PREFIX)/bin/$(TARGET) !"
	@echo "👉 Type 'bdh-engine' from ANY folder in your terminal to run!"
	@echo "=================================================================="

# --- Universal System Uninstall Target ---
uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)
	@echo "BDH Engine uninstalled from system successfully!"

clean:
	rm -f $(TARGET) *.o
	@echo "Cleaned old builds successfully!"

.PHONY: all clean install uninstall
