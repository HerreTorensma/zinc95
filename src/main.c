#include <stdio.h>
#include <string.h>

#include "computer.h"
#include "backend/backend.h"
#include "util/util.h"
#include "api/api.h"

int main(int argc, char *argv[]) {
	computer_t computer = {0};
	computer_init(&computer);

	load_palette_from_disk(&computer, "palette.txt");
	
	// memset(mem.framebuffer.data, 1, SCREEN_WIDTH * SCREEN_HEIGHT);

	computer.ram->framebuffer.data[0] = 1;
	computer.ram->framebuffer.data[1] = 1;
	computer.ram->framebuffer.data[2] = 2;
	computer.ram->framebuffer.data[3] = 2;
	computer.ram->framebuffer.data[642] = 2;

	for (int i = 0; i < 480; i++) {
		computer.ram->framebuffer.data[i] = 32;
	}

	api_cls(&computer, 7);

	api_rectf(&computer, 64 + 1, 1, 3, 3, 4);
	api_rect(&computer, 64 + 5, 1, 3, 3, 0);
	
	// api_rect(&computer, 9, 1, 10, 10, 0);
	
	api_rect(&computer, 64 + 0, 0, 50, 25, 0);
	
	for (int i = 0; i < PALETTE_SIZE; i++) {
		api_rectf(&computer, (i % 8) * 8, (i / 8) * 8, 8, 8, i);
	}
	api_line(&computer, 64 + 0, 0, 10, 25, 4);

	api_spr(&computer, 0, 128, 128, 1, 1);

	api_cls(&computer, 7);

	for (int i = 0; i < SPRITE_PAGE_HEIGHT; i++) {
		for (int j = 0; j < SPRITE_SHEET_WIDTH; j++) {
			int index = i * SPRITE_SHEET_WIDTH + j;
			int x = 0 + (j * SPRITE_WIDTH);
			int y = 50 + i * SPRITE_HEIGHT;
			api_spr(&computer, index, x, y, 1, 1);
			// printf("ja toch %d %d %d\n", index, x, y);
		}
	}

	for (int i = 0; i < PALETTE_SIZE; i++) {
		api_rectf(&computer, (i % 32) * 8, 40 + 256 + (i / 32) * 8, 8, 8, i);
	}

	backend_init("zinc95", 2);

	while (window_is_open()) {
		backend_tick(&computer);

		// TODO: Call game update function

		// TODO: Call game draw function

		backend_render(&computer);

		backend_tick_end(&computer);
	}

	backend_quit();
}