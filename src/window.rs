use gtk::prelude::*;
use gtk::{Application, ApplicationWindow};
use webkit6::prelude::*;
use webkit6::WebView;

use crate::webview;

pub fn build(app: &Application) -> ApplicationWindow {
    let webview = webview::build();
    let window = ApplicationWindow::builder()
        .application(app)
        .title("ChatGPT")
        .default_width(1100)
        .default_height(800)
        .child(&webview)
        .build();

    bind_window_title(&window, &webview);
    bind_popup_windows(app, &window, &webview);

    window
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

fn sync_window_title(window: &ApplicationWindow, webview: &WebView) {
    let title = webview
        .title()
        .filter(|title| !title.is_empty())
        .map(|title| title.to_string())
        .unwrap_or_else(|| "ChatGPT".to_string());

    window.set_title(Some(&title));
}
