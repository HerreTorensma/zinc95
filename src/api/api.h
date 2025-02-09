#pragma once

#include <inttypes.h>

// Runs when the game starts
void api_init();

// Runs at 60fps
void api_update();

// Runs at fps, after update
void api_draw();

void api_cls(int color);

// Draw a sprite
void api_spr(int index, int x, int y, int width, int height);

void api_map(int x, int y, int width, int height);

void api_btn(int button);

void api_btnp(int button);

// Saves mouse coordinates in x and y
void api_mouse(int *x, int *y);

uint8_t api_peek(uint64_t address);

void api_poke(uint64_t address, uint8_t value);

// Function that converts a radian angle to an 8 directional index
// so the developer can draw a sprite for every 8 direction
int api_angto8(double angle);

void api_line(int x1, int y1, int x2, int y2);