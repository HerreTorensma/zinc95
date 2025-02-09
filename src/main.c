#include <stdio.h>
#include <string.h>

#include "computer.h"
#include "backend/backend.h"
#include "util/util.h"

#define BACKEND_SDL2

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