#include <stdio.h>
#include <string.h>

#include "computer.h"
#include "backend/backend.h"
#include "util/util.h"
#include "api/api.h"
#include "workspaces/menu.h"
#include "backend/input.h"
#include "api/lua_api.h"

int main(int argc, char *argv[]) {
	computer_t computer = {0};
	computer_init(&computer);

	set_global_computer(&computer);

	backend_init("zinc95", 2);

	workspace_menu_init(&computer);

	while (window_is_open()) {
		backend_tick_start(&computer);

		switch (computer.state) {
			case STATE_EDITING:
				workspace_menu_update(&computer);
				break;
			case STATE_PLAYING:
				lua_call_update();
				if (api_keyp(&computer, KEY_ESC)) {
					quit_game(&computer);
				}
				break;
		}
		
		switch (computer.state) {
			case STATE_EDITING:
				workspace_menu_draw(&computer);
				break;
			case STATE_PLAYING:
				lua_call_draw();
				break;
		}

		backend_render(&computer);

		backend_tick_end(&computer);
	}

	backend_quit();
}