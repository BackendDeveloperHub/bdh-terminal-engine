# Makefile - BDH Pure Linux CLI Multiplexer Engine & Standalone Editor & IDE (Universal Edition)
# --- FIX: Termux (clang) மற்றும் Arch Linux (gcc) இரண்டிற்கும் பொருந்த CC ?= gcc ---
CC ?= gcc
CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE -Wall -Wextra
LDFLAGS = -lutil
PREFIX ?= /usr/local

# --- BDH Linux IDE Submodule Directory ---
IDE_DIR = src/bdh-ide

# --- 1. BDH Multiplexer Engine Sources (bdh-engine) ---
ENGINE_SRCS = src/main.c \
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

# --- 2. BDH Standalone Editor Sources (bdh-edit) ---
EDITOR_SRCS = src/edit_main.c \
              src/editor/edit.c

# --- Main Targets ---
all: bdh-engine bdh-edit build-ide

# 1. Build BDH Multiplexer Engine:
bdh-engine: $(ENGINE_SRCS)
	$(CC) $(ENGINE_SRCS) $(CFLAGS) $(LDFLAGS) -o bdh-engine
	@echo "BDH Multiplexer Engine (bdh-engine) built successfully! 🚀"

# 2. Build BDH Standalone Text Editor:
bdh-edit: $(EDITOR_SRCS)
	$(CC) $(EDITOR_SRCS) $(CFLAGS) -o bdh-edit
	@echo "BDH Standalone Text Editor (bdh-edit) built successfully! 📝"

# 3. Build BDH Linux IDE (Recursive Make via Submodule):
build-ide:
	@if [ -d "$(IDE_DIR)" ] && [ -f "$(IDE_DIR)/Makefile" ]; then \
		echo "Building BDH Linux IDE via Submodule... 🛠️"; \
		$(MAKE) -C $(IDE_DIR); \
	else \
		echo "⚠️ BDH Linux IDE folder not found or empty! Did you run 'git submodule add'?"; \
	fi

# --- Universal System Install Target (Global CLI Commands) ---
install: all
	install -Dm755 bdh-engine $(PREFIX)/bin/bdh-engine
	install -Dm755 bdh-edit $(PREFIX)/bin/bdh-edit
	@if [ -d "$(IDE_DIR)" ] && [ -f "$(IDE_DIR)/Makefile" ]; then \
		echo "Installing BDH Linux IDE... 📦"; \
		$(MAKE) -C $(IDE_DIR) install PREFIX=$(PREFIX); \
	fi
	@echo "=================================================================="
	@echo "🔥 BDH Engine, Editor & IDE installed globally to $(PREFIX)/bin/ !"
	@echo "👉 Type 'bdh-engine [filename]' to launch Multiplexer Engine!"
	@echo "👉 Type 'bdh-edit [filename]' to launch Standalone Text Editor!"
	@echo "👉 Type 'bdh-linux-ide' (or related cmds) to launch the IDE tools!"
	@echo "=================================================================="

# --- Universal System Uninstall Target ---
uninstall:
	rm -f $(PREFIX)/bin/bdh-engine $(PREFIX)/bin/bdh-edit
	@if [ -d "$(IDE_DIR)" ] && [ -f "$(IDE_DIR)/Makefile" ]; then \
		$(MAKE) -C $(IDE_DIR) uninstall PREFIX=$(PREFIX); \
	fi
	@echo "BDH Engine, Editor, and IDE uninstalled from system successfully!"

clean:
	rm -f bdh-engine bdh-edit *.o
	@if [ -d "$(IDE_DIR)" ] && [ -f "$(IDE_DIR)/Makefile" ]; then \
		echo "Cleaning BDH Linux IDE..."; \
		$(MAKE) -C $(IDE_DIR) clean; \
	fi
	@echo "Cleaned old builds successfully!"

.PHONY: all build-ide clean install uninstall
