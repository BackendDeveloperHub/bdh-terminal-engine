# Makefile - BDH Pure Linux CLI Multiplexer Engine (Ultimate Multi-Env Edition)

# ==============================================================================
#  ULTIMATE OS DETECTION ENGINE (The Architect Update)
# ==============================================================================
UNAME_S := $(shell uname -s)
UNAME_O := $(shell uname -o 2>/dev/null || echo "Other")
# Termux Environment-ல் இருக்கிறோமா என்று செக் செய்யும் மேஜிக்
IS_TERMUX := $(shell stat /data/data/com.termux/files/usr >/dev/null 2>&1 && echo "yes" || echo "no")
# உண்மையான லினக்ஸ் OS-ஆ என்று செக் செய்ய
HAS_OS_RELEASE := $(shell test -f /etc/os-release && echo "yes" || echo "no")

# டீஃபால்ட் செட்டிங்ஸ்
CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE -Wall -Wextra
LDFLAGS = -lutil
PREFIX ?= /usr/local

# 1. Termux & PRoot Environments
ifeq ($(IS_TERMUX), yes)
    ifeq ($(HAS_OS_RELEASE), yes)
        # Termux-க்குள் ஓடும் PRoot (Arch / Rocky)
        CC = gcc
        PREFIX = /usr/local
        OS_NAME = $(shell grep -E '^PRETTY_NAME=' /etc/os-release | cut -d '"' -f 2) [via Termux PRoot]
    else
        # உண்மையான Termux (Native Android)
        CC = clang
        PREFIX = /data/data/com.termux/files/usr
        OS_NAME = Termux (Native Android)
    endif

# 2. Standard Linux Environment (Laptop / Server)
else ifeq ($(UNAME_S), Linux)
    CC = gcc
    PREFIX = /usr/local
    OS_NAME = $(shell grep -E '^PRETTY_NAME=' /etc/os-release 2>/dev/null | cut -d '"' -f 2 || echo "Standard Linux")

# 3. macOS (Apple) Environment
else ifeq ($(UNAME_S), Darwin)
    CC = clang
    PREFIX = /usr/local
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

# --- Submodule Initialization ---
init-submodules:
	@echo "=> Checking and Updating Git Submodules... 🔄"
	@git submodule update --init --recursive 2>/dev/null || true

# --- Main Targets ---
all: init-submodules check-os bdh-engine build-ide

# பில்ட் ஆகும்முன் எந்த OS என்று ஸ்க்ரீனில் கெத்தாகக் காட்டும் லாஜிக்
check-os:
	@echo "================================================="
	@echo "Auto-Detected OS : $(OS_NAME)"
	@echo " Selected Compiler: $(CC)"
	@echo "Install Path     : $(PREFIX)"
	@echo "================================================="

# 1. Build BDH Multiplexer Engine:
bdh-engine: $(ENGINE_SRCS)
	PROOT_NO_SECCOMP=1 $(CC) $(ENGINE_SRCS) $(CFLAGS) $(LDFLAGS) -o bdh-engine
	@echo "BDH Multiplexer Engine (bdh-engine) built successfully! 🚀"

# 2. Build BDH Linux IDE (Recursive Make via Submodule):
build-ide:
	@if [ -d "$(IDE_DIR)" ] && [ -f "$(IDE_DIR)/Makefile" ]; then \
		echo "Building BDH Linux IDE via Submodule... 🛠️"; \
		$(MAKE) -C $(IDE_DIR); \
	else \
		echo " BDH Linux IDE folder not found or empty! Did you run 'git submodule add'?"; \
	fi

# --- Universal System Install Target ---
install: all
	install -Dm755 bdh-engine $(PREFIX)/bin/bdh-engine
	@if [ -d "$(IDE_DIR)" ] && [ -f "$(IDE_DIR)/Makefile" ]; then \
		echo "Installing BDH Linux IDE... "; \
		$(MAKE) -C $(IDE_DIR) install PREFIX=$(PREFIX); \
	fi
	@echo "=================================================================="
	@echo "BDH Engine & IDE installed globally to $(PREFIX)/bin/ !"
	@echo "Type 'bdh-engine' to launch Multiplexer Engine!"
	@echo "Type 'bdh-linux-ide'  type to bdh-ide (or related cmds) to launch the IDE tools!"
    @echo "type 'bdh-tree' to explore system structure"
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

.PHONY: all init-submodules check-os build-ide clean install uninstall
