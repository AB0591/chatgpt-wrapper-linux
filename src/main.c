#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "continuation.h"
#include "downloads.h"
#include "hotkey.h"
#include "settings.h"

#define CHATGPT_URL "https://chatgpt.com/"
#define PROFILE_DIR_NAME "chatgpt-wrapper-c"
#define DATA_DIR_NAME "data"
#define CACHE_DIR_NAME "cache"
#define COOKIE_FILE_NAME "cookies.sqlite"

static GtkWindow *main_window = NULL;
static GtkWidget *back_button = NULL;
static GtkWidget *forward_button = NULL;
static GtkWidget *continue_button = NULL;

static char *
build_profile_path(const char *base_dir, const char *leaf_dir)
{
    return g_build_filename(base_dir, PROFILE_DIR_NAME, leaf_dir, NULL);
}

static void
ensure_directory_exists(const char *path)
{
    if (g_mkdir_with_parents(path, 0700) != 0) {
        g_printerr("Failed to create %s\n", path);
    }
}

static WebKitNetworkSession *
create_network_session(void)
{
    char *data_dir = build_profile_path(g_get_user_data_dir(), DATA_DIR_NAME);
    char *cache_dir = build_profile_path(g_get_user_cache_dir(), CACHE_DIR_NAME);
    char *cookie_path = g_build_filename(data_dir, COOKIE_FILE_NAME, NULL);
    WebKitNetworkSession *session;
    WebKitCookieManager *cookie_manager;

    ensure_directory_exists(data_dir);
    ensure_directory_exists(cache_dir);

    session = webkit_network_session_new(data_dir, cache_dir);
    webkit_network_session_set_persistent_credential_storage_enabled(session, TRUE);

    cookie_manager = webkit_network_session_get_cookie_manager(session);
    webkit_cookie_manager_set_accept_policy(
        cookie_manager,
        WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS
    );
    webkit_cookie_manager_set_persistent_storage(
        cookie_manager,
        cookie_path,
        WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE
    );
    downloads_install(session);

    g_free(cookie_path);
    g_free(cache_dir);
    g_free(data_dir);

    return session;
}

static WebKitSettings *
create_web_settings(void)
{
    WebKitSettings *settings = webkit_settings_new();

    webkit_settings_set_enable_javascript(settings, TRUE);
    webkit_settings_set_enable_html5_local_storage(settings, TRUE);
    webkit_settings_set_enable_media(settings, TRUE);
    webkit_settings_set_enable_media_stream(settings, TRUE);
    webkit_settings_set_enable_mediasource(settings, TRUE);
    webkit_settings_set_enable_webgl(settings, TRUE);

    return settings;
}

static void
sync_navigation_buttons(WebKitWebView *web_view)
{
    if (back_button != NULL) {
        gtk_widget_set_sensitive(
            back_button,
            webkit_web_view_can_go_back(web_view)
        );
    }

    if (forward_button != NULL) {
        gtk_widget_set_sensitive(
            forward_button,
            webkit_web_view_can_go_forward(web_view)
        );
    }
}

static void
persist_window_size(GtkWindow *window)
{
    int width;
    int height;

    gtk_window_get_default_size(window, &width, &height);
    settings_store_window(width, height);
}

static void
on_back_clicked(GtkButton *button, gpointer user_data)
{
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(user_data);

    (void)button;

    if (webkit_web_view_can_go_back(web_view)) {
        webkit_web_view_go_back(web_view);
    }
}

static void
on_forward_clicked(GtkButton *button, gpointer user_data)
{
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(user_data);

    (void)button;

    if (webkit_web_view_can_go_forward(web_view)) {
        webkit_web_view_go_forward(web_view);
    }
}

static void
on_reload_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    webkit_web_view_reload(WEBKIT_WEB_VIEW(user_data));
}

static void
on_continue_clicked(GtkButton *button, gpointer user_data)
{
    WebKitWebView *web_view = g_object_get_data(G_OBJECT(button), "chatgpt-webview");
    GtkWindow *window = GTK_WINDOW(user_data);

    continuation_start(window, web_view);
}

static void
on_web_view_load_changed(WebKitWebView *web_view, WebKitLoadEvent load_event, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    const char *title = webkit_web_view_get_title(web_view);

    (void)load_event;

    if (title != NULL && title[0] != '\0') {
        gtk_window_set_title(window, title);
    } else {
        gtk_window_set_title(window, "ChatGPT");
    }

    sync_navigation_buttons(web_view);
}

static void
on_web_view_notify_title(WebKitWebView *web_view, GParamSpec *pspec, gpointer user_data)
{
    const char *title = webkit_web_view_get_title(web_view);

    (void)pspec;

    if (title != NULL && title[0] != '\0') {
        gtk_window_set_title(GTK_WINDOW(user_data), title);
    } else {
        gtk_window_set_title(GTK_WINDOW(user_data), "ChatGPT");
    }
}

