/*
Main
*/

#include "core/gfx.h"
#include "computer.h"
#include "core/window.h"
#include "api/lua_api.h"
#include "api/api.h"
#include "core/audio.h"
#include "common/io.h"
#include "common/string.h"
#include <stdlib.h>

#define SDL_MAIN_HANDLED

void run_tests(void);

int main(int argc, char *argv[]) {
	run_tests();

	computer_t computer = {0};
	computer_init(&computer);

	set_global_computer(&computer);
	
	api_meta_print();
	
	window_init("zinc95", 2);
	audio_init(&computer);

	temp_mem_init(MB(4ULL));

	computer_load_resouces(&computer);

	if (argc > 1) {
		game_load(&computer, STR(argv[1]));
	} else {
		// printf("Specify a game as an argument");
		exit(EXIT_SUCCESS);
	}

	computer.ram->border_color = 8;

	create_default_directories();
	
	// TODO: simplify the state switching logic
	while (window_is_open()) {
		temp_clear();

		window_tick_start(&computer);

		switch (computer.state) {
			case STATE_IN_GAME: {
				if (lua_call_update() != 0) {
					abort_game(&computer);
				}
				break;
			}
		}
		
		switch (computer.state) {
			case STATE_IN_GAME: {
				if (lua_call_draw() != 0) {
					abort_game(&computer);
				}

				gfx_generate_rgb_framebuffer(&computer);
				break;
			}
		}

		window_render(&computer);

		window_tick_end(&computer);
	}

	temp_free();

	// TODO: rename to computer_deinit
	computer_quit(&computer);

	audio_deinit(&computer);
	window_quit();
}
