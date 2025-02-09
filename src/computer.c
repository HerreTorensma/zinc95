#include "computer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void computer_init(computer_t *computer) {
	// Allocate 4MB ram
	computer->ram.data = malloc(RAM_SIZE);
	if (computer->ram.data == NULL) {
		printf("Couldn't allocate memory for fantasy RAM.\n");
		exit(EXIT_FAILURE);
	}
	memset(computer->ram.data, 0, RAM_SIZE);
}

void generate_rgb_framebuffer(computer_t *computer) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			uint8_t pixel = computer->ram.framebuffer.data[y * SCREEN_WIDTH + x];
			computer->rgb_framebuffer[y * SCREEN_WIDTH + x] = computer->ram.palette.colors[pixel];
		}
	}
}