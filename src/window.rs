use gtk::prelude::*;
use gtk::{Application, ApplicationWindow, Box as GtkBox, Button, HeaderBar, Orientation};
use webkit6::prelude::*;
use webkit6::WebView;

use crate::continuation;
use crate::settings::WindowSettings;
use crate::webview;

pub fn build(app: &Application) -> ApplicationWindow {
    let webview = webview::build();
    let stored = WindowSettings::load();
    let content = GtkBox::builder()
        .orientation(Orientation::Vertical)
        .hexpand(true)
        .vexpand(true)
        .build();
    content.append(&webview);
    let window = ApplicationWindow::builder()
        .application(app)
        .title("ChatGPT")
        .default_width(stored.width)
        .default_height(stored.height)
        .child(&content)
        .build();

    install_header_bar(&window, &webview);
    bind_window_size(&window);
    bind_window_title(&window, &webview);
    bind_popup_windows(app, &window, &webview);

    window
}

fn install_header_bar(window: &ApplicationWindow, webview: &WebView) {
    let header = HeaderBar::builder().show_title_buttons(true).build();

    let back = Button::builder().label("Back").build();
    let forward = Button::builder().label("Forward").build();
    let reload = Button::builder().label("Reload").build();
    let continue_chat = Button::builder()
        .label("New Chat with Context")
        .tooltip_text("Open a fresh chat and prepare a continuation draft")
        .build();

    header.pack_start(&back);
    header.pack_start(&forward);
    header.pack_end(&continue_chat);
    header.pack_end(&reload);
    window.set_titlebar(Some(&header));

    let webview_for_back = webview.clone();
    back.connect_clicked(move |_| {
        if webview_for_back.can_go_back() {
            webview_for_back.go_back();
        }
    });

    let webview_for_forward = webview.clone();
    forward.connect_clicked(move |_| {
        if webview_for_forward.can_go_forward() {
            webview_for_forward.go_forward();
        }
    });

    let webview_for_reload = webview.clone();
    reload.connect_clicked(move |_| {
        webview_for_reload.reload();
    });

    let window_for_continue = window.clone();
    let webview_for_continue = webview.clone();
    continue_chat.connect_clicked(move |_| {
        continuation::start(&window_for_continue, &webview_for_continue);
    });

    sync_navigation_buttons(&back, &forward, webview);

    let back_for_load = back.clone();
    let forward_for_load = forward.clone();
    let webview_for_load = webview.clone();
    webview.connect_load_changed(move |_, _| {
        sync_navigation_buttons(&back_for_load, &forward_for_load, &webview_for_load);
    });

    let back_for_uri = back.clone();
    let forward_for_uri = forward.clone();
    let webview_for_uri = webview.clone();
    webview.connect_uri_notify(move |_| {
        sync_navigation_buttons(&back_for_uri, &forward_for_uri, &webview_for_uri);
    });
}

fn bind_window_title(window: &ApplicationWindow, webview: &WebView) {
    sync_window_title(window, webview);

    let window_for_title = window.clone();
    let webview_for_title = webview.clone();
    webview.connect_title_notify(move |_| {
        sync_window_title(&window_for_title, &webview_for_title);
    });

    let window_for_load = window.clone();
    let webview_for_load = webview.clone();
    webview.connect_load_changed(move |_, _| {
        sync_window_title(&window_for_load, &webview_for_load);
    });
}

fn bind_window_size(window: &ApplicationWindow) {
    let window_for_width = window.clone();
    window.connect_default_width_notify(move |_| {
        persist_window_size(&window_for_width);
    });

    let window_for_height = window.clone();
    window.connect_default_height_notify(move |_| {
        persist_window_size(&window_for_height);
    });
}

fn bind_popup_windows(app: &Application, parent: &ApplicationWindow, webview: &WebView) {
    let app = app.clone();
    let parent = parent.clone();

    webview.connect_create(move |related_view, _| {
        let popup_webview = webview::build_related(related_view);
        let popup_window = ApplicationWindow::builder()
            .application(&app)
            .title("ChatGPT")
            .default_width(900)
            .default_height(700)
            .transient_for(&parent)
            .child(&popup_webview)
            .build();

        bind_window_title(&popup_window, &popup_webview);

        let popup_window_for_close = popup_window.clone();
        popup_webview.connect_close(move |_| {
            popup_window_for_close.close();
        });

        popup_window.present();
        popup_webview.upcast()
    });
}

fn persist_window_size(window: &ApplicationWindow) {
    let width = window.default_width();
    let height = window.default_height();

    if width > 0 && height > 0 {
        WindowSettings::store(width, height);
    }
}

fn sync_navigation_buttons(back: &Button, forward: &Button, webview: &WebView) {
    back.set_sensitive(webview.can_go_back());
    forward.set_sensitive(webview.can_go_forward());
}

fn sync_window_title(window: &ApplicationWindow, webview: &WebView) {
    let title = webview
        .title()
        .filter(|title| !title.is_empty())
        .map(|title| title.to_string())
        .unwrap_or_else(|| "ChatGPT".to_string());

    window.set_title(Some(&title));
}
