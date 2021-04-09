## TODO

### Misc
- [ ] Add more compound commands: yi" and the likes.
- [ ] Add summery of shortcuts in help.
- [ ] PageUp/PageDown movement could be configured to prompt motions.

### Shortcuts
- [ ] Shortcuts: Make keyboard shortcuts configurable from vconfig.
- [ ] Shortcuts: star and hashtag command to search for word under cursor.
- [ ] Shortcuts: click on notebook header and tabs to close and create new tab.
- [ ] Shortcuts: separate tab to new window.

### Fixes
- [ ] FIX: Scroll to insert mode cursor when switching to insert mode from normal
    mode (and make this configurable).
- [ ] FIX: VTE documentation errors.
- [ ] FIX: Some word motions are buggy; maybe check nvim implementation.
- [ ] FIX: Sometimes the prompt scrolling wont work if first prompt output is
  too large.
### Style
- [ ] Style: make search entry BG configurable
- [ ] Style: Make it clear if no search results
- [ ] Style: Add official icon and .desktop shortcut
- [ ] Style: Add documentation on CSS file and name css objects clearly.

### Features
- [ ] Feature: Search for selected text in browser.
- [ ] Feature: modifyOtherKeys.
- [ ] Feature: Url mode.
- [ ] Feature: Figure out some mode indication on the screen: cursor color, or
    active-tab color.
- [ ] Feature: Copy previous command output.
- [ ] Feature: yc/yc: copy the command/the output of the current command.

### Motion
- [ ] Motion: Go to first non-blank char.
- [ ] Motion: ctrl-d/u ctrl-b/f for half and full page movement.
- [ ] Motion: Move cursor to prev/next prompt.
- [ ] Motion: vim o command: swap cursor & selection start positions.
- [ ] Motion: {} movement.

### Done
- [x] Feature: Confirm closing if more than one tab or program is running.
- [x] Style: Add tabs styling through css.
- [x] Shortcuts: yank line/word (configurable) if in normal mode.
- [x] Shortcuts: Enter key to exit normal/visual mode.
- [x] FIX: some colors default value should be null so that they are set to be
    swapped.
- [x] FIX: Check shortcuts when capslock is set!
- [x] Shortcuts: yank selection
- [x] Shortcuts: Better shortcut to enter/exit visual modes and normal mode.
- [x] Shortcuts: Better shortcuts to search.
- [x] Shortcuts: Shift-right/left word movement.
- [x] Shortcuts: Ctrl-shift left/right to start visual mode and select directly.
- [x] Shortcuts: Control-left & control-right in normal mode for word motion.
- [x] FIX: Add search wrap around to config or as shortcut.
- [x] Feature: Fullscreen.
- [x] Feature: clickable urls.
- [x] FIX: Move vterm cursor with search.
- [x] Feature: Search in terminal.
- [x] Prompt scrolling to down should be to next prompt.
- [x] FIX: Change git submodule url to http instead.
