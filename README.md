# ⚡ BDH Terminal Engine (`bdh-terminal-engine`)

**A 100% Pure Linux CLI Terminal Multiplexer & Standalone Text Editor**, architected & built using an AI-Orchestrated Low-Level C Workflow.

Built for Backend Developers, Systems Engineers, and CLI Enthusiasts.

---

## 📌 Overview

BDH Terminal Engine is a lightweight, high-performance terminal multiplexer inspired by tools like `tmux` and GNU `screen`, bundled with its own built-in and standalone modal text editor. Architected from the ground up to interact directly with Linux kernel pseudo-terminals (PTY), it manages virtual screen rendering, integrated text editing, and seamless text-based web browsing right inside your active terminal pane — with zero GUI dependencies.

---

## 🔥 Key Features

- **🖥️ True Multi-Tab & Multi-Shell Multiplexing**
  Spawn and manage multiple concurrent `/bin/zsh` or `/bin/bash` PTY sessions inside a single terminal window with instant keyboard and mouse switching.

- **📝 Built-in & Standalone Modal Text Editor (`bdh-edit`)**
  Includes a fully functional, lightweight VT100 text editor. Launch it standalone via `bdh-edit <filename>` from any shell, or toggle it instantly inside the multiplexer engine using `Ctrl + E` — complete with a blinking block cursor (█) and clean alternate-screen isolation.

- **📱 Responsive Multi-Platform Capacity**
  - **Linux Desktop / Server Mode:** Supports up to 18 concurrent sessions (`MAX_SESSIONS 18`).
  - **Termux / Mobile Mode:** Optimized lightweight layout supporting up to 8 concurrent sessions (`MAX_SESSIONS 8`) for clutter-free mobile screens.

- **🐭 XTerm SGR Mouse Protocol Support**
  Click directly on any tab in the top badge or footer overlay to switch active sessions instantly without touching the keyboard.

- **🔍 Built-in Token Scanner (`Ctrl + K`)**
  Scan the entire screen dynamically for URLs, IP addresses, UUIDs, and file paths, and copy them directly to your clipboard with a single keystroke.

- **🌐 100% Pure CLI Web Browser Integration**
  Browse documentation, GitHub repositories, and websites directly inside your active tab using `links`, rendered in sharp, glitch-free ASCII/VT100 text mode (`Ctrl + B`).

- **⚡ Non-Blocking Asynchronous I/O & Zero-Gap Footer**
  Powered by a responsive `select()` event loop with a 10ms timeout, ensuring zero input latency while maintaining a permanent, zero-gap Active Sessions Manager Footer Box at the bottom of your screen.

- **🛡️ 16KB Anti-Glitch & Anti-Tearing Buffer**
  Engineered with a high-capacity 16,384-byte I/O buffer to eliminate screen tearing, flickering, and ghosting during heavy stdout bursts or full-screen browser layouts.

- **📦 Lightweight & Zero Bloat**
  Compiles to tiny binaries (`bdh-engine` and `bdh-edit`). No X11, Wayland, GTK, or WebKit dependencies required — runs flawlessly on servers, SSH sessions, TTYs, Termux, and Linux desktops.

---

## 🏗️ Modular Architecture

The engine is structured into clean, decoupled C modules for maintainability and extensibility:

