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
