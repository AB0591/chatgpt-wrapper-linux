#include "hotkey.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <glib.h>

#define CONTROL_MASK (1U << 2)
#define MOD1_MASK (1U << 3)
#define LOCK_MASK (1U << 1)
#define MOD2_MASK (1U << 4)

typedef struct {
    GtkApplication *app;
    GAsyncQueue *events;
} HotkeyContext;

static guint
modifier_variant(guint base, guint index)
{
    switch (index) {
    case 0:
        return base;
    case 1:
        return base | LOCK_MASK;
    case 2:
        return base | MOD2_MASK;
    default:
        return base | LOCK_MASK | MOD2_MASK;
    }
}

static gboolean
dispatch_hotkey_event(gpointer user_data)
{
    HotkeyContext *context = user_data;

    while (g_async_queue_try_pop(context->events) != NULL) {
        g_action_group_activate_action(
            G_ACTION_GROUP(context->app),
            "toggle-window",
            NULL
        );
    }

    return G_SOURCE_CONTINUE;
}

static gpointer
hotkey_listener_thread(gpointer user_data)
{
    HotkeyContext *context = user_data;
    Display *display = XOpenDisplay(NULL);
    KeySym keysym;
    KeyCode keycode;
    Window root;
    guint base_modifiers = CONTROL_MASK | MOD1_MASK;
    int i;

    if (display == NULL) {
        g_printerr("Failed to initialize global hotkey: XOpenDisplay returned null\n");
        return NULL;
    }

    keysym = XStringToKeysym("space");
    if (keysym == NoSymbol) {
        g_printerr("Failed to initialize global hotkey: could not resolve space keysym\n");
        XCloseDisplay(display);
        return NULL;
    }

    keycode = XKeysymToKeycode(display, keysym);
    if (keycode == 0) {
        g_printerr("Failed to initialize global hotkey: invalid keycode for space\n");
        XCloseDisplay(display);
        return NULL;
    }

    root = DefaultRootWindow(display);
    for (i = 0; i < 4; i++) {
        XGrabKey(
            display,
            keycode,
            modifier_variant(base_modifiers, i),
            root,
            False,
            GrabModeAsync,
            GrabModeAsync
        );
    }

    XSelectInput(display, root, 0);
    XSync(display, False);

    while (1) {
        XEvent event;

        XNextEvent(display, &event);
        if (event.type == KeyPress) {
            g_async_queue_push(context->events, GINT_TO_POINTER(1));
        }
    }

    XCloseDisplay(display);
    return NULL;
}

void
hotkey_start(GtkApplication *app)
{
    HotkeyContext *context = g_new0(HotkeyContext, 1);

    context->app = g_object_ref(app);
    context->events = g_async_queue_new();

    g_timeout_add(50, dispatch_hotkey_event, context);
    g_thread_new("global-hotkey", hotkey_listener_thread, context);
}
