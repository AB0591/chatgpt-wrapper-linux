# Product Spec

## Product name
ChatGPT Wrapper for Lubuntu

## Objective
Build a simple desktop application for Lubuntu/LXQt that wraps the ChatGPT web experience in a focused app window and can be summoned with a global hotkey.

## Primary user story
As a Lubuntu user,
I want to press a global shortcut from anywhere,
so I can instantly bring up ChatGPT in a focused desktop window.

## Core requirements

1. The app must target Lubuntu on Ubuntu 25.10 with LXQt as the primary environment.
2. The web content should be embedded using `WebKitGTK` unless later feasibility testing proves it unsuitable.
3. The app must support a global hotkey that works when the app is not focused.
4. Pressing the hotkey should:
   - show the window if hidden
   - bring it to the front if behind other apps
   - optionally hide it if already frontmost
5. The codebase must be minimal and easy to audit.
6. The app must not intercept credentials or manipulate ChatGPT authentication flows.
7. The app must rely on normal user login through the embedded web view.
8. The app must avoid invasive page automation and broad DOM manipulation.
9. The app must build cleanly with straightforward structure and comments.
10. The app must remain maintainable by one person.

## Nice-to-have requirements

1. Remember window size and position
2. Basic back/forward/reload controls
3. Menu or tray entry for Show/Hide ChatGPT
4. Configurable hotkey
5. Download support for files and generated images
6. A continuation helper for moving to a fresh chat with context

## Security constraints

1. No telemetry
2. No analytics SDKs
3. No auto-update framework in v1
4. No hidden background networking other than loading ChatGPT
5. No storage of passwords or tokens beyond normal embedded web view behavior
6. No private or unusual system APIs unless clearly justified
7. No accessibility permission requirement in v1 unless absolutely necessary

## Acceptance criteria

1. App launches to a window containing ChatGPT
2. User can log in normally through the embedded web view
3. Global hotkey works under LXQt
4. Hotkey reliably toggles or focuses the app window
5. Build succeeds without large external frameworks unless clearly justified
6. Code organization is understandable to a careful reviewer
7. README documents target environment, architecture, and limitations
