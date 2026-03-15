use std::fs;
use std::path::PathBuf;

const PROFILE_DIR: &str = "chatgpt-wrapper";
const SETTINGS_FILE: &str = "settings.ini";
const WINDOW_GROUP: &str = "window";
const WIDTH_KEY: &str = "width";
const HEIGHT_KEY: &str = "height";
const DEFAULT_WIDTH: i32 = 1100;
const DEFAULT_HEIGHT: i32 = 800;

pub struct WindowSettings {
    pub width: i32,
    pub height: i32,
}

impl WindowSettings {
    pub fn load() -> Self {
        let path = settings_path();
        let key_file = glib::KeyFile::new();

        if key_file
            .load_from_file(&path, glib::KeyFileFlags::NONE)
            .is_ok()
        {
            let width = key_file
                .integer(WINDOW_GROUP, WIDTH_KEY)
                .unwrap_or(DEFAULT_WIDTH);
            let height = key_file
                .integer(WINDOW_GROUP, HEIGHT_KEY)
                .unwrap_or(DEFAULT_HEIGHT);

            return Self { width, height };
        }

        Self {
            width: DEFAULT_WIDTH,
            height: DEFAULT_HEIGHT,
        }
    }

    pub fn store(width: i32, height: i32) {
        let path = settings_path();
        if let Some(parent) = path.parent() {
            if let Err(error) = fs::create_dir_all(parent) {
                eprintln!("Failed to create {}: {error}", parent.display());
                return;
            }
        }

        let key_file = glib::KeyFile::new();
        key_file.set_integer(WINDOW_GROUP, WIDTH_KEY, width);
        key_file.set_integer(WINDOW_GROUP, HEIGHT_KEY, height);

        if let Err(error) = key_file.save_to_file(&path) {
            eprintln!("Failed to save {}: {error}", path.display());
        }
    }
}

fn settings_path() -> PathBuf {
    glib::user_config_dir().join(PROFILE_DIR).join(SETTINGS_FILE)
}