| Module | Source File | Core Responsibility |
|---|---|---|
| **PTY Engine** | `src/engine/pty.c` | Forking processes and creating pseudo-terminals (`pty_spawn`) via Linux system calls. |
| **Session Manager** | `src/engine/session.c` | Initializing, managing, and cleaning up multi-tab lifecycle arrays (8 to 18 sessions). |
| **Virtual Screen** | `src/engine/screen.c`, `src/ui/panes.c` | Detecting real terminal dimensions (`ioctl`) and maintaining off-screen window buffers. |
| **ANSI / VT100 Parser** | `src/engine/parser.c` | Parsing terminal escape sequences, CSI codes, cursor positioning, and line clearing. |
| **Text Editor Engine** | `src/editor/edit.c`, `src/edit_main.c` | Lightweight CLI text editing, row buffer rendering, file I/O, and standalone binary execution. |
| **Mouse Parser** | `src/engine/mouse.c`, `include/engine/mouse.h` | Decoding XTerm SGR mouse click/release events (`\033[<btn;col;rowM`) for instant UI interaction. |
| **Input & Clipboard** | `src/engine/input.c`, `src/engine/clipboard.c` | Handling raw keyboard shortcuts, mouse events, tab switching, copy/paste, and browser execution. |
| **Token Scanner** | `src/engine/scanner.c` | Extracting structured tokens (URLs, IPs, paths) from the screen buffer into clipboard storage. |
| **Footer & Tabs UI** | `src/ui/tabs.c`, `src/ui/statusbar.c` | Rendering the zero-gap BDH Active Sessions Manager footer box and status indicators. |
| **Renderer** | `src/engine/renderer.c` | Efficiently flushing the virtual screen state to the physical terminal without flickering. |
| **Terminal Control** | `src/engine/terminal.c` | Managing Linux terminal Raw Mode and restoring canonical settings cleanly on exit. |

---

## 🛠️ Prerequisites & Installation

### 1. Install System Dependencies

**Arch Linux / Manjaro:**
```bash
sudo pacman -S --needed gcc make links
```

**Ubuntu / Debian:**
```bash
sudo apt update && sudo apt install -y build-essential links
```

**Termux (Android):**
```bash
pkg update && pkg install -y clang make links
```

### 2. Build & Install the Project

Clone the repository and compile using the included Makefile:

```bash
# Clone repository
git clone https://github.com/BackendDeveloperHub/bdh-terminal-engine.git
cd bdh-terminal-engine

# Build clean executables and install globally
make clean && make && sudo make install
```

> **Note for Termux users:** Run `make clean && make && make install` (without `sudo`).

---

## 🚀 Usage

**1. Start the Multiplexer Engine**

Start the terminal engine directly from any shell (optionally pass a filename to open in the built-in editor):

```bash
bdh-engine [optional_filename.txt]
```

**2. Launch the Standalone Text Editor**

Open or create a file using the standalone CLI text editor:

```bash
bdh-edit filename.tz
```

---

## ⌨️ Keyboard & Mouse Controls / Action Triggers

| Shortcut | Action |
|---|---|
| `Ctrl + A` | Switch to the next active Tab / Session sequentially |
| `Ctrl + E` | Toggle the built-in full-screen modal text editor on/off |
| `Ctrl + B` | Launch the CLI Web Browser (`links`) inside the currently active tab |
| `Ctrl + K` | Trigger the Token Scanner to scan and copy URLs/IPs from the screen |
| `Ctrl + S` | Save the active file (while in Editor Mode) |
| `Ctrl + X` | Exit Editor Mode and return to the multiplexer shell |
| Mouse Left Click | Click on any session tab to switch directly to that session |
| `Ctrl + Q` | Safely terminate the engine and restore canonical terminal settings |

---

## 🏗️ Engineering Notes

Early iterations of `bdh-terminal-engine` experimented with embedded graphical WebKitGTK windows. However, GUI loops (`gtk_main_quit`) clashed with the low-level PTY `select()` loop, adding megabytes of bloat and restricting usage to desktop environments.

By refactoring to a 100% Pure Linux CLI Multiplexer & Modal Editor:

- **Zero Crash Rate** — Removed all display server (`DISPLAY=:0`) and GTK signal errors.
- **Server & Mobile Ready** — Perfectly functional over headless SSH connections, minimal Arch Linux installations, and Termux mobile environments.
- **High Performance** — Reduced binary size and memory footprint by over 99%.

---

## 👨‍💻 Author & Engineering Workflow

**Prabakaran202** (Backend Developer Hub - BDH Linux)
Systems Architecture & Engineering Design | AI-Assisted Low-Level C Implementation | Python / FastAPI | Arch Linux

> **A note on modern engineering:** This engine was conceptualized, architected, and debugged using an AI-assisted systems engineering workflow — combining human architectural vision with AI-driven code generation for rapid, low-level C development.
