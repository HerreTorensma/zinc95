#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include "../computer.h"

// Fill the screen with specified color
void api_cls(computer_t *computer, int color);

// --- Graphics/Map
// Draw a sprite
void api_spr(computer_t *computer, int index, int x, int y, int width, int height);

// Draw a scaled sprite
void api_sspr(computer_t *computer, int index, int x, int y, int width, int height, int scale);

// Draw a portion of the map
void api_map(computer_t *computer, int x, int y, int width, int height);

// --- Input
// Check button is held
void api_btn(computer_t *computer, int button);

// Check button pressed
void api_btnp(computer_t *computer, int button);

// Saves mouse coordinates in x and y
void api_mouse(computer_t *computer, int *x, int *y);

// Get a byte from ram
uint8_t api_peek(computer_t *computer, uint64_t address);

// Set a byte in ram
void api_poke(computer_t *computer, uint64_t address, uint8_t value);

// --- Additional math
// Function that converts a radian angle to an 8 directional index
// so the developer can draw a sprite for every 8 direction
int api_angto8(computer_t *computer, double angle);

// --- Drawing
// Draw a line from a point to a point
void api_line(computer_t *computer, int x1, int y1, int x2, int y2, int color);

// Draw an unfilled rectangle
void api_rect(computer_t *computer, int x, int y, int w, int h, int color);

// Draw a filled rectangle
void api_rectf(computer_t *computer, int x, int y, int w, int h, int color);

// Draw an unfilled circle
void api_circ(computer_t *computer, int x, int y, int r, bool filled, int color);

// --- Input
// Check if a key is held
bool api_key(computer_t *computer, int key);

// Check if a key is pressed
bool api_keyp(computer_t *computer, int key);

// Check if a key is released
bool api_keyr(computer_t *computer, int key);

// Check if a mouse button is held
bool api_mouse_btn(computer_t *computer, int button);

// Check if a mouse button is pressed
bool api_mouse_btnp(computer_t *computer, int button);

// Check if a mouse button is released
bool api_mouse_btnr(computer_t *computer, int button);