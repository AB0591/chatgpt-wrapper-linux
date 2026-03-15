use gio::prelude::*;
use gio::ActionEntry;
use gtk::prelude::*;
use gtk::{Application, ApplicationWindow};
use std::sync::mpsc;
use std::time::Duration;

use crate::hotkey;
use crate::window;

const APP_ID: &str = "com.ody.chatgptwrapper";

pub fn run() -> glib::ExitCode {
    let app = Application::builder().application_id(APP_ID).build();

    install_actions(&app);
    install_hotkey_listener(&app);

    app.connect_activate(|app| {
        show_main_window(app);
    });

    app.run()
}

fn install_actions(app: &Application) {
    app.add_action_entries([
        ActionEntry::builder("show-window")
            .activate(|app: &Application, _, _| {
                show_main_window(app);
            })
            .build(),
        ActionEntry::builder("hide-window")
            .activate(|app: &Application, _, _| {
                hide_main_window(app);
            })
            .build(),
        ActionEntry::builder("toggle-window")
            .activate(|app: &Application, _, _| {
                toggle_main_window(app);
            })
            .build(),
    ]);
}

fn install_hotkey_listener(app: &Application) {
    let (sender, receiver) = mpsc::channel();
    if let Err(error) = hotkey::start(sender) {
        eprintln!("Failed to initialize global hotkey: {error}");
        return;
    }

    let app = app.clone();
    glib::timeout_add_local(Duration::from_millis(50), move || {
        while receiver.try_recv().is_ok() {
            app.activate_action("toggle-window", None);
        }

        glib::ControlFlow::Continue
    });
}

fn show_main_window(app: &Application) {
    let window = main_window(app).unwrap_or_else(|| window::build(app));
    window.unminimize();
    window.present();
}

fn hide_main_window(app: &Application) {
    if let Some(window) = main_window(app) {
        window.hide();
    }
}

fn toggle_main_window(app: &Application) {
    let window = main_window(app).unwrap_or_else(|| window::build(app));

    if window.is_visible() && window.is_active() {
        window.hide();
        return;
    }

    window.unminimize();
    window.present();
}

fn main_window(app: &Application) -> Option<ApplicationWindow> {
    app.windows()
        .into_iter()
        .filter_map(|window| window.downcast::<ApplicationWindow>().ok())
        .find(|window| window.transient_for().is_none())
}
