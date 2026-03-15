use gtk::{Application, ApplicationWindow};

use crate::webview;

pub fn build(app: &Application) -> ApplicationWindow {
    let webview = webview::build();

    ApplicationWindow::builder()
        .application(app)
        .title("ChatGPT")
        .default_width(1100)
        .default_height(800)
        .child(&webview)
        .build()
}
