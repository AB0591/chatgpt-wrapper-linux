mod app;
mod downloads;
mod hotkey;
mod settings;
mod webview;
mod window;

fn main() -> glib::ExitCode {
    app::run()
}
