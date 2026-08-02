// src/browser.h
#ifndef BDH_BROWSER_H
#define BDH_BROWSER_H

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

// பிரவுசரின் முழு UI கூறுகளையும் தாங்கும் Struct
typedef struct {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *toolbar;
    GtkWidget *url_entry;
    GtkWidget *btn_back;
    GtkWidget *btn_forward;
    GtkWidget *btn_reload;
    GtkWidget *webview;
} BrowserWindow;

// Modular Browser Functions
BrowserWindow* browser_create(const char *title, int width, int height);
void browser_load_url(BrowserWindow *browser, const char *url);
void browser_destroy(BrowserWindow *browser);

#endif // BDH_BROWSER_H
