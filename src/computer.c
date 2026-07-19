#include "computer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "core/file.h"
#include "editor/menu.h"
#include "res.h"
#include "api/lua_api.h"
#include "common/mem.h"
#include "common/io.h"
#include "common/string.h"

// This is global because the Lua API functions can't take arguments and they need the computer
static computer_t *_computer;

void set_global_computer(computer_t *computer) {
	_computer = computer;
}

computer_t *get_global_computer() {
	return _computer;
}

// TODO: design this file, probably seperate some stuff into another file, in a way I don't have forward declarations

void gui_init_monospace_font_widths(ram_t *ram, int font_index, int width);

void gui_init_font_widths(ram_t *ram, int font_index);

void computer_load_resouces(computer_t *computer) {
	// TODO: load the widths based on the lines drawn in the sprites
	// the monospace bool can also go
	// And the vertical_space is kinda stupid since there is already height
	computer->ram->fonts[0] = (font_t){
		.start_pos = {3200, 432},
		.horizontal_space = 1,
		.vertical_space = 3,
		.height = 10,
		.char_max_width = 8,
		.char_max_height = 16,
		.columns = 48,
		.surface = SURFACE_SKIN,

		.color_key = COLOR_BLACK,
		.seperator_color = COLOR_GREEN,
	};
	gui_init_font_widths(computer->ram, 0);

	computer->ram->fonts[1] = (font_t){
		.start_pos = {3200, 464},
		.horizontal_space = 1,
		.vertical_space = 1,
		.height = 8,
		.char_max_width = 8,
		.char_max_height = 8,
		.columns = 48,
		.surface = SURFACE_SKIN,

		.color_key = COLOR_BLACK,
		.seperator_color = COLOR_GREEN,
	};
	gui_init_monospace_font_widths(computer->ram, 1, 5);

	computer->ram->code_editor_config = (code_editor_config_t){
		.line_number_color = COLOR_DARKGRAY,
		.selection_color = COLOR_CYAN,
		.cursor_color = COLOR_BLUE,
		.scroll_speed = 3,
		// .background_color = COLOR_WHITE,
		.font_index = 1,
		.tab_size = 4,
		.token_colors = {
			[LUA_TOKEN_KEYWORD] = 12,
			[LUA_TOKEN_BUILTIN_FUNCTION] = 6,
			[LUA_TOKEN_IDENTIFIER] = COLOR_BLACK,
			[LUA_TOKEN_LITERAL] = 5,
			[LUA_TOKEN_STRING] = 2,
			[LUA_TOKEN_COMMENT] = 7,
			[LUA_TOKEN_OPERATOR] = 9,
			[LUA_TOKEN_WHITESPACE] = COLOR_LIGHTGRAY,
		},
	};

	// Init patterns
	for (size_t i = 0; i < PATTERN_AMOUNT; i++) {
		computer->ram->patterns[i].speed = 1;
		for (size_t j = 0; j < STEPS_IN_PATTERN; j++) {
			computer->ram->patterns[i].steps[j].volume = 11;
		}
	}

	// Init instruments
	for (size_t i = 0; i < MAX_INSTRUMENTS; i++) {
		computer->ram->instruments[i].attack = 16;
		computer->ram->instruments[i].decay = 32;
		computer->ram->instruments[i].sustain = 200;
		computer->ram->instruments[i].release = 48;
	}

}

void computer_init(computer_t *computer) {
	computer->ram = heap_alloc(RAM_SIZE);
	if (computer->ram == NULL) {
		printf("Couldn't allocate memory for fantasy RAM.\n");
		exit(EXIT_FAILURE);
	}
	memset(computer->ram, 0, RAM_SIZE);

	computer->ram->palette = g_builtin_palette;

	computer->current_path = string_copy(get_heap_allocator(), STR("/"));

	// Text files
	file_append_string(&computer->files[0], STR(""));

	computer->active_files_amount = 1;
}

void computer_quit(computer_t *computer) {
	// Free the code first
	for (size_t i = 0; i < FILES_AMOUNT; i++) {
		file_clear(&computer->files[0]);
	}

	heap_dealloc(computer->ram);
}

// Forward declaration so I don't have cyclic dependencies
// string_t file_to_string(file_t *file, allocator_t allocator);

void play_game(computer_t *computer) {
	if (lua_init(computer) == 0 && lua_call_init() == 0) {
		
		// Reset draw state
		// memset(&computer->ram->draw_state, 0, sizeof(draw_state_t));
		
		// Set the state
		computer->game_running = true;
		computer->state = STATE_IN_GAME;
	} else {
		abort_game(computer);
	}
}

void shell_new_command(computer_t *computer);

void abort_game(computer_t *computer) {
	computer->game_running = false;
	computer->state = STATE_IN_SHELL;
	lua_quit();

	shell_new_command(computer);
}

void quit_game(computer_t *computer) {
	computer->game_running = false;

	shell_new_command(computer);
	
	lua_quit();
}

// TODO: resume_game function that calls an equivalent Lua global
// for debugging

// TODO: reimplement this for the new sprite system, when I need it
/*
int sprite_x_to_sprite_sheet_x(ram_t *ram, int sprite_sheet_index, int sprite_index, int x) {
	// int sprite_width = ram->spritesheets[sprite_sheet_index].sprite_width;

	int sprite_x = sprite_index % (SPRITESHEET_WIDTH / SPRITE_WIDTH);

	return (sprite_x * SPRITE_WIDTH) + x;
}

int sprite_y_to_sprite_sheet_y(ram_t *ram, int sprite_sheet_index, int sprite_index, int y) {
	int sprite_width = ram->spritesheets[sprite_sheet_index].sprite_width;
	int sprite_height = ram->spritesheets[sprite_sheet_index].sprite_height;

	int sprite_y = sprite_index / (SPRITESHEET_WIDTH / sprite_width);

	return (sprite_y * sprite_height) + y;
}

int sprite_get_pixel(ram_t *ram, int sprite_sheet_index, int sprite_index, int x, int y) {
	int sprite_sheet_x = sprite_x_to_sprite_sheet_x(ram, sprite_sheet_index, sprite_index, x);
	int sprite_sheet_y = sprite_y_to_sprite_sheet_y(ram, sprite_sheet_index, sprite_index, y);

	return ram->spritesheets[sprite_sheet_index].data[sprite_sheet_y * SPRITESHEET_WIDTH + sprite_sheet_x];
}

void sprite_set_pixel(ram_t *ram, int sprite_sheet_index, int sprite_index, int x, int y, uint8_t color) {
	int sprite_sheet_x = sprite_x_to_sprite_sheet_x(ram, sprite_sheet_index, sprite_index, x);
	int sprite_sheet_y = sprite_y_to_sprite_sheet_y(ram, sprite_sheet_index, sprite_index, y);

	ram->spritesheets[sprite_sheet_index].data[sprite_sheet_y * SPRITESHEET_WIDTH + sprite_sheet_x] = color;
}
*/

void set_game_path(computer_t *computer, string_t new_path) {
	heap_dealloc(computer->game_path.data);
	computer->game_path = string_copy(get_heap_allocator(), new_path);
}

void sprite_editor_import_spritesheet(computer_t *computer, string_t path);

void import_file(computer_t *computer, string_t path) {
	string_t extension = path_get_filename_extension(path);

	if (string_eq(extension, STR("bmp"))) {
		sprite_editor_import_spritesheet(computer, path);
	}
}
