# Makefile
CC = gcc
PKGS = gtk+-3.0 webkit2gtk-4.1

# GTK & WebKit2GTK-க்கான CFLAGS மற்றும் LDFLAGS தானாகவே இணைக்கப்படுகிறது:
CFLAGS = -Iinclude -Isrc -D_GNU_SOURCE $(shell pkg-config --cflags $(PKGS))
LDFLAGS = -lutil $(shell pkg-config --libs $(PKGS))

# புதிய browser.c ஃபைலும் SRCS-ல் சேர்க்கப்பட்டுள்ளது:
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

$(TARGET): $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o
	@echo "Cleaned old builds successfully!"

.PHONY: all clean
