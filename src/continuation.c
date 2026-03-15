#include "continuation.h"

#include <jsc/jsc.h>

#define CHATGPT_URL "https://chatgpt.com/"
#define MAX_CONTEXT_CHARS 4000
#define COMPOSE_ATTEMPTS 20

static const char *EXTRACT_SCRIPT =
    "(function () {"
    "  function clean(text) { return (text || '').replace(/\\s+/g, ' ').trim(); }"
    "  const messages = Array.from(document.querySelectorAll('[data-message-author-role]'))"
    "    .map((node) => {"
    "      const role = clean(node.getAttribute('data-message-author-role')).toUpperCase();"
    "      const text = clean(node.innerText);"
    "      if (!role || !text) return '';"
    "      return role + ': ' + text;"
    "    })"
    "    .filter(Boolean);"
    "  if (messages.length) return messages.slice(-6).join('\\n\\n').slice(-4000);"
    "  const main = document.querySelector('main');"
    "  if (!main) return '';"
    "  return clean(main.innerText).slice(-4000);"
    "})();";

typedef struct {
    GtkWindow *window;
    WebKitWebView *web_view;
    char *prompt;
    guint attempts_remaining;
} ComposeContext;

static char *
js_string_literal(const char *text)
{
    GString *escaped = g_string_new("\"");

    for (const char *p = text; *p != '\0'; p++) {
        switch (*p) {
        case '\\': g_string_append(escaped, "\\\\"); break;
        case '"': g_string_append(escaped, "\\\""); break;
        case '\n': g_string_append(escaped, "\\n"); break;
        case '\r': g_string_append(escaped, "\\r"); break;
        case '\t': g_string_append(escaped, "\\t"); break;
        default:
            g_string_append_c(escaped, *p);
            break;
        }
    }

    g_string_append_c(escaped, '"');
    return g_string_free(escaped, FALSE);
}

static void
show_message(GtkWindow *window, const char *message, const char *detail)
{
    GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", message);

    gtk_alert_dialog_set_modal(dialog, TRUE);
    gtk_alert_dialog_set_detail(dialog, detail);
    gtk_alert_dialog_show(dialog, window);
    g_object_unref(dialog);
}

static char *
build_prompt(const char *snapshot)
{
    const char *trimmed = snapshot;

    if (strlen(snapshot) > MAX_CONTEXT_CHARS) {
        trimmed = snapshot + strlen(snapshot) - MAX_CONTEXT_CHARS;
    }

    return g_strdup_printf(
        "Continue this conversation in a fresh chat. Use the previous context below, preserve important constraints and decisions, and continue from where it left off. If something is ambiguous, ask a clarifying question instead of guessing.\n\nPrevious context:\n%s\n\nContinue from here:",
        trimmed
    );
}

static void
copy_to_clipboard(GtkWindow *window, const char *text)
{
    GdkClipboard *clipboard = gtk_widget_get_clipboard(GTK_WIDGET(window));
    gdk_clipboard_set_text(clipboard, text);
}

static void schedule_compose_attempt(ComposeContext *context);

static void
finish_compose_context(ComposeContext *context)
{
    g_object_unref(context->window);
    g_object_unref(context->web_view);
    g_free(context->prompt);
    g_free(context);
}

static void
on_fill_composer_finished(GObject *source_object, GAsyncResult *result, gpointer user_data)
{
    ComposeContext *context = user_data;
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(source_object);
    GError *error = NULL;
    JSCValue *value = webkit_web_view_evaluate_javascript_finish(web_view, result, &error);
    gboolean filled = FALSE;

    if (error == NULL && value != NULL && jsc_value_is_boolean(value)) {
        filled = jsc_value_to_boolean(value);
    }

    if (value != NULL) {
        g_object_unref(value);
    }

    if (error != NULL) {
        g_error_free(error);
    }

    if (filled) {
        finish_compose_context(context);
        return;
    }

    if (context->attempts_remaining <= 1) {
        show_message(
            context->window,
            "Draft copied to clipboard.",
            "A fresh chat was opened, but the composer could not be populated automatically. Paste the draft manually."
        );
        finish_compose_context(context);
        return;
    }

    context->attempts_remaining--;
    schedule_compose_attempt(context);
}

