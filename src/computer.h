#pragma once

#include <inttypes.h>

#define RAM_SIZE (4 * 1024 * 1024)
// #define SCREEN_BUFFER_ADDRESS 0x000000
// #define PALETTE_ADDRESS 0x04B000

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define PALETTE_SIZE 256

#define MAX_COLORS 256

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

typedef union ram {
	/*	
	4MB RAM (excluding what the lua code takes up)
	
	1MB VRAM
	  Contains screen buffer, color palette, etc.
	512KB sprites

	so

	000000 - 4B000 screen buffer
	*/

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

void computer_init(computer_t *computer);

void generate_rgb_framebuffer(computer_t *computer);