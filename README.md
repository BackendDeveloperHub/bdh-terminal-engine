# ⚡ BDH Terminal Engine (bdh-terminal-engine)
> **A 100% Pure Linux CLI Terminal Multiplexer architected & built using an AI-Orchestrated Low-Level C Workflow.**  
> Built for Backend Developers, Systems Engineers, and CLI Enthusiasts.
---
## 📌 Overview
**BDH Terminal Engine** is a lightweight, high-performance terminal multiplexer inspired by tools like `tmux` and `GNU Screen`. Architected from the ground up to interact directly with Linux kernel pseudo-terminals (**PTY**), it manages virtual screen rendering and integrates seamless text-based web browsing right inside your active terminal pane—with **zero GUI dependencies**.
---
## 🔥 Key Features
* **🖥️ True Multi-Tab & Multi-Shell Multiplexing**  
  Spawn and manage multiple concurrent `/bin/zsh` or `/bin/bash` PTY sessions inside a single terminal window with instant keyboard and mouse switching.
* **📱 Responsive Multi-Platform Capacity**  
  * **Linux Desktop / Server Mode:** Supports up to **18 concurrent sessions** (`MAX_SESSIONS 18`).
  * **Termux / Mobile Mode:** Optimized lightweight layout supporting up to **8 concurrent sessions** (`MAX_SESSIONS 8`) for clutter-free mobile screens.
* **🐭 XTerm SGR Mouse Protocol Support**  
  Click directly on any tab in the top badge or footer overlay to switch active sessions instantly without touching the keyboard.
* **🔍 Built-in Token Scanner (`Ctrl + K`)**  
  Scan the entire screen dynamically for URLs, IP addresses, UUIDs, and file paths, and copy them directly to your clipboard with a single keystroke.
* **🌐 100% Pure CLI Web Browser Integration**  
  Browse documentation, GitHub repositories, and websites directly inside your active tab using **`links`**—rendered in sharp, glitch-free ASCII/VT100 text mode (`Ctrl + B`).
* **⚡ Non-Blocking Asynchronous I/O & Zero-Gap Footer**  
  Powered by a responsive `select()` event loop with a **10ms timeout**, ensuring zero input latency while maintaining a permanent, zero-gap **Active Sessions Manager Footer Box** at the bottom of your screen.
* **🛡️ 16KB Anti-Glitch & Anti-Tearing Buffer**  
  Engineered with a high-capacity **16,384-byte I/O buffer** to eliminate screen tearing, flickering, and ghosting during heavy stdout bursts or full-screen browser layouts.
* **📦 Lightweight & Zero Bloat**  
  Compiles to a tiny binary (~40 KB). No X11, Wayland, GTK, or WebKit dependencies required—runs flawlessly on servers, SSH sessions, TTYs, Termux, and Linux desktops.
---
## 🏗️ Modular Architecture
The engine is structured into clean, decoupled C modules for maintainability and extensibility:

| Module | Source File | Core Responsibility |
| :--- | :--- | :--- |
| **PTY Engine** | `src/engine/pty.c` | Forking processes and creating pseudo-terminals (`pty_spawn`) via Linux system calls. |
| **Session Manager** | `src/engine/session.c` | Initializing, managing, and cleaning up multi-tab lifecycle arrays (8 to 18 sessions). |
| **Virtual Screen** | `src/engine/screen.c`, `src/ui/panes.c` | Detecting real terminal dimensions (`ioctl`) and maintaining off-screen window buffers. |
| **ANSI / VT100 Parser** | `src/engine/parser.c` | Parsing terminal escape sequences, CSI codes, cursor positioning, and line clearing. |
| **Mouse Parser** | `src/engine/mouse.c`, `include/engine/mouse.h` | Decoding XTerm SGR mouse click/release events (`\033[<btn;col;rowM`) for instant UI interaction. |
| **Input & Clipboard** | `src/engine/input.c`, `src/engine/clipboard.c` | Handling raw keyboard shortcuts, mouse events, tab switching, copy/paste, and browser execution. |
| **Token Scanner** | `src/engine/scanner.c` | Extracting structured tokens (URLs, IPs, paths) from the screen buffer into clipboard storage. |
| **Footer & Tabs UI** | `src/ui/tabs.c`, `src/ui/statusbar.c` | Rendering the zero-gap **BDH Active Sessions Manager** footer box and status indicators. |
| **Renderer** | `src/engine/renderer.c` | Efficiently flushing the virtual screen state to the physical terminal without flickering. |
| **Terminal Control** | `src/engine/terminal.c` | Managing Linux terminal Raw Mode and restoring canonical settings cleanly on exit. |

---
## 🛠️ Prerequisites & Installation
### 1. Install System Dependencies
#### Arch Linux / Manjaro:
```bash
sudo pacman -S --needed gcc make links
```
ubuntu / Debian:
```bash
sudo apt update && sudo apt install -y build-essential links
```
Termux (Android):
```bash
pkg update && pkg install -y clang make links
```
2. Build the Project
Clone the repository and compile using the included Makefile:
``` bash
# Clone repository
git clone [https://github.com/BackendDeveloperHub/bdh-terminal-engine.git](https://github.com/BackendDeveloperHub/bdh-terminal-engine.git)
cd bdh-terminal-engine

# Build clean executable and install
make clean && make && sudo make install

```

(Note for Termux users: Run make clean && make && make install without sudo)

Usage
Start the terminal engine directly from your shell:








