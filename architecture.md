# Architecture

## Overview

The Linux app should keep the same high-level shape as the macOS version while using Linux-appropriate building blocks:

1. App lifecycle
2. Window management
3. Embedded web view
4. Global hotkey handling

## Technology choices

### Linux-native desktop approach
Prefer a lightweight Linux desktop stack that works cleanly on Lubuntu/LXQt and stays easy to audit.

The UI toolkit should remain open until feasibility is validated, but the chosen stack should:
- feel native enough on LXQt
- stay small and maintainable
- avoid heavy multi-layer abstractions when possible

### Embedded web view
Use `WebKitGTK` as the preferred embedded web layer.
Reasons:
- native Linux web view option
- much smaller than bundling Chromium
- easier to audit than a browser-like framework
- aligned with the narrow wrapper goal

### Window management
Use the desktop toolkit and windowing APIs directly for:
- show/hide behavior
- focus and raise behavior
- remembered window geometry where practical

Validate these behaviors specifically under LXQt, since Linux desktop behavior can vary by session and window manager.

### Hotkey implementation
Prefer the smallest reliable implementation that works under Lubuntu/LXQt.

This should be validated early because global hotkey behavior can differ across Linux desktop sessions.

## Proposed file structure

- `app/`
- `ui/`
- `services/`
- `web/`
- `README.md`
- `spec.md`
- `architecture.md`
- `TASKS.md`

The exact source layout can be adjusted once the Linux stack is chosen, but the code should still separate:
- app lifecycle
- window and hotkey services
- web embedding
- lightweight UI

## Authentication model

The app does not manage login directly.
The user signs in through the ChatGPT website loaded inside the embedded web view.
