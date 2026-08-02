// src/engine/browser.c
#include "browser.h"
#include <stdlib.h>
#include <string.h>

static void on_window_destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

BrowserWindow* browser_create(const char *title, int width, int height) {
    BrowserWindow *browser = (BrowserWindow*)malloc(sizeof(BrowserWindow));

    // 1. Main Window உருவாக்குதல்
    browser->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(browser->window), width, height);
    gtk_window_set_title(GTK_WINDOW(browser->window), title);

    // 2. Layout Box
    browser->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(browser->window), browser->vbox);

    // 3. WebKit WebView உருவாக்குதல்
    browser->webview = webkit_web_view_new();
    gtk_box_pack_start(GTK_BOX(browser->vbox), browser->webview, TRUE, TRUE, 0);

    // 4. Close Event இணைத்தல்
    g_signal_connect(browser->window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    return browser;
}

void browser_load_url(BrowserWindow *browser, const char *url) {
    if (!browser || !browser->webview || !url) return;
    
    if (g_str_has_prefix(url, "http://") || g_str_has_prefix(url, "https://")) {
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(browser->webview), url);
    } else {
        char full_url[1024];
        snprintf(full_url, sizeof(full_url), "https://%s", url);
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(browser->webview), full_url);
    }
}

void browser_destroy(BrowserWindow *browser) {
    if (browser) {
        free(browser);
    }
}
