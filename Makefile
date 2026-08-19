# Makefile - BDH Pure Linux CLI Multiplexer Engine (Dynamic OS Auto-Detect Edition)

# ==============================================================================
# 🔥 OS DETECTION ENGINE (The Architect Update)
# ==============================================================================
UNAME_S := $(shell uname -s)
UNAME_O := $(shell uname -o 2>/dev/null || echo "Other")

# டீஃபால்ட் செட்டிங்ஸ்
CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE -Wall -Wextra
LDFLAGS = -lutil
PREFIX ?= /usr/local

# 1. Termux (Android) Environment
ifeq ($(UNAME_O), Android)
    CC = clang
    PREFIX = /data/data/com.termux/files/usr
    OS_NAME = Termux (Android)

# 2. Standard Linux Environment (Rocky, Arch, Ubuntu, Proot)
else ifeq ($(UNAME_S), Linux)
    CC = gcc
    OS_NAME = Standard Linux

# 3. macOS (Apple) Environment
else ifeq ($(UNAME_S), Darwin)
    CC = clang
    OS_NAME = macOS (Darwin)

# மற்ற OS-களுக்கு
else
    CC = gcc
    OS_NAME = Unknown OS
endif
# ==============================================================================

# --- BDH Linux IDE Submodule Directory ---
IDE_DIR = src/bdh-ide

# --- BDH Multiplexer Engine Sources ---
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
              src/engine/terminal.c

# --- Main Targets ---
all: check-os bdh-engine build-ide

# 🔥 பில்ட் ஆகும்முன் எந்த OS என்று ஸ்க்ரீனில் கெத்தாகக் காட்டும் லாஜிக்
check-os:
	@echo "================================================="
	@echo "🔍 Auto-Detected OS : $(OS_NAME)"
	@echo "⚙️  Selected Compiler: $(CC)"
	@echo "📁 Install Path     : $(PREFIX)"
	@echo "================================================="

# 1. Build BDH Multiplexer Engine:
bdh-engine: $(ENGINE_SRCS)
	$(CC) $(ENGINE_SRCS) $(CFLAGS) $(LDFLAGS) -o bdh-engine
	@echo "BDH Multiplexer Engine (bdh-engine) built successfully! 🚀"

# 2. Build BDH Linux IDE (Recursive Make via Submodule):
build-ide:
	@if [ -d "$(IDE_DIR)" ] && [ -f "$(IDE_DIR)/Makefile" ]; then \
		echo "Building BDH Linux IDE via Submodule... 🛠️"; \
		$(MAKE) -C $(IDE_DIR); \
	else \
		echo "⚠️ BDH Linux IDE folder not found or empty! Did you run 'git submodule add'?"; \
	fi

# --- Universal System Install Target ---
install: all
	install -Dm755 bdh-engine $(PREFIX)/bin/bdh-engine
	@if [ -d "$(IDE_DIR)" ] && [ -f "$(IDE_DIR)/Makefile" ]; then \
		echo "Installing BDH Linux IDE... 📦"; \
		$(MAKE) -C $(IDE_DIR) install PREFIX=$(PREFIX); \
	fi
	@echo "=================================================================="
	@echo "🔥 BDH Engine & IDE installed globally to $(PREFIX)/bin/ !"
	@echo "👉 Type 'bdh-engine' to launch Multiplexer Engine!"
	@echo "👉 Type 'bdh-linux-ide' (or related cmds) to launch the IDE tools!"
	@echo "=================================================================="

# --- Universal System Uninstall Target ---
uninstall:
	rm -f $(PREFIX)/bin/bdh-engine
	@if [ -d "$(IDE_DIR)" ] && [ -f "$(IDE_DIR)/Makefile" ]; then \
		$(MAKE) -C $(IDE_DIR) uninstall PREFIX=$(PREFIX); \
	fi
	@echo "BDH Engine and IDE uninstalled from system successfully!"

clean:
	rm -f bdh-engine *.o
	@if [ -d "$(IDE_DIR)" ] && [ -f "$(IDE_DIR)/Makefile" ]; then \
		echo "Cleaning BDH Linux IDE..."; \
		$(MAKE) -C $(IDE_DIR) clean; \
	fi
	@echo "Cleaned old builds successfully!"

.PHONY: all check-os build-ide clean install uninstall
