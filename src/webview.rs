use std::fs;
use std::path::PathBuf;
use std::{env, sync::OnceLock};

use webkit6::prelude::*;
use webkit6::{
    CookieAcceptPolicy, CookiePersistentStorage, LoadEvent, NetworkSession, Settings, WebView,
};

const CHATGPT_URL: &str = "https://chatgpt.com";
const PROFILE_DIR: &str = "chatgpt-wrapper";
const DATA_DIR: &str = "data";
const CACHE_DIR: &str = "cache";
const COOKIE_FILE: &str = "cookies.sqlite";

pub fn build() -> WebView {
    let network_session = build_network_session();
    let settings = build_settings();
    let webview = WebView::builder()
        .network_session(&network_session)
        .settings(&settings)
        .hexpand(true)
        .vexpand(true)
        .focusable(true)
        .build();

    configure(&webview);
    webview.load_uri(CHATGPT_URL);
    webview
}

pub fn build_related(related_view: &WebView) -> WebView {
    let mut builder = WebView::builder()
        .related_view(related_view)
        .hexpand(true)
        .vexpand(true)
        .focusable(true);

    if let Some(network_session) = related_view.network_session() {
        builder = builder.network_session(&network_session);
    }

    if let Some(settings) = webkit6::prelude::WebViewExt::settings(related_view) {
        builder = builder.settings(&settings);
    }

    let webview = builder.build();
    configure(&webview);
    webview
}

fn build_network_session() -> NetworkSession {
    let data_directory = profile_dir(glib::user_data_dir(), DATA_DIR);
    let cache_directory = profile_dir(glib::user_cache_dir(), CACHE_DIR);
    let cookie_path = data_directory.join(COOKIE_FILE);

    ensure_dir(&data_directory);
    ensure_dir(&cache_directory);

    let session = NetworkSession::new(
        Some(data_directory.to_string_lossy().as_ref()),
        Some(cache_directory.to_string_lossy().as_ref()),
    );
    session.set_persistent_credential_storage_enabled(true);
    if let Some(cookie_manager) = session.cookie_manager() {
        cookie_manager.set_accept_policy(CookieAcceptPolicy::Always);
        cookie_manager.set_persistent_storage(
            cookie_path.to_string_lossy().as_ref(),
            CookiePersistentStorage::Sqlite,
        );
    }
    session
}

fn build_settings() -> Settings {
    let settings = Settings::new();
    settings.set_enable_developer_extras(false);
    settings.set_enable_html5_local_storage(true);
    settings.set_enable_javascript(true);
    settings.set_enable_media(true);
    settings.set_enable_media_stream(true);
    settings.set_enable_mediasource(true);
    settings.set_enable_page_cache(true);
    settings.set_enable_smooth_scrolling(true);
    settings.set_enable_webaudio(true);
    settings.set_enable_webgl(true);
    settings
}

fn configure(webview: &WebView) {
    webview.connect_load_changed(|webview, load_event| {
        if debug_webkit_logging() {
            log_load_event(webview, load_event);
        }
    });

    webview.connect_load_failed(|_, _, failing_uri, error| {
        eprintln!("Failed to load {failing_uri}: {error}");
        false
    });
}

fn debug_webkit_logging() -> bool {
    static ENABLED: OnceLock<bool> = OnceLock::new();

    *ENABLED.get_or_init(|| {
        env::var("CHATGPT_WRAPPER_DEBUG_WEBKIT")
            .map(|value| value == "1")
            .unwrap_or(false)
    })
}

fn log_load_event(webview: &WebView, load_event: LoadEvent) {
    let uri = webview
        .uri()
        .map(|uri| uri.to_string())
        .unwrap_or_else(|| "<no uri>".to_string());

    eprintln!(
        "WebView load event: {load_event:?}, uri: {uri}, progress: {:.0}%",
        webview.estimated_load_progress() * 100.0
    );
}

fn profile_dir(base_dir: PathBuf, leaf: &str) -> PathBuf {
    base_dir.join(PROFILE_DIR).join(leaf)
}

fn ensure_dir(path: &PathBuf) {
    if let Err(error) = fs::create_dir_all(path) {
        eprintln!("Failed to create {}: {error}", path.display());
    }
}
