/*
Main
*/

#include "core/gfx.h"
#include "computer.h"
#include "core/window.h"
#include "editor/menu.h"
#include "core/input.h"
#include "api/lua_api.h"
#include "api/api.h"
#include "core/audio.h"
#include "editor/gui.h"
#include "core/txt.h"
#include "common/io.h"
#include "common/string.h"
#include "res.h"
#include "serialize.h"
#include <stdlib.h>

#ifdef BACKEND_SDL2
#define SDL_MAIN_HANDLED
#endif

void run_tests(void);

int main(int argc, char *argv[]) {
	run_tests();

	temp_mem_init(MB(4ULL));

	computer_t computer = {0};
	computer_init(&computer);

	set_global_computer(&computer);
	
	api_meta_print();
	
	window_init(STR("zinc95"), 2);
	input_init();
	audio_init(&computer);
	
	// Spritesheet page: 352x128 pixels (416x128 maybe)
	// or 44x16 = 704 sprites per pages
	// 8 of those pages
	// so 5632 8x8 sprites
	// Or maybe the sprites will be 16x16
	// You could set default "size" for each sprite page but under the hood they will still be 8x8
	// while drawing, sprite page is treated as big image and upon finishing a stroke is then converted to sprites
	// or each sprite references the actual image (nah) (actually that might be good)

	// OR

	// Keep ability to change sprite size per sprite
	// whenever these are changed, the 
	// no the whole order would fuck up

	// OR

	// For now fuck multiple lua files, fuck text format carts etc.
	// Probably also fuck dynamic allocation
	// Just make the shit like I originally envisioned

	// Collision api
	// But then id need a whole object api and thats fucky
	// Spatial grid

	// I think it would be good for development to save code, sprites, map etc. in multiple files to make it more git-friendly
	// then there would be a burn to cart option or whatever (export) to package it into one file, maybe with some compression

	// REMINDER FUTURE ME: make sure sprite page sizes are independent from sprite sheet sizes, actually probably remove the concept of a page since youre just editing a part of the larger sprite sheet
	// Maybe make 'total' sprite sheet 768x768 or 2x6 'pages'
	// actually that would also make a lot of stuff pretty annoying so maybe i wont do it

	// I should make a red line to indicate character width for fonts instead of setting them dynamically

	// About the map editor
	// you get 8 layers
	// Per layer you can set the tile size

	// I should also make vector functions but just input and output 2 floats instead of a vector datastructure
	// idk i just like x, y

	// Ok I think I'm just gonna implement vectors at least for C

	// Make a macro for rects like ANCHOR_RECT so I don't have to hard code positions of GUI elements
	// or just make it a function, and also functions like rect_get_bottom, rect_set_bottom etc. and also make those API functions
	// man I just need to refactor with a shit ton of helper functions to make my life easier
	// Also for the refactor I need a temp allocator for strings and such

	// maybe i should just get and store the mouse position at the beginning of every frame/tick so it won't call SDL everytime you do input_get_mouse_pos()

	// I want to add debugging stuff like a stack of print statements that are drawn while the game is playing
	// maybe implement breakpoints in the editor if possible but thats gonna be hard

	// Fonts
	// A skin should contain a GUI and text mode font (maybe the same)
	// Then there should be a builtin font used by games if there is no font index given or the user didn't make any font themself
	// Because that shouldn't rely on the skin, it should just be hardcoded builtin type shit
	// Or maybe in the spritesheet but nah because I want a blank project to be truly blank, no boilerplate whatsoever
	// Then IDK if the code editor font should be part of the skin or not
	// I think the text mode font shouldn't be part of the skin

	// printf("RAM size: %llu\n", sizeof(struct readable_ram));

	// game_load(&computer, STR("game.zinc95"));
	// game_load_old(&computer, "game.zinc95");
	
	gui_load_skin(computer.ram, STR("res/skin.bmp"), 1, 0);
	computer_load_resouces(&computer);
	// gui_load_skin(computer.ram, "res/skin2.png", 1, 40);
	gfx_load_surface(
		&computer.ram->palette,
		(surface_t){.data = computer.ram->text_mode_font.data, .width = TEXT_MODE_FONT_BITMAP_WIDTH, .height = TEXT_MODE_FONT_BITMAP_HEIGHT},
		STR("res/font.bmp")
	);

	if (argc > 1) {
		game_load(&computer, STR(argv[1]));
	}

	computer.ram->border_color = 8;

	workspace_menu_init(&computer);

	// Logic
	// For now the shell is just always running if the text video mode is active, and the game and editor are paused
	// When pressing the play button or hitting F5 the game is run
	// Using the ESC key you 
	// idk
	// The shell should not be active when the game is running, it should just print stuff from the game
	// 

	// I gotta think about getting rid of the update loop in the editors
	// I feel like it's redundant there because the main advantage I think is being able to pause the update while the draw keeps running
	// especially when you have GUI stuff in the draw you can pause your game and stuff have working UI
	// But there will never be an option to pause the editor since it's all GUI anyway
	// idk

	// I need to implement a popup message system
	// for some status update like project saved or whatever

	// TODO:
	// Make tiles 16x16?

	shell_init(&computer);

	create_default_directories();

	// string_t absolute_path = get_absolute_path(get_temp_allocator(), path_append(get_temp_allocator(), STR("discs"), STR("idksambdsanm")));
	// set_game_path(&computer, absolute_path);
	
	// TODO: simplify the state switching logic
	while (window_is_open()) {
		temp_clear();

		window_tick_start(&computer);

		if (is_keybind_pressed(g_keybinds.global.toggle_terminal)) {
			computer.state++;
			if (computer.state == STATE_IN_GAME + 1) {
				computer.state = STATE_IN_SHELL;
			}

			if (computer.state == STATE_IN_GAME && !computer.game_running) {
				computer.state = STATE_IN_SHELL;
			}
		}

		switch (computer.state) {
			case STATE_IN_SHELL: {
				if (!computer.game_running) {
					shell_update(&computer);
				} else {
					// Game keeps running while the terminal is open
					if (lua_call_update() != 0) {
						abort_game(&computer);
					}
				}
				break;
			}
			case STATE_IN_EDITOR: {
				workspace_menu_update(&computer);
				break;
			}
			case STATE_IN_GAME: {
				if (lua_call_update() != 0) {
					abort_game(&computer);
				}
				break;
			}
		}
		
		switch (computer.state) {
			case STATE_IN_SHELL: {
				txt_generate_rgb_framebuffer(&computer);
				break;
			}
			case STATE_IN_EDITOR: {
				workspace_menu_draw(&computer);
				gfx_generate_rgb_framebuffer(&computer);
				break;
			}
			case STATE_IN_GAME: {
				if (lua_call_draw() != 0) {
					abort_game(&computer);
				}

				// The code below makes the whole screen darker by one step
				// TODO: put in its own function
				/*
				for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
					for (size_t x = 0; x < SCREEN_WIDTH; x++) {
						int pixel = gfx_get_pixel(&computer.ram->framebuffer, x, y);
			
						int new_pixel = 0;

						if (pixel < 16) {
							new_pixel = pixel;	
						} else if (pixel >= 16 && pixel <= 31) {
							new_pixel = pixel - 2;
							if (new_pixel < 16) {
								new_pixel = 16;
							}
						} else {
							new_pixel = pixel + 72;
						}

						if (new_pixel > 255) {
							new_pixel = 0;
						}

						gfx_set_pixel(&computer.ram->framebuffer, x, y, new_pixel);
					}
				}
				*/

				gfx_generate_rgb_framebuffer(&computer);
				break;
			}
		}

		window_render(&computer);

		window_tick_end(&computer);
	}

	workspace_menu_deinit(&computer);

	shell_deinit(&computer);

	temp_free();

	// TODO: rename to computer_deinit
	computer_quit(&computer);

	audio_deinit(&computer);
	window_quit();

	return EXIT_SUCCESS;
}
