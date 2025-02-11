#include "computer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t sample_sprite[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 12, 0, 0, 0, 0, 12, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 2, 0, 0, 0, 0, 2, 0,
	0, 0, 2, 2, 2, 2, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};

void computer_init(computer_t *computer) {
	// Allocate 8MB ram
	computer->ram = malloc(RAM_SIZE);
	if (computer->ram == NULL) {
		printf("Couldn't allocate memory for fantasy RAM.\n");
		exit(EXIT_FAILURE);
	}
	memset(computer->ram, 0, RAM_SIZE);

	memcpy(&computer->ram->spritesheet.sprites[0], sample_sprite, sizeof(sprite_t)); 
}

void generate_rgb_framebuffer(computer_t *computer) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			uint8_t pixel = computer->ram->framebuffer.data[y * SCREEN_WIDTH + x];
			computer->rgb_framebuffer[y * SCREEN_WIDTH + x] = computer->ram->palette.colors[pixel];
		}
	}
}

bool point_in_bounds(int x, int y) {
	if (x < 0) return false;
	if (x >= SCREEN_WIDTH) return false;
	if (y < 0) return false;
	if (y >= SCREEN_HEIGHT) return false;

	return true;
}

void set_pixel(computer_t *computer, int x, int y, int color) {
	if (point_in_bounds(x, y)) {
		computer->ram->framebuffer.data[y * SCREEN_WIDTH + x] = color;
	}
}