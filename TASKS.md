# Build Tasks

## Milestone 1: Linux feasibility
- [ ] Choose the Linux UI/runtime stack
- [ ] Confirm the app can target Lubuntu/LXQt cleanly
- [ ] Confirm `WebKitGTK` is viable for loading ChatGPT
- [ ] Create a minimal desktop app shell
- [ ] Confirm the app builds and runs locally

## Milestone 2: Embed ChatGPT
- [ ] Add a Linux web view wrapper around `WebKitGTK`
- [ ] Load ChatGPT in the main content area
- [ ] Confirm normal navigation and rendering
- [ ] Confirm login is possible through the embedded web view
- [ ] Keep implementation native-feeling and easy to audit

## Milestone 3: Window management
- [ ] Implement reliable show, focus, hide, and toggle behavior
- [ ] Ensure app activation is reliable under LXQt
- [ ] Keep window behavior easy to understand and debug

## Milestone 4: Global hotkey
- [ ] Implement a global shortcut that works under Lubuntu/LXQt
- [ ] Wire the shortcut to window toggle/focus behavior
- [ ] Test behavior when the app is backgrounded
- [ ] Keep the implementation minimal and native where possible

## Milestone 5: Downloads
- [ ] Add native download support for ChatGPT-generated files and images
- [ ] Use a native save flow for user-selected destinations
- [ ] Keep download handling minimal and auditable

## Milestone 6: Polish
- [ ] Persist window size and position if it stays small and reliable
- [ ] Add simple navigation controls
- [ ] Add concise documentation and known caveats
- [ ] Keep the UI lightweight and maintainable

## Milestone 7: Conversation continuation aid
- [ ] Add a `New Chat with Context` action
- [ ] Extract a limited recent context snapshot from the current conversation
- [ ] Open a fresh ChatGPT thread and populate the composer with a continuation prompt
- [ ] Keep the user in control by not auto-sending the draft
- [ ] Fail safely when prompt population is unavailable
