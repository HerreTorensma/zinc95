# zinc95
zinc95 is a complete fantasy PC and game development enviroment inspired by fantasy consoles such as the PICO-8 and TIC-80, but with Windows 95-era specs.

- Programming in Lua
- Builtin code editor, sprite editor, map editor

## Specification
display: 640x480, 256 colors graphics mode
input: keyboard, mouse

## API
### cls
`cls(color=0)`
Clear the screen

### rect
`rect(x, y, w, h, color)`
Draw an unfilled rectangle

### rectf
`rectf(x, y, w, h, color)`
Draw a filled rectangle

### line
`line(x1, y1, x2, y2, color)`
Draw a line from (x1, y1) to (x2, y2)

### circ
`circ(x, y, radius, color)`
Draw an unfilled circle

### spr
`spr(idx, x, y, [width], [height])`
Draw a sprite by global index

### map
`map(layer, x, y, cell_x, cell_y, cell_w, cell_h)`
Draw a portion of the given map layer

### key
`key(key)`
Key if a key is being held

### ticks
`ticks()`
Get the amount of ticks the program has been running

## Dependencies
- SDL2
- Lua
- stb_image

## Compiling
For now the program only runs on Windows

- `make libs all`