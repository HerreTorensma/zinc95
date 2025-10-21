/*
API
*/

#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include "../computer.h"

// Here I keep info about each API function in one location,
// so I can use the name for registering lua functions and also syntax highlighting,
// And I can use the rest for generating documentation or possibly builtin documentation
// This also ensures I don't mix them up

// extern const api_metas[API_FUNC_COUNT];

typedef enum api_func {
	API_FUNC_CLS,
	API_FUNC_RECT,
	API_FUNC_RECTF,
	API_FUNC_LINE,
	API_FUNC_CIRC,
	API_FUNC_SPR,

	API_FUNC_MAP,

	API_FUNC_KEY,
	API_FUNC_KEYP,

	API_FUNC_TICKS,
	
	API_FUNC_SAVE_TO_SLOT,
	API_FUNC_LOAD_FROM_SLOT,

	API_FUNC_COUNT,

} api_func_t;

// Uses null-terminated strings because they are only fed to Lua or used to generate documentation
// there is no real benefit to using length-based strings here
typedef struct api_meta {
	char *name;
	char *signature;
	char *desc;

	// Not used yet
	// Actually not sure if it's a good idea to define here but we'll see
	int accepted_args_amount;
	char *error;
} api_meta_t;

// I feel it's fine to be extremely global since it's immutable and compile-time evaluated
extern const api_meta_t api_metas[API_FUNC_COUNT];

void api_meta_print();



// --- Math ---

// Function that converts a radian angle to an 8 directional index
// so the developer can draw a sprite for every 8 direction
int api_angto8(ram_t *ram, double angle);



// --- Graphics ---

// Fill the screen with specified color
void api_cls(ram_t *ram, int color);

// Draw an unfilled rectangle
void api_rect(ram_t *ram, int x, int y, int w, int h, int color);

// Draw a filled rectangle
void api_rectf(ram_t *ram, int x, int y, int w, int h, int color);

// Draw a line from a point to a point
void api_line(ram_t *ram, int x1, int y1, int x2, int y2, int color);

// Draw an unfilled circle
void api_circ(ram_t *ram, int x, int y, int radius, int color);

// Draw a sprite
void api_spr(ram_t *ram, int idx, int x, int y, int width, int height);

// Draw a rect from the spritesheet
void api_sspr(ram_t *ram, int dst_x, int dst_y, int dst_w, int dst_h, int src_x, int src_y, int src_w, int src_h, int color_key);

// Draw a portion of a map layer
// The x, y, w, h are the rect of the drawn portion in tiles
void api_map(ram_t *ram, int layer, int x, int y, int cell_x, int cell_y, int cell_w, int cell_h);



// --- Input ---

// Saves mouse coordinates in x and y
void api_mouse(ram_t *ram, int *x, int *y);

// Check if a key is held
bool api_key(ram_t *ram, int key);

// Check if a key is pressed
bool api_keyp(ram_t *ram, int key);

// Check if a key is released
bool api_keyr(ram_t *ram, int key);

// Check if a mouse gui_button is held
bool api_mouse_btn(ram_t *ram, int button);

// Check if a mouse gui_button is pressed
bool api_mouse_btnp(ram_t *ram, int button);

// Check if a mouse gui_button is released
bool api_mouse_btnr(ram_t *ram, int button);

bool api_mouse_scrolled(ram_t *ram, int direction);



// --- GUI ---

// Draws a string on the screen
void api_text(ram_t *ram, int font_index, char text[], int x, int y, int color);



// --- Utils ---

// Get the amount of ticks since the console has been running
int api_ticks(ram_t *ram);

// Get a byte from ram
uint8_t api_peek(ram_t *ram, uint64_t address);

// Set a byte in ram
void api_poke(ram_t *ram, uint64_t address, uint8_t value);


// --- Audio ---
void api_sfx(ram_t *ram, int index);
