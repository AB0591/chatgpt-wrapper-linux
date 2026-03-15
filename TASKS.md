# Build Tasks

## Milestone 1: Linux feasibility
- [x] Choose the Linux UI/runtime stack
- [x] Confirm the app can target Lubuntu/LXQt cleanly
- [x] Confirm `WebKitGTK` is viable for loading ChatGPT
- [x] Create a minimal desktop app shell
- [x] Confirm the app builds and runs locally

## Milestone 2: Embed ChatGPT
- [x] Add a Linux web view wrapper around `WebKitGTK`
- [x] Load ChatGPT in the main content area
- [x] Confirm normal navigation and rendering
- [x] Confirm login is possible through the embedded web view
- [x] Keep implementation native-feeling and easy to audit

## Milestone 3: Window management
- [x] Implement reliable show, focus, hide, and toggle behavior
- [x] Ensure app activation is reliable under LXQt
- [x] Keep window behavior easy to understand and debug

## Milestone 4: Global hotkey
- [x] Implement a global shortcut that works under Lubuntu/LXQt
- [x] Wire the shortcut to window toggle/focus behavior
- [x] Test behavior when the app is backgrounded
- [x] Keep the implementation minimal and native where possible

## Milestone 5: Downloads
- [x] Add native download support for ChatGPT-generated files and images
- [x] Use a native save flow for user-selected destinations
- [x] Keep download handling minimal and auditable

## Milestone 6: Polish
- [ ] Persist window size and position if it stays small and reliable
- [x] Add simple navigation controls
- [x] Add concise documentation and known caveats
- [x] Keep the UI lightweight and maintainable

## Milestone 7: Conversation continuation aid
- [x] Add a `New Chat with Context` action
- [x] Extract a limited recent context snapshot from the current conversation
- [x] Open a fresh ChatGPT thread and populate the composer with a continuation prompt
- [x] Keep the user in control by not auto-sending the draft
- [x] Fail safely when prompt population is unavailable
