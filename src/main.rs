mod app;
mod downloads;
mod hotkey;
mod webview;
mod window;

fn main() -> glib::ExitCode {
    app::run()
}
