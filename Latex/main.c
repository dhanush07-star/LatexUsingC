#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

GtkWidget *window;
GtkWidget *text_view;
GtkWidget *scroll;

void open_file(GtkWidget *widget, gpointer window) {
    GtkWidget *dialog;
    GtkTextBuffer *buffer;
    gchar *content;
    gsize length;

    dialog = gtk_file_chooser_dialog_new("Open File",
        GTK_WINDOW(window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        ("_Cancel"), GTK_RESPONSE_CANCEL,
        ("_Open"), GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        FILE *file;

        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        file = fopen(filename, "r");

        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            length = ftell(file);
            rewind(file);
            content = (gchar *)malloc(length * sizeof(gchar));
            fread(content, sizeof(gchar), length, file);
            fclose(file);

            buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            gtk_text_buffer_set_text(buffer, content, -1);
            free(content);
        }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void save_file(GtkWidget *widget, gpointer window) {
    GtkWidget *dialog;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    gchar *text;

    dialog = gtk_file_chooser_dialog_new("Save File",
        GTK_WINDOW(window),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        ("_Cancel"), GTK_RESPONSE_CANCEL,
        ("_Save"), GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        FILE *file;

        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

        file = fopen(filename, "w");
        if (file != NULL) {
            fputs(text, file);
            fclose(file);
        }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void compile_latex(GtkWidget *widget, gpointer window) {
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    gchar *text;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    FILE *file = fopen("document.tex", "w");
    if (file != NULL) {
        fputs(text, file);
        fclose(file);
    }

    system("pdflatex document.tex");
    system("xdg-open document.pdf");
}

void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *box;
    GtkWidget *menu_bar;
    GtkWidget *file_menu;
    GtkWidget *file_item;
    GtkWidget *open_item;
    GtkWidget *save_item;
    GtkWidget *compile_item;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "LaTeX Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    menu_bar = gtk_menu_bar_new();
    file_menu = gtk_menu_new();

    file_item = gtk_menu_item_new_with_label("File");
    open_item = gtk_menu_item_new_with_label("Open");
    save_item = gtk_menu_item_new_with_label("Save");
    compile_item = gtk_menu_item_new_with_label("Compile");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), compile_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);
    gtk_box_pack_start(GTK_BOX(box), menu_bar, FALSE, FALSE, 0);

    g_signal_connect(open_item, "activate", G_CALLBACK(open_file), window);
    g_signal_connect(save_item, "activate", G_CALLBACK(save_file), window);
    g_signal_connect(compile_item, "activate", G_CALLBACK(compile_latex), window);

    text_view = gtk_text_view_new();
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_box_pack_start(GTK_BOX(box), scroll, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.latex_editor", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
