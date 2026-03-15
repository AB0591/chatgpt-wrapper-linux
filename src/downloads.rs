use gio::prelude::*;
use gtk::prelude::*;
use gtk::{ApplicationWindow, FileChooserAction, FileChooserNative, ResponseType};
use webkit6::{Download, NetworkSession};

pub fn install(network_session: &NetworkSession) {
    network_session.connect_download_started(|_, download| {
        configure(download);
    });
}

fn configure(download: &Download) {
    download.set_allow_overwrite(true);

    download.connect_decide_destination(|download, suggested_filename| {
        prompt_for_destination(download, suggested_filename);
        true
    });

    download.connect_failed(|download, error| {
        let source = download.request().and_then(|request| request.uri());
        if let Some(source) = source {
            eprintln!("Download failed for {source}: {error}");
        } else {
            eprintln!("Download failed: {error}");
        }
    });

    download.connect_finished(|download| {
        if let Some(destination) = download.destination() {
            eprintln!("Download finished: {destination}");
        }
    });
}

fn prompt_for_destination(download: &Download, suggested_filename: &str) {
    let dialog = FileChooserNative::new(
        Some("Save Download"),
        parent_window(download).as_ref(),
        FileChooserAction::Save,
        Some("Save"),
        Some("Cancel"),
    );
    dialog.set_modal(true);
    dialog.set_current_name(suggested_filename);

    if let Some(downloads_dir) = glib::user_special_dir(glib::UserDirectory::Downloads) {
        let folder = gio::File::for_path(downloads_dir);
        let _ = dialog.set_file(&folder);
    }

    let download = download.clone();
    dialog.run_async(move |dialog, response| {
        handle_destination_response(dialog, response, &download);
        dialog.destroy();
    });
}

fn handle_destination_response(
    dialog: &FileChooserNative,
    response: ResponseType,
    download: &Download,
) {
    if response != ResponseType::Accept {
        download.cancel();
        return;
    }

    let Some(file) = dialog.file() else {
        download.cancel();
        return;
    };

    let Some(path) = file.path() else {
        eprintln!("Download canceled: selected destination is not a local filesystem path");
        download.cancel();
        return;
    };

    download.set_destination(path.to_string_lossy().as_ref());
}

fn parent_window(download: &Download) -> Option<ApplicationWindow> {
    download
        .web_view()
        .and_then(|web_view| web_view.root())
        .and_then(|root| root.downcast::<ApplicationWindow>().ok())
}
