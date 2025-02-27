# Todo

## Requirements
### Critical
- [ ] Undo/redo
- [ ] Lua integration
- [ ] Sprite editor
- [ ] Map editor
- [ ] Switching between editors
- [ ] Lua integration
- [ ] Seperate color palette etc. for editor and game (not sure how to go about this yet)

### Important
- [ ] Sound effects editor
- [ ] Music editor
- [ ] Code editor
- [ ] Saving 'disc' (instead of cart) as text file so it's easily editable
- [ ] Open to future support to different backends such as Sokol (SDL2 is default)
- [ ] Make screen 'black border' color editable per game
- [ ] Make editor palette and such configurable through lua

- Sprite editor
	- Brush tool
	- Bucket fill tool
	- Copy sprite and paste it in another
	- Show current color and palette index
	- Make spritesheet a union that can also be accessed as a 2d array of "pages" of 32x32 sprites

- For text rendering: do inline sprites with some string formatting

### Would like to have
- [ ] Pseudo 3d by raycasting

## Tasks
### Pending
- Implement non-monospace fonts
- Do more research on immediate-mode GUIs
- Create function buttons
- Make menu bar to switch between editors
- Implement undo/redo for sprite editor (make it usable for other editors)
- Make most basic form of map editor
- Make each UI element focusable

### Done