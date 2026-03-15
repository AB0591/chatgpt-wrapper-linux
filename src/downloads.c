#include "downloads.h"

#include <gtk/gtk.h>

typedef struct {
    WebKitDownload *download;
    GtkFileDialog *dialog;
} DownloadPrompt;

static GtkWindow *
download_parent_window(WebKitDownload *download)
{
    WebKitWebView *web_view = webkit_download_get_web_view(download);
    GtkRoot *root;

    if (web_view == NULL) {
        return NULL;
    }

    root = gtk_widget_get_root(GTK_WIDGET(web_view));
    if (root != NULL && GTK_IS_WINDOW(root)) {
        return GTK_WINDOW(root);
    }

    return NULL;
}

static void
finish_prompt(DownloadPrompt *prompt)
{
    g_object_unref(prompt->download);
    g_object_unref(prompt->dialog);
    g_free(prompt);
}

static void
on_download_prompt_complete(GObject *source_object, GAsyncResult *result, gpointer user_data)
{
    DownloadPrompt *prompt = user_data;
    GError *error = NULL;
    GFile *file;

    (void)source_object;

    file = gtk_file_dialog_save_finish(prompt->dialog, result, &error);
    if (error != NULL) {
        webkit_download_cancel(prompt->download);
        g_error_free(error);
        finish_prompt(prompt);
        return;
    }
    if (file == NULL) {
        webkit_download_cancel(prompt->download);
        finish_prompt(prompt);
        return;
    }

    char *path = g_file_get_path(file);
    if (path == NULL) {
        g_object_unref(file);
        webkit_download_cancel(prompt->download);
        finish_prompt(prompt);
        return;
    }

    webkit_download_set_destination(prompt->download, path);

    g_free(path);
    g_object_unref(file);
    finish_prompt(prompt);
}

static gboolean
on_download_decide_destination(WebKitDownload *download, gchar *suggested_filename, gpointer user_data)
{
    GtkFileDialog *dialog;
    GFile *downloads_dir = NULL;
    DownloadPrompt *prompt;

    (void)user_data;

    dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Save Download");
    gtk_file_dialog_set_accept_label(dialog, "Save");
    gtk_file_dialog_set_modal(dialog, TRUE);
    gtk_file_dialog_set_initial_name(dialog, suggested_filename);

    if (g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD) != NULL) {
        downloads_dir = g_file_new_for_path(g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD));
        gtk_file_dialog_set_initial_folder(dialog, downloads_dir);
        g_object_unref(downloads_dir);
    }

    prompt = g_new0(DownloadPrompt, 1);
    prompt->download = g_object_ref(download);
    prompt->dialog = g_object_ref(dialog);

    gtk_file_dialog_save(
        dialog,
        download_parent_window(download),
        NULL,
        on_download_prompt_complete,
        prompt
    );
    g_object_unref(dialog);

    return TRUE;
}

static void
on_download_failed(WebKitDownload *download, GError *error, gpointer user_data)
{
    WebKitURIRequest *request;
    const char *uri = NULL;

    (void)user_data;

    request = webkit_download_get_request(download);
    if (request != NULL) {
        uri = webkit_uri_request_get_uri(request);
    }

    if (uri != NULL) {
        g_printerr("Download failed for %s: %s\n", uri, error->message);
    } else {
        g_printerr("Download failed: %s\n", error->message);
    }
}

static void
on_download_finished(WebKitDownload *download, gpointer user_data)
{
    const char *destination = webkit_download_get_destination(download);

    (void)user_data;

    if (destination != NULL) {
        g_printerr("Download finished: %s\n", destination);
    }
}

static void
on_download_started(WebKitNetworkSession *session, WebKitDownload *download, gpointer user_data)
{
    (void)session;
    (void)user_data;

    webkit_download_set_allow_overwrite(download, TRUE);
    g_signal_connect(
        download,
        "decide-destination",
        G_CALLBACK(on_download_decide_destination),
        NULL
    );
    g_signal_connect(download, "failed", G_CALLBACK(on_download_failed), NULL);
    g_signal_connect(download, "finished", G_CALLBACK(on_download_finished), NULL);
}

void
downloads_install(WebKitNetworkSession *session)
{
    g_signal_connect(session, "download-started", G_CALLBACK(on_download_started), NULL);
}
