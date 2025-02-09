#pragma once

#include <inttypes.h>
#include <stdbool.h>

#define RAM_SIZE (4 * 1024 * 1024)

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define PALETTE_SIZE 256

#define FPS 60

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

// 4MB RAM (excluding what the lua code takes up)
typedef union ram {
	struct {
		framebuffer_t framebuffer;
		palette_t palette;
	};

	uint8_t *data;
} ram_t;

typedef struct computer {
	rgb_color_t rgb_framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
	ram_t ram;
} computer_t;

// This function currently only allocates memory for the fantasy ram
void computer_init(computer_t *computer);

// Generate a buffer of rgb_color_t using palette so it can be rendered by a backend later
void generate_rgb_framebuffer(computer_t *computer);

// Check if a given point is within the bounds of the framebuffer
bool point_in_bounds(int x, int y);

// Set a pixel in the framebuffer
void set_pixel(computer_t *computer, int x, int y, int color);