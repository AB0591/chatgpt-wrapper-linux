# ChatGPT Wrapper for Lubuntu

A small, auditable desktop wrapper for ChatGPT targeting Lubuntu on Ubuntu 25.10 with the LXQt desktop session.

## Goals

- Native-feeling Linux desktop app
- Global hotkey to show, focus, or hide the app window
- Embedded ChatGPT web UI
- Small, understandable codebase
- Minimal dependencies
- No telemetry, analytics, or auto-update
- No credential interception
- No broad DOM hacking

## Target environment

- Ubuntu 25.10 base
- Lubuntu flavor
- `DESKTOP_SESSION=Lubuntu`
- LXQt desktop environment

## Development philosophy

- Prefer native Linux desktop APIs where practical
- Keep the first version narrow and stable
- Make each change easy to audit
- Stop after each milestone for review

## Preferred web layer

- `WebKitGTK` is the default choice for embedding ChatGPT on Linux

## Repo docs

- `spec.md`: product requirements and acceptance criteria
- `architecture.md`: proposed Linux architecture and technology choices
- `TASKS.md`: milestone-based implementation checklist
- `AGENTS.md`: repo instructions for future coding sessions

## Milestone 1 stack

- Rust from Ubuntu packages via `apt`
- GTK4 for the app shell
- WebKitGTK 6.0 for embedded web content
- No libadwaita, Electron, Tauri, or custom browser logic

## Build dependencies

Install the required toolchain and native development packages with `apt`:

```bash
sudo apt install build-essential cargo rustc pkg-config libgtk-4-dev libwebkitgtk-6.0-dev
```

This project intentionally uses distro-managed Rust so the compiler and GTK/WebKit system libraries stay aligned with normal `apt upgrade` maintenance.
The crate versions are pinned to exact GTK/WebKit Rust releases that stay within the `rustc 1.85.1` compatibility window used by Ubuntu 25.10.

## Build and run

```bash
cargo build
cargo run
```

## Current status

- Milestone 2 adds a dedicated `WebKitGTK` wrapper with persistent browser data and popup window support for web-driven auth/navigation flows
- The app opens ChatGPT in the main content area with JavaScript, local storage, media, cache, and persistent cookie storage enabled
- Milestone 3 adds app-level show/hide/toggle actions for later hotkey wiring while keeping normal close behavior intact until a real reactivation surface exists
- Milestone 4 registers a global X11 hotkey on Lubuntu/LXQt and wires it to the existing window toggle action
- Milestone 5 adds native download handling with a user-selected save location through GTK's file chooser
- Milestone 6 adds remembered window size and a small native header bar with back, forward, and reload controls
- Navigation and login still require manual validation in an interactive LXQt session

## Current limitations

- The global hotkey currently targets X11 sessions and defaults to `Ctrl+Alt+Space`
- The global hotkey is not configurable yet
- Downloads currently rely on GTK's native save dialog and do not yet expose in-app progress UI
- Window size is restored, but window position is not currently persisted
- No tray/menu integration yet
- No DOM automation or credential interception
- WebKit may emit internal load errors on stderr even when ChatGPT still renders and works; failed loads are logged by default, and verbose load-state logging can be enabled with `CHATGPT_WRAPPER_DEBUG_WEBKIT=1 cargo run`

## Known caveats

- This app currently targets Lubuntu/LXQt on X11 first; the global hotkey is not implemented for Wayland sessions
- ChatGPT behavior still depends on WebKitGTK compatibility with the live site, so occasional site-specific rendering or login quirks may need follow-up
- Window size persistence is implemented conservatively; window position restore is intentionally not forced because it is less reliable across Linux desktop environments and window managers
