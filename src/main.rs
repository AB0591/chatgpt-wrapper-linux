mod app;
mod hotkey;
mod webview;
mod window;

fn main() -> glib::ExitCode {
    app::run()
}
