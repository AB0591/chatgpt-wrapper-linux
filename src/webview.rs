use webkit6::prelude::*;
use webkit6::WebView;

const CHATGPT_URL: &str = "https://chatgpt.com";

pub fn build() -> WebView {
    let webview = WebView::new();
    webview.load_uri(CHATGPT_URL);
    webview
}
