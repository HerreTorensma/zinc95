#pragma once

#define BACKEND_SDL2

#include <inttypes.h>
#include <stdbool.h>

#define RAM_SIZE (8 * 1024 * 1024)

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define PALETTE_SIZE 256

#define SPRITE_WIDTH 16
#define SPRITE_HEIGHT 16

#define SPRITE_SHEET_WIDTH 16
#define SPRITE_SHEET_HEIGHT 8 * 16
#define SPRITE_PAGE_HEIGHT 16

#define VISIBLE_CHARACTERS_SIZE 96
#define VISIBLE_CHARACTERS_START 32
#define MAX_CHARACTER_WIDTH 16
#define MAX_CHARACTER_HEIGHT 16

#define FPS 60

typedef struct rect {
	int x;
	int y;
	int w;
	int h;
} rect_t;

typedef struct rgb_color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_color_t;

typedef struct palette {
	rgb_color_t colors[PALETTE_SIZE];
} palette_t;

typedef struct framebuffer {
	uint8_t data[SCREEN_WIDTH * SCREEN_HEIGHT];
} framebuffer_t;

typedef struct sprite {
	uint8_t data[SPRITE_WIDTH * SPRITE_HEIGHT];
} sprite_t;

// There are 8 of these 320x320 spritesheets
// The last one is reserved for fonts
typedef struct spritesheet {
	sprite_t sprites[SPRITE_SHEET_WIDTH * SPRITE_SHEET_HEIGHT];
} spritesheet_t;

typedef struct font_meta {
	uint8_t max_width;
	uint8_t height;
	uint8_t spacing;

	uint8_t widths[VISIBLE_CHARACTERS_SIZE];
} font_meta_t;

// Each character in a font is 16*16 pixels
typedef union font_data {
	sprite_t sprites[VISIBLE_CHARACTERS_SIZE];
	uint8_t data[VISIBLE_CHARACTERS_SIZE * MAX_CHARACTER_WIDTH * MAX_CHARACTER_HEIGHT];
} font_data_t;

// 8MB RAM (excluding what the lua code takes up)
typedef union ram {
	struct {
		framebuffer_t framebuffer;
		palette_t palette;
		spritesheet_t spritesheet;
		font_data_t font_data;
	};

	uint8_t data[RAM_SIZE];
} ram_t;

typedef struct computer {
	rgb_color_t rgb_framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
	ram_t *ram;
} computer_t;

// This function currently only allocates memory for the fantasy ram
void computer_init(computer_t *computer);

// Generate a buffer of rgb_color_t using palette so it can be rendered by a backend later
void generate_rgb_framebuffer(computer_t *computer);

// Check if a given point is within the bounds of the framebuffer
bool point_in_bounds(int x, int y);

// Set a pixel in the framebuffer
void set_pixel(computer_t *computer, int x, int y, int color);

bool point_in_rect(int x, int y, rect_t rect);
