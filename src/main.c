#include <gtk/gtk.h>
#include <webkit/webkit.h>

#define CHATGPT_URL "https://chatgpt.com/"
#define PROFILE_DIR_NAME "chatgpt-wrapper-c"
#define DATA_DIR_NAME "data"
#define CACHE_DIR_NAME "cache"
#define COOKIE_FILE_NAME "cookies.sqlite"

static GtkWindow *main_window = NULL;

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

static GtkWindow *
ensure_main_window(GtkApplication *app)
{
    if (main_window != NULL) {
        return main_window;
    }

    GtkWidget *window = gtk_application_window_new(app);
    WebKitNetworkSession *session = create_network_session();
    WebKitSettings *settings = create_web_settings();
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(g_object_new(
        WEBKIT_TYPE_WEB_VIEW,
        "network-session", session,
        "settings", settings,
        NULL
    ));

    gtk_window_set_title(GTK_WINDOW(window), "ChatGPT");
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 780);
    webkit_web_view_load_uri(web_view, CHATGPT_URL);
    gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(web_view));

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
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
