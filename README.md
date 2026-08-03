# ⚡ BDH Terminal Engine (bdh-terminal-engine)
> **A 100% Pure Linux CLI Terminal Multiplexer & Virtual Screen Engine written in Low-Level C.**  
> Built for Backend Developers, Systems Engineers, and CLI Enthusiasts.
---
## 📌 Overview
**BDH Terminal Engine** is a lightweight, high-performance terminal multiplexer inspired by tools like `tmux` and `GNU Screen`. Built from the ground up in C, it interacts directly with Linux kernel pseudo-terminals (**PTY**), manages virtual screen rendering, and integrates seamless text-based web browsing right inside your active terminal pane—with **zero GUI dependencies**.
---
## 🔥 Key Features
* **🖥️ True Multi-Tab / Session Multiplexing**  
  Spawn and manage multiple concurrent `/bin/bash` PTY sessions (`[ TAB 1/2 ]`, `[ TAB 2/2 ]`) inside a single terminal window with instant keyboard switching.
* **🌐 100% Pure CLI Web Browser Integration**  
  Browse documentation, GitHub repositories, and websites directly inside your active Bash tab using **`links`**—rendered in sharp, glitch-free ASCII/VT100 text mode (`Ctrl + B`).
* **⚡ Non-Blocking Asynchronous I/O**  
  Powered by a responsive `select()` event loop with a **10ms timeout**, ensuring zero input latency while handling simultaneous shell output streams.
* **🛡️ 16KB Anti-Glitch & Anti-Tearing Buffer**  
  Engineered with a high-capacity **16,384-byte I/O buffer** to eliminate screen tearing, flickering, and ghosting during heavy stdout bursts or full-screen browser layouts.
* **📦 Lightweight & Zero Bloat**  
  Compiles to a tiny binary (~40 KB). No X11, Wayland, GTK, or WebKit dependencies required—runs flawlessly on servers, SSH sessions, TTYs, and Linux desktops.
---
## 🏗️ Modular Architecture
The engine is structured into clean, decoupled C modules for maintainability and extensibility:

| Module | Source File | Core Responsibility |
| :--- | :--- | :--- |
| **PTY Engine** | `src/engine/pty.c` | Forking processes and creating pseudo-terminals (`pty_spawn`) via Linux system calls. |
| **Virtual Screen** | `src/engine/screen.c`, `src/ui/panes.c` | Detecting real terminal dimensions (`ioctl`) and maintaining off-screen window buffers. |
| **ANSI / VT100 Parser** | `src/engine/parser.c` | Parsing terminal escape sequences, CSI codes, cursor positioning, and line clearing. |
| **Input & Clipboard** | `src/engine/input.c`, `src/engine/clipboard.c` | Handling raw keyboard events, tab switching, copy/paste, and browser execution. |
| **Renderer** | `src/engine/renderer.c` | Efficiently flushing the virtual screen state to the physical terminal without flickering. |
| **Terminal Control** | `src/engine/terminal.c` | Managing Linux terminal Raw Mode and restoring canonical settings on exit. |

---
## 🛠️ Prerequisites & Installation
### 1. Install System Dependencies (Arch Linux)
The engine only requires the standard C build tools and the **`links`** CLI browser for web support:
```bash
sudo pacman -S --needed gcc make links
```
For Ubuntu/Debian: sudo apt install build-essential links)
2. Build the Project
Clone the repository and compile using the included Makefile:
```bash

# Clone repository
git clone [https://github.com/BackendDeveloperHub/bdh-terminal-engine.git](https://github.com/BackendDeveloperHub/bdh-terminal-engine.git)
cd bdh-terminal-engine

# Build clean executable
make clean
make

```
🚀 Usage
Start the terminal engine directly from your shell:

```bash
./bdh-engine
```
Default Keyboard Controls / Action Triggers
​Switch Active Tab / Bash Pane: Triggers window switch between Primary and Secondary Bash sessions.
​Open CLI Web Browser (Ctrl + B): Launches links https://github.com/BackendDeveloperHub inside the currently active tab.
​Exit Browser: Press q and confirm with y to return immediately to your shell prompt.
​Exit Engine: Type exit inside all active shell sessions or trigger clean engine termination.
​💡 Engineering Design Decision: Why Pure CLI?
​Early iterations of bdh-terminal-engine experimented with embedded graphical WebKitGTK windows. However, GUI loops (gtk_main_quit) clashed with the low-level PTY select() loop, adding megabytes of bloat and restricting usage to desktop environments.
​By refactoring to a 100% Pure Linux CLI Multiplexer:
​Zero Crash Rate: Removed all display server (DISPLAY=:0) and GTK signal errors.
​Server Ready: Perfectly functional over headless SSH connections and minimal Linux installations.
​High Performance: Reduced binary size and memory footprint by over 99%.
​👨‍💻 Author
​Prabakaran (Backend Developer Hub - BDH Linux)
Systems Engineering | Python / FastAPI | Low-Level C | Arch Linux
