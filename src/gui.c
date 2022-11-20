#include <string.h>
#include <stddef.h>
#include <gtk/gtk.h>
#include <assert.h>
#include "cast128.h"
#include "utils.h"

static GtkWidget *resultLabel;
static GtkWidget *msgInput;
static GtkWidget *keyInput;

static Cast128 cast;
static boolean cast_init = FALSE;
static byte last_key[KEYSIZE];

static void set_margin(GtkWidget* widget, int margin) {
    gtk_widget_set_margin_top(widget, margin);
    gtk_widget_set_margin_bottom(widget, margin);
    gtk_widget_set_margin_start(widget, margin);
    gtk_widget_set_margin_end(widget, margin);
}

static void on_click_encrypt(GtkWidget* btn, gpointer user_data) {
    const char* msg = gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(
                gtk_text_get_buffer(GTK_TEXT(msgInput))));
    size_t msg_len = strlen(msg);

    const char* key = gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(
                gtk_text_get_buffer(GTK_TEXT(keyInput))));
    if (strlen(key) != 16) {
        fprintf(stderr, "Key must be exactly 16 bytes long\n");
    }
    //assert(key_len-1 == KEYSIZE);

    // check if we need to initialize Cast128
    if (strncmp(last_key, key, KEYSIZE) || !cast_init) {
        Cast128_init(&cast, key);
        memcpy(last_key, key, KEYSIZE);
        cast_init = TRUE;
    }

    byte cipher[msg_len];
    Cast128_encrypt(&cast, msg, msg_len, cipher);

    char cipher_str[(2*sizeof(cipher)) + 1]; // 1 byte = 2 hex chars; also '\0'
    strhexdump(cipher_str, cipher, sizeof(cipher));
    gtk_label_set_label(GTK_LABEL(resultLabel), cipher_str);
}

static void on_click_decrypt(GtkWidget* btn, gpointer user_data) {
    const char* msg = gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(
                gtk_text_get_buffer(GTK_TEXT(msgInput))));
    const char* key = gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(
                gtk_text_get_buffer(GTK_TEXT(keyInput))));

    //gtk_label_set_label(GTK_LABEL(resultLabel), sha1_str);
}

static void activate (GtkApplication* app, gpointer user_data) {
    GtkWidget *window;

    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "CAST-128 Encryption");
    gtk_window_set_default_size (GTK_WINDOW (window), 600, 600);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), box);

    // Message input
    GtkWidget* msgLabel = gtk_label_new("Message");
    gtk_label_set_xalign(GTK_LABEL(msgLabel), 0);
    set_margin(msgLabel, 15);
    gtk_widget_set_margin_bottom(msgLabel, 2);
    gtk_box_append(GTK_BOX(box), msgLabel);

    msgInput = gtk_text_new();
    gtk_text_set_placeholder_text(GTK_TEXT(msgInput), "Text...");
    set_margin(msgInput, 15);
    gtk_widget_set_margin_bottom(msgInput, 2);
    gtk_box_append(GTK_BOX(box), msgInput);

    // Key input
    GtkWidget* keyLabel = gtk_label_new("Key");
    gtk_label_set_xalign(GTK_LABEL(keyLabel), 0);
    set_margin(keyLabel, 15);
    gtk_widget_set_margin_bottom(keyLabel, 2);
    gtk_box_append(GTK_BOX(box), keyLabel);

    keyInput = gtk_text_new();
    gtk_text_set_placeholder_text(GTK_TEXT(keyInput), "Text...");
    set_margin(keyInput, 15);
    gtk_widget_set_margin_bottom(keyInput, 2);
    gtk_box_append(GTK_BOX(box), keyInput);

    GtkWidget* btn = gtk_button_new_with_label("Encrypt");
    set_margin(btn, 15);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_click_encrypt), NULL);
    gtk_box_append(GTK_BOX(box), btn);

    resultLabel = gtk_label_new("");
    set_margin(resultLabel, 15);
    gtk_box_append(GTK_BOX(box), resultLabel);

    gtk_widget_show (window);
}

int main (int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new ("cs.cryptohacker", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    return status;
}

