/*
Main
*/

#include <stdio.h>
#include <string.h>

#include "computer.h"
#include "backend/window.h"
#include "editor/menu.h"
#include "backend/input.h"
#include "api/lua_api.h"

#define SDL_MAIN_HANDLED

int main(int argc, char *argv[]) {
	computer_t computer = {0};
	computer_init(&computer);

	set_global_computer(&computer);

	api_meta_print();

	window_init("zinc95", 2);

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
	
	game_load(&computer, "game.zinc95");
	computer_load_resouces(&computer);
	
	workspace_menu_init(&computer);
	
	while (window_is_open()) {
		window_tick_start(&computer);

		switch (computer.state) {
			case STATE_EDITING:
				workspace_menu_update(&computer);
				break;
			case STATE_PLAYING:
				lua_call_update();
				if (input_key_pressed(KEY_ESC)) {
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

		window_render(&computer);

		window_tick_end(&computer);
	}

	computer_quit(&computer);

	window_quit();
}