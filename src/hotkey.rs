use std::ffi::{c_char, c_int, c_long, c_uint, c_ulong, c_void, CString};
use std::ptr;
use std::sync::mpsc::Sender;
use std::thread;

const KEY_PRESS: c_int = 2;
const GRAB_MODE_ASYNC: c_int = 1;
const CONTROL_MASK: c_uint = 1 << 2;
const MOD1_MASK: c_uint = 1 << 3;
const LOCK_MASK: c_uint = 1 << 1;
const MOD2_MASK: c_uint = 1 << 4;

type Display = c_void;
type Window = c_ulong;
type KeySym = c_ulong;
type KeyCode = u8;

#[repr(C)]
union XEvent {
    type_: c_int,
    pad: [c_long; 24],
}

#[link(name = "X11")]
unsafe extern "C" {
    fn XOpenDisplay(display_name: *const c_char) -> *mut Display;
    fn XCloseDisplay(display: *mut Display) -> c_int;
    fn XDefaultRootWindow(display: *mut Display) -> Window;
    fn XStringToKeysym(string: *const c_char) -> KeySym;
    fn XKeysymToKeycode(display: *mut Display, keysym: KeySym) -> KeyCode;
    fn XGrabKey(
        display: *mut Display,
        keycode: c_int,
        modifiers: c_uint,
        grab_window: Window,
        owner_events: c_int,
        pointer_mode: c_int,
        keyboard_mode: c_int,
    ) -> c_int;
    fn XSelectInput(display: *mut Display, window: Window, event_mask: c_long) -> c_int;
    fn XNextEvent(display: *mut Display, event_return: *mut XEvent) -> c_int;
    fn XSync(display: *mut Display, discard: c_int) -> c_int;
}

pub fn start(toggle_sender: Sender<()>) -> Result<(), String> {
    let hotkey = CString::new("space").map_err(|error| error.to_string())?;

    thread::spawn(move || {
        if let Err(error) = run_listener(hotkey, toggle_sender) {
            eprintln!("Failed to start global hotkey: {error}");
        }
    });

    Ok(())
}

fn run_listener(hotkey: CString, toggle_sender: Sender<()>) -> Result<(), String> {
    let display = unsafe { XOpenDisplay(ptr::null()) };
    if display.is_null() {
        return Err("XOpenDisplay returned null".to_string());
    }

    let result = listen(display, &hotkey, toggle_sender);
    unsafe {
        XCloseDisplay(display);
    }
    result
}

fn listen(display: *mut Display, hotkey: &CString, toggle_sender: Sender<()>) -> Result<(), String> {
    let keysym = unsafe { XStringToKeysym(hotkey.as_ptr()) };
    if keysym == 0 {
        return Err("XStringToKeysym could not resolve the hotkey keysym".to_string());
    }

    let keycode = unsafe { XKeysymToKeycode(display, keysym) };
    if keycode == 0 {
        return Err("XKeysymToKeycode returned an invalid keycode".to_string());
    }

    let root = unsafe { XDefaultRootWindow(display) };
    for modifiers in modifier_variants(CONTROL_MASK | MOD1_MASK) {
        unsafe {
            XGrabKey(
                display,
                c_int::from(keycode),
                modifiers,
                root,
                0,
                GRAB_MODE_ASYNC,
                GRAB_MODE_ASYNC,
            );
        }
    }

    unsafe {
        XSelectInput(display, root, 0);
        XSync(display, 0);
    }

    loop {
        let mut event = XEvent { type_: 0 };
        unsafe {
            XNextEvent(display, &mut event);
        }

        if unsafe { event.type_ } == KEY_PRESS && toggle_sender.send(()).is_err() {
            return Ok(());
        }
    }
}

fn modifier_variants(base: c_uint) -> [c_uint; 4] {
    [base, base | LOCK_MASK, base | MOD2_MASK, base | LOCK_MASK | MOD2_MASK]
}