static gboolean
try_fill_composer(gpointer user_data)
{
    ComposeContext *context = user_data;
    char *prompt = js_string_literal(context->prompt);
    char *script = g_strdup_printf(
        "(function () {"
        "  const prompt = %s;"
        "  function dispatchInput(element) {"
        "    element.dispatchEvent(new InputEvent('input', { bubbles: true, data: prompt, inputType: 'insertText' }));"
        "    element.dispatchEvent(new Event('change', { bubbles: true }));"
        "  }"
        "  function fill(element) {"
        "    if (!element) return false;"
        "    element.focus();"
        "    if (element instanceof HTMLTextAreaElement) {"
        "      const prototype = window.HTMLTextAreaElement.prototype;"
        "      const descriptor = Object.getOwnPropertyDescriptor(prototype, 'value');"
        "      if (descriptor && descriptor.set) {"
        "        descriptor.set.call(element, prompt);"
        "      } else {"
        "        element.value = prompt;"
        "      }"
        "      dispatchInput(element);"
        "      return true;"
        "    }"
        "    if (element.isContentEditable) {"
        "      element.textContent = '';"
        "      const lines = prompt.split('\\n');"
        "      const fragment = document.createDocumentFragment();"
        "      lines.forEach((line, index) => {"
        "        if (index > 0) fragment.appendChild(document.createElement('br'));"
        "        fragment.appendChild(document.createTextNode(line));"
        "      });"
        "      element.replaceChildren(fragment);"
        "      dispatchInput(element);"
        "      return true;"
        "    }"
        "    return false;"
        "  }"
        "  const candidates = ["
        "    document.querySelector('#prompt-textarea'),"
        "    document.querySelector('textarea'),"
        "    document.querySelector('[data-testid=\"composer-text-input\"] textarea'),"
        "    document.querySelector('[data-testid=\"composer-text-input\"] [contenteditable=\"true\"]'),"
        "    document.querySelector('[data-testid*=\"composer\"] textarea'),"
        "    document.querySelector('[contenteditable=\"true\"][role=\"textbox\"]'),"
        "    document.querySelector('[contenteditable=\"true\"][data-testid*=\"composer\"]'),"
        "    document.querySelector('form [contenteditable=\"true\"]'),"
        "    document.querySelector('main [contenteditable=\"true\"]')"
        "  ];"
        "  for (const candidate of candidates) { if (fill(candidate)) return true; }"
        "  return false;"
        "})();",
        prompt
    );

    webkit_web_view_evaluate_javascript(
        context->web_view,
        script,
        -1,
        NULL,
        NULL,
        NULL,
        on_fill_composer_finished,
        context
    );

    g_free(script);
    g_free(prompt);
    return G_SOURCE_REMOVE;
}

static void
schedule_compose_attempt(ComposeContext *context)
{
    g_timeout_add(300, try_fill_composer, context);
}

static void
open_new_chat(GtkWindow *window, WebKitWebView *web_view, const char *prompt)
{
    ComposeContext *context = g_new0(ComposeContext, 1);

    context->window = g_object_ref(window);
    context->web_view = g_object_ref(web_view);
    context->prompt = g_strdup(prompt);
    context->attempts_remaining = COMPOSE_ATTEMPTS;

    webkit_web_view_load_uri(web_view, CHATGPT_URL);
    schedule_compose_attempt(context);
}

static void
on_extract_snapshot_finished(GObject *source_object, GAsyncResult *result, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(source_object);
    GError *error = NULL;
    JSCValue *value = webkit_web_view_evaluate_javascript_finish(web_view, result, &error);
    char *snapshot = NULL;

    if (error != NULL) {
        show_message(window, "Could not read the current conversation.", error->message);
        g_error_free(error);
        g_object_unref(window);
        return;
    }

    if (value != NULL && jsc_value_is_string(value)) {
        snapshot = jsc_value_to_string(value);
    }

    if (value != NULL) {
        g_object_unref(value);
    }

    if (snapshot == NULL || snapshot[0] == '\0') {
        show_message(
            window,
            "Could not read the current conversation.",
            "No recent conversation text was detected on the current ChatGPT page."
        );
        g_free(snapshot);
        g_object_unref(window);
        return;
    }

    char *prompt = build_prompt(snapshot);
    copy_to_clipboard(window, prompt);
    open_new_chat(window, web_view, prompt);

    g_free(prompt);
    g_free(snapshot);
    g_object_unref(window);
}

void
continuation_start(GtkWindow *window, WebKitWebView *web_view)
{
    webkit_web_view_evaluate_javascript(
        web_view,
        EXTRACT_SCRIPT,
        -1,
        NULL,
        NULL,
        NULL,
        on_extract_snapshot_finished,
        g_object_ref(window)
    );
}
