You are working in this repository:

/Users/arlanbrown/development/chatgpt-wrapper-linux

This project is a Linux port of my existing ChatGPT wrapper app.

## Target environment

- Ubuntu 25.10 base
- Lubuntu flavor
- LXQt desktop session
- Primary test environment is Lubuntu/LXQt, not GNOME

## Product goal

Build a small, auditable desktop wrapper for ChatGPT that feels native enough on Lubuntu and remains maintainable by one person.

The app should eventually support:

- embedded ChatGPT web UI
- global hotkey to show/focus/hide the app window
- minimal native window controls
- downloads
- a “New Chat with Context” continuation helper

## Project principles

Keep the project:

- minimal
- easy to audit
- easy to understand
- conservative in scope
- low dependency
- low magic
- maintainable by one person

Prefer:

- small files
- clear names
- straightforward control flow
- comments only where useful
- simple state management
- additive, incremental changes

Avoid:

- unnecessary abstraction
- heavy frameworks unless clearly justified
- complicated async pipelines unless clearly needed
- invasive browser automation
- trying to outsmart the ChatGPT web app

## Platform guidance

- Prefer native Linux desktop APIs where practical
- Prefer `WebKitGTK` for the embedded web content unless feasibility proves it unsuitable
- Do not assume GNOME-specific UI or libadwaita conventions
- Optimize for Lubuntu/LXQt compatibility first
- Validate hotkey and window-focus behavior specifically under LXQt

## Security and scope constraints

- Do not add telemetry or analytics
- Do not intercept credentials
- Do not manipulate ChatGPT authentication flows
- Do not build a custom browser engine
- Do not add auto-update in v1
- Avoid invasive DOM automation unless a milestone explicitly requires a small, auditable amount

## Working style

1. Read `README.md`, `spec.md`, `architecture.md`, `TASKS.md`, and `AGENTS.md` first.
2. Inspect the existing repo structure before proposing code changes.
3. Keep changes small and milestone-based.
4. Explain the plan briefly before editing files.
5. After edits, summarize:
   - files added
   - files changed
   - design choices
   - assumptions
   - fragile points
6. Stop after each milestone is complete.

## Current priority

Start with Milestone 1 only.

Milestone 1 goals:
- choose the Linux UI/runtime stack
- confirm `WebKitGTK` viability for ChatGPT
- create a minimal app shell
- confirm the app builds and runs locally

## Implementation preference for Milestone 1

Bias toward the smallest realistic Linux-native approach.

If a stack decision is needed, prefer the option that:
- works cleanly on Lubuntu/LXQt
- stays lightweight
- is easier to audit than Chromium-based approaches
- does not create unnecessary cross-platform abstraction

Before making large changes, explain the chosen Linux stack and why it fits this repo.