static void
on_web_view_notify_uri(WebKitWebView *web_view, GParamSpec *pspec, gpointer user_data)
{
    (void)pspec;
    (void)user_data;
    sync_navigation_buttons(web_view);
}

static void
on_window_notify_default_size(GObject *object, GParamSpec *pspec, gpointer user_data)
{
    (void)pspec;
    (void)user_data;
    persist_window_size(GTK_WINDOW(object));
}

static GtkWindow *
ensure_main_window(GtkApplication *app)
{
    WindowSettings stored = settings_load_window();

    if (main_window != NULL) {
        return main_window;
    }

    GtkWidget *window = gtk_application_window_new(app);
    GtkWidget *header = gtk_header_bar_new();
    GtkWidget *reload_button = gtk_button_new_with_label("Reload");
    WebKitNetworkSession *session = create_network_session();
    WebKitSettings *settings = create_web_settings();
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(g_object_new(
        WEBKIT_TYPE_WEB_VIEW,
        "network-session", session,
        "settings", settings,
        NULL
    ));

    back_button = gtk_button_new_with_label("Back");
    forward_button = gtk_button_new_with_label("Forward");
    continue_button = gtk_button_new_with_label("New Chat with Context");

    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header), TRUE);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), back_button);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), forward_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), continue_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), reload_button);

    gtk_window_set_title(GTK_WINDOW(window), "ChatGPT");
    gtk_window_set_default_size(GTK_WINDOW(window), stored.width, stored.height);
    gtk_window_set_titlebar(GTK_WINDOW(window), header);
    webkit_web_view_load_uri(web_view, CHATGPT_URL);
    gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(web_view));

    g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_clicked), web_view);
    g_signal_connect(forward_button, "clicked", G_CALLBACK(on_forward_clicked), web_view);
    g_signal_connect(reload_button, "clicked", G_CALLBACK(on_reload_clicked), web_view);
    g_object_set_data(G_OBJECT(continue_button), "chatgpt-webview", web_view);
    g_signal_connect(continue_button, "clicked", G_CALLBACK(on_continue_clicked), window);
    g_signal_connect(
        web_view,
        "load-changed",
        G_CALLBACK(on_web_view_load_changed),
        window
    );
    g_signal_connect(
        web_view,
        "notify::title",
        G_CALLBACK(on_web_view_notify_title),
        window
    );
    g_signal_connect(
        web_view,
        "notify::uri",
        G_CALLBACK(on_web_view_notify_uri),
        NULL
    );
    g_signal_connect(
        window,
        "notify::default-width",
        G_CALLBACK(on_window_notify_default_size),
        NULL
    );
    g_signal_connect(
        window,
        "notify::default-height",
        G_CALLBACK(on_window_notify_default_size),
        NULL
    );

    sync_navigation_buttons(web_view);

    g_object_unref(settings);
    g_object_unref(session);

    main_window = GTK_WINDOW(window);
    g_object_add_weak_pointer(G_OBJECT(window), (gpointer *)&main_window);

    return main_window;
}

static void
show_main_window(GtkApplication *app)
{
    GtkWindow *window = ensure_main_window(app);

    gtk_window_unminimize(window);
    gtk_window_present(window);
}

static void
hide_main_window(GtkApplication *app)
{
    (void)app;

    if (main_window == NULL) {
        return;
    }

    gtk_widget_set_visible(GTK_WIDGET(main_window), FALSE);
}

static void
toggle_main_window(GtkApplication *app)
{
    GtkWindow *window = ensure_main_window(app);

    if (gtk_widget_get_visible(GTK_WIDGET(window)) && gtk_window_is_active(window)) {
        gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
        return;
    }

    gtk_window_unminimize(window);
    gtk_window_present(window);
}

static void
on_show_window(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    (void)action;
    (void)parameter;
    show_main_window(GTK_APPLICATION(user_data));
}

static void
on_hide_window(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    (void)action;
    (void)parameter;
    hide_main_window(GTK_APPLICATION(user_data));
}

static void
on_toggle_window(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    (void)action;
    (void)parameter;
    toggle_main_window(GTK_APPLICATION(user_data));
}

static void
install_actions(GtkApplication *app)
{
    GActionEntry entries[] = {
        { "show-window", on_show_window, NULL, NULL, NULL, {0} },
        { "hide-window", on_hide_window, NULL, NULL, NULL, {0} },
        { "toggle-window", on_toggle_window, NULL, NULL, NULL, {0} },
    };

    g_action_map_add_action_entries(
        G_ACTION_MAP(app),
        entries,
        G_N_ELEMENTS(entries),
        app
    );
}

static void
on_app_activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;
    show_main_window(app);
}

int
main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new(
        "com.example.chatgptwrapper",
        G_APPLICATION_DEFAULT_FLAGS
    );
    int status;

    install_actions(app);
    hotkey_start(app);
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
