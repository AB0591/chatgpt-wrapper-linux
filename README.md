# ChatGPT Wrapper for Lubuntu

A small, auditable desktop wrapper for ChatGPT targeting Lubuntu on Ubuntu 25.10 with the LXQt desktop session.

This branch is the plain C implementation track. It does not require Rust to build or run.

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

## Current implementation status

- Milestone 1 complete with a plain C `GTK4` + `WebKitGTK` app shell
- Milestone 2 complete with ChatGPT loading in the main content area and persistent login storage
- Milestone 3 complete with app-level show, hide, and toggle window actions
- WebKit profile data, cookies, and credential storage persist under the normal user data/cache directories
- No Rust toolchain, Cargo project, or Rust runtime dependency
- Local compilation verified with `make`, and the app has been run successfully under LXQt

## Build requirements

- `build-essential`
- `pkg-config`
- `libgtk-4-dev`
- `libwebkitgtk-6.0-dev`

## Build and run

```bash
make
make run
```

## Profile storage

- Data: `~/.local/share/chatgpt-wrapper-c/`
- Cache: `~/.cache/chatgpt-wrapper-c/`

This is where the embedded WebKit session stores cookies and related site data so you do not need to log in on every launch.

## Repo docs

- `spec.md`: product requirements and acceptance criteria
- `architecture.md`: proposed Linux architecture and technology choices
- `TASKS.md`: milestone-based implementation checklist
- `AGENTS.md`: repo instructions for future coding sessions
