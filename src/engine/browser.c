// src/browser.c - BDH Custom GUI Web Browser
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

static void destroy_cb(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

int main(int argc, char *argv[]) {
    // 1. GTK Engine-ஐத் தொடங்குதல்
    gtk_init(&argc, &argv);

    // 2. மெயின் GUI விண்டோ உருவாக்குதல்
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    gtk_window_set_title(GTK_WINDOW(window), "BDH GUI Browser - Lightweight");

    // 3. WebKit Browser View உருவாக்குதல்
    GtkWidget *webview = webkit_web_view_new();
    gtk_container_add(GTK_CONTAINER(window), webview);

    // 4. விரும்பிய URL-ஐ லோட் செய்தல் (Default: Google / Custom URL)
    const char *url = (argc > 1) ? argv[1] : "https://www.google.com";
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), url);

    // 5. விண்டோவை மூடும் நிகழ்வை இணைத்தல்
    g_signal_connect(window, "destroy", G_CALLBACK(destroy_cb), NULL);
    
    // 6. திரையில் காட்டுதல்
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
