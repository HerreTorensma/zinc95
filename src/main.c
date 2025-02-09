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

	computer.ram.framebuffer.data[0] = 1;
	computer.ram.framebuffer.data[1] = 1;
	computer.ram.framebuffer.data[2] = 2;
	computer.ram.framebuffer.data[3] = 2;
	computer.ram.framebuffer.data[642] = 2;

	for (int i = 0; i < 480; i++) {
		computer.ram.framebuffer.data[i] = 32;
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