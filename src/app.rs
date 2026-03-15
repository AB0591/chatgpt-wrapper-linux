use gtk::prelude::*;
use gtk::Application;

use crate::window;

const APP_ID: &str = "com.ody.chatgptwrapper";

pub fn run() -> glib::ExitCode {
    let app = Application::builder().application_id(APP_ID).build();

    app.connect_activate(|app| {
        let window = window::build(app);
        window.present();
    });

    app.run()
}
