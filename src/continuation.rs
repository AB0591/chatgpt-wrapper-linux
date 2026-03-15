use gtk::prelude::*;
use gtk::{ApplicationWindow, ButtonsType, MessageDialog, MessageType};
use std::time::Duration;
use webkit6::prelude::*;
use webkit6::WebView;

const CHATGPT_URL: &str = "https://chatgpt.com/";
const MAX_CONTEXT_CHARS: usize = 4000;
const EXTRACT_SCRIPT: &str = r#"
(function () {
  function clean(text) {
    return (text || '').replace(/\s+/g, ' ').trim();
  }

  const messages = Array.from(document.querySelectorAll('[data-message-author-role]'))
    .map((node) => {
      const role = clean(node.getAttribute('data-message-author-role')).toUpperCase();
      const text = clean(node.innerText);
      if (!role || !text) {
        return '';
      }
      return role + ': ' + text;
    })
    .filter(Boolean);

  if (messages.length) {
    return messages.slice(-6).join('\n\n').slice(-4000);
  }

  const main = document.querySelector('main');
  if (!main) {
    return '';
  }

  const fallback = clean(main.innerText);
  return fallback.slice(-4000);
})();
"#;
pub fn start(window: &ApplicationWindow, webview: &WebView) {
    extract_snapshot(window, webview);
}

fn extract_snapshot(window: &ApplicationWindow, webview: &WebView) {
    let window = window.clone();
    let webview = webview.clone();

    let webview_for_callback = webview.clone();
    webview.evaluate_javascript(EXTRACT_SCRIPT, None, None, Option::<&gio::Cancellable>::None, move |result| {
        let snapshot = match result {
            Ok(value) => value
                .to_string_as_bytes()
                .and_then(|bytes| String::from_utf8(bytes.as_ref().to_vec()).ok())
                .unwrap_or_default()
                .trim()
                .to_string(),
            Err(error) => {
                show_failure(
                    &window,
                    "Could not read the current conversation.",
                    &format!("WebKit could not extract the recent context: {error}"),
                );
                return;
            }
        };

        if snapshot.is_empty() {
            show_failure(
                &window,
                "Could not read the current conversation.",
                "No recent conversation text was detected on the current ChatGPT page.",
            );
            return;
        }

        let prompt = build_prompt(&snapshot);
        copy_to_clipboard(&window, &prompt);
        request_new_chat(&window, &webview_for_callback, prompt);
    });
}

fn request_new_chat(window: &ApplicationWindow, webview: &WebView, prompt: String) {
    let window = window.clone();
    let webview = webview.clone();

    webview.load_uri(CHATGPT_URL);
    schedule_compose_attempt(&window, &webview, prompt, 20);
}

fn schedule_compose_attempt(
    window: &ApplicationWindow,
    webview: &WebView,
    prompt: String,
    attempts_remaining: u32,
) {
    let window = window.clone();
    let webview = webview.clone();

    glib::timeout_add_local_once(Duration::from_millis(300), move || {
        try_fill_composer(&window, &webview, prompt, attempts_remaining);
    });
}

fn try_fill_composer(
    window: &ApplicationWindow,
    webview: &WebView,
    prompt: String,
    attempts_remaining: u32,
) {
    let script = format!(
        r#"
(function () {{
  const prompt = {prompt};

  function fill(element) {{
    if (!element) {{
      return false;
    }}

    element.focus();

    if (element instanceof HTMLTextAreaElement) {{
      element.value = prompt;
      element.dispatchEvent(new Event('input', {{ bubbles: true }}));
      element.dispatchEvent(new Event('change', {{ bubbles: true }}));
      return true;
    }}

    if (element.isContentEditable) {{
      element.textContent = prompt;
      element.dispatchEvent(new Event('input', {{ bubbles: true }}));
      return true;
    }}

    return false;
  }}

  const candidates = [
    document.querySelector('#prompt-textarea'),
    document.querySelector('textarea'),
    document.querySelector('[contenteditable="true"][role="textbox"]'),
    document.querySelector('[contenteditable="true"][data-testid*="composer"]'),
    document.querySelector('form [contenteditable="true"]')
  ];

  for (const candidate of candidates) {{
    if (fill(candidate)) {{
      return true;
    }}
  }}

  return false;
}})();
"#,
        prompt = js_string_literal(&prompt)
    );

    let window_clone = window.clone();
    let webview_clone = webview.clone();
    let prompt_clone = prompt.clone();

    webview.evaluate_javascript(&script, None, None, Option::<&gio::Cancellable>::None, move |result| {
        let filled = match result {
            Ok(value) => value.to_boolean(),
            Err(error) => {
                eprintln!("Failed to populate the new chat composer: {error}");
                false
            }
        };

        if filled {
            return;
        }

        if attempts_remaining <= 1 {
            show_failure(
                &window_clone,
                "Draft copied to clipboard.",
                "A fresh chat was opened, but the composer could not be populated automatically. Paste the draft manually.",
            );
            return;
        }

        schedule_compose_attempt(
            &window_clone,
            &webview_clone,
            prompt_clone,
            attempts_remaining - 1,
        );
    });
}

fn build_prompt(snapshot: &str) -> String {
    let trimmed = if snapshot.len() > MAX_CONTEXT_CHARS {
        &snapshot[snapshot.len() - MAX_CONTEXT_CHARS..]
    } else {
        snapshot
    };

    format!(
        "Continue this conversation in a fresh chat. Use the previous context below, preserve important constraints and decisions, and continue from where it left off. If something is ambiguous, ask a clarifying question instead of guessing.\n\nPrevious context:\n{trimmed}\n\nContinue from here:"
    )
}

fn copy_to_clipboard(window: &ApplicationWindow, text: &str) {
    let clipboard = gtk::prelude::WidgetExt::display(window).clipboard();
    clipboard.set(&text);
}

fn js_string_literal(text: &str) -> String {
    let escaped = text
        .chars()
        .flat_map(|ch| match ch {
            '\\' => "\\\\".chars().collect::<Vec<_>>(),
            '"' => "\\\"".chars().collect(),
            '\n' => "\\n".chars().collect(),
            '\r' => "\\r".chars().collect(),
            '\t' => "\\t".chars().collect(),
            '\u{08}' => "\\b".chars().collect(),
            '\u{0C}' => "\\f".chars().collect(),
            ch if ch.is_control() => format!("\\u{:04x}", ch as u32).chars().collect(),
            ch => vec![ch],
        })
        .collect::<String>();

    format!("\"{escaped}\"")
}

fn show_failure(window: &ApplicationWindow, message: &str, detail: &str) {
    let dialog = MessageDialog::builder()
        .transient_for(window)
        .modal(true)
        .buttons(ButtonsType::Ok)
        .message_type(MessageType::Info)
        .text(message)
        .secondary_text(detail)
        .build();
    dialog.connect_response(|dialog, _| {
        dialog.close();
    });
    dialog.show();
}
