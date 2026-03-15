#include "settings.h"

#include <glib.h>

#define PROFILE_DIR_NAME "chatgpt-wrapper-c"
#define SETTINGS_FILE_NAME "settings.ini"
#define WINDOW_GROUP_NAME "window"
#define WINDOW_WIDTH_KEY "width"
#define WINDOW_HEIGHT_KEY "height"
#define DEFAULT_WINDOW_WIDTH 1100
#define DEFAULT_WINDOW_HEIGHT 780

static char *
settings_path(void)
{
    return g_build_filename(
        g_get_user_config_dir(),
        PROFILE_DIR_NAME,
        SETTINGS_FILE_NAME,
        NULL
    );
}

WindowSettings
settings_load_window(void)
{
    WindowSettings settings = { DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT };
    GKeyFile *key_file = g_key_file_new();
    char *path = settings_path();

    if (g_key_file_load_from_file(key_file, path, G_KEY_FILE_NONE, NULL)) {
        settings.width = g_key_file_get_integer(
            key_file,
            WINDOW_GROUP_NAME,
            WINDOW_WIDTH_KEY,
            NULL
        );
        settings.height = g_key_file_get_integer(
            key_file,
            WINDOW_GROUP_NAME,
            WINDOW_HEIGHT_KEY,
            NULL
        );
        if (settings.width <= 0) {
            settings.width = DEFAULT_WINDOW_WIDTH;
        }
        if (settings.height <= 0) {
            settings.height = DEFAULT_WINDOW_HEIGHT;
        }
    }

    g_free(path);
    g_key_file_unref(key_file);
    return settings;
}

void
settings_store_window(int width, int height)
{
    GKeyFile *key_file;
    char *path;
    char *directory;

    if (width <= 0 || height <= 0) {
        return;
    }

    key_file = g_key_file_new();
    path = settings_path();
    directory = g_path_get_dirname(path);

    if (g_mkdir_with_parents(directory, 0700) != 0) {
        g_printerr("Failed to create %s\n", directory);
        g_free(directory);
        g_free(path);
        g_key_file_unref(key_file);
        return;
    }

    g_key_file_set_integer(key_file, WINDOW_GROUP_NAME, WINDOW_WIDTH_KEY, width);
    g_key_file_set_integer(key_file, WINDOW_GROUP_NAME, WINDOW_HEIGHT_KEY, height);

    if (!g_key_file_save_to_file(key_file, path, NULL)) {
        g_printerr("Failed to save %s\n", path);
    }

    g_free(directory);
    g_free(path);
    g_key_file_unref(key_file);
}
