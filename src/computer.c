#include "computer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "backend/file.h"
#include "backend/input.h"
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
		.sprite_index = 5376,
		.horizontal_space = 1,
		.vertical_space = 3,
		.height = 10,
		.sprite_width = 1,
		.sprite_height = 2,

		.color_key = COLOR_BLACK,
		.seperator_color = 10,
	};
	gui_init_font_widths(computer->ram, 0);

	computer->ram->fonts[1] = (font_t){
		.sprite_index = 5568,
		.horizontal_space = 1,
		.vertical_space = 1,
		.height = 8,
		.sprite_width = 1,
		.sprite_height = 1,

		.color_key = COLOR_BLACK,
		.seperator_color = 10,
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
			[LUA_TOKEN_WHITESPACE] = COLOR_NONE,
		},
	};
}

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

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
	// file_append_line(&computer->files[0], STR(""));

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

static const char _hex_chars[] = "0123456789abcdef";

// hex should be twice as big as bytes
static void _bytes_to_hex(uint8_t bytes[], size_t len, char hex[]) {
	for (size_t i = 0; i < len; i++) {
		hex[i * 2] = _hex_chars[(bytes[i] >> 4) & 0x0f]; \
		hex[i * 2 + 1] = _hex_chars[(bytes[i] & 0x0f)]; \
	}
}

// TODO: put this and the load one in io but first I need an io.c that's not platform specific
string_t file_write_string(string_t path, string_t string) {
	FILE *file = fopen(string_to_c_string(get_temp_allocator(), path), "w");
	fwrite(string.data, sizeof(char), string.len, file);
	fclose(file);
}

// New implementation with length based strings
void game_save(computer_t *computer, string_t path) {
	printf("Saving game...\n");
	
	string_builder_t builder = {0};
	
	// 8MB should be enough for most games
	// Also make it heap allocated to not overload the temporary memory
	string_builder_init(&builder, get_heap_allocator(), MB(8));
	
	// Lua code
	for (size_t i = 0; i < computer->active_files_amount; i++) {
		if (i > 0) {
			string_builder_append(&builder, STR("-->8\n"));
		}
		
		string_t code = computer->files[i].string;
		string_builder_append(&builder, code);
		string_builder_append(&builder, STR("\n"));
	}

	// Spritesheet
	string_builder_append(&builder, STR("__gfx__\n"));
	for (size_t y = 0; y < SPRITESHEET_HEIGHT; y++) {
		for (size_t x = 0; x < SPRITESHEET_WIDTH; x++) {
			color_t color = computer->ram->spritesheet.data[y * SPRITESHEET_WIDTH + x];
			
			char hex[2] = {0};
			_bytes_to_hex((uint8_t *)&color, 1, hex);
			string_builder_append(&builder, (string_t){.data = hex, .len = sizeof(color_t) * 2});
		}
		string_builder_append(&builder, STR("\n"));
	}
	string_builder_append(&builder, STR("\n"));

	// Sprite flags and color keys
	string_builder_append(&builder, STR("__spr__\n"));
	for (size_t i = 0; i < TOTAL_SPRITES; i++) {
		char hex[sizeof(sprite_t) * 2] = {0};
		_bytes_to_hex((uint8_t *)&computer->ram->sprites[i], sizeof(sprite_t), hex);
		string_builder_append(&builder, (string_t){.data = hex, .len = sizeof(sprite_t) * 2});
	}
	string_builder_append(&builder, STR("\n\n"));

	// Map
	string_builder_append(&builder, STR("__map__\n"));
	for (int i = 0; i < MAP_LAYERS_AMOUNT; i++) {
		for (int y = 0; y < MAP_HEIGHT; y++) {
			for (int x = 0; x < MAP_WIDTH; x++) {
				char hex[sizeof(uint16_t) * 2] = {0};
				_bytes_to_hex((uint8_t *)(&computer->ram->map.layers[i].data[y * MAP_WIDTH + x]), sizeof(uint16_t), hex);
				string_builder_append(&builder, (string_t){.data = hex, .len = sizeof(uint16_t) * 2});
			}
			string_builder_append(&builder, STR("\n"));
		}
	}

	// Write it to disk
	file_write_string(path, builder.string);

	string_builder_deinit(&builder);

	printf("Game saved!\n");
}

static string_t _file_load_to_string(allocator_t allocator, string_t path) {
	FILE *file = fopen(string_to_c_string(get_temp_allocator(), path), "r");
	if (file == NULL) {
		printf("Unable to open file\n");
		return (string_t){.data = NULL, .len = 0};
	}

	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	string_t string = {
		.data = alloc(allocator, file_size),
		.len = file_size,
	};
	assert(string.data != NULL);

	size_t read_len = fread(string.data, sizeof(char), file_size, file);
	// assert(read_len == file_size);
	
	fclose(file);

	return string;
}

static uint8_t _hex_char_to_value(char c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	}

	if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	}

	return 0;
}

static int _hex_string_to_raw(string_t hex_string, uint8_t buffer[], size_t size) {
	// The string is too small
	if (hex_string.len < size * 2) {
		return 1;
	}

	// The string is too big
	if (hex_string.len > size * 2) {
		return 1;
	}

	for (size_t i = 0; i < size; i++) {
		// Get the first c
		uint8_t high = _hex_char_to_value(hex_string.data[i * 2]);
		uint8_t low = _hex_char_to_value(hex_string.data[i * 2 + 1]);
		buffer[i] = (high << 4) | low;
	}

	return 0;
}

int game_load(computer_t *computer, string_t path) {
	if (path_is_file(path)) {
		set_game_path(computer, path);
	} else {
		return 1;
	}

	printf("Loading game at ");
	print_string(path);
	printf("\n");

	// Deinit all files
	for (size_t i = 0; i < FILES_AMOUNT; i++) {
		file_clear(&computer->files[i]);
	}

	enum {
		SECTION_LUA,
		SECTION_GFX,
		SECTION_SPR,
		SECTION_MAP,
	} current_section = SECTION_LUA;

	string_t string = _file_load_to_string(get_heap_allocator(), path);

	string_t_array_t lines = string_split(get_heap_allocator(), string, '\n');

	size_t gfx_offset = 0;
	size_t spr_offset = 0;
	size_t map_offset = 0;

	computer->active_files_amount = 0;

	bool last_line_was_seperator = true;

	size_t last_file_start = 0;

	for (size_t i = 0; i < lines.len; i++) {
		string_t line_string = lines.data[i];

		if (string_eq(line_string, STR("__gfx__"))) {
			// Remove last newline
			file_t *file = &computer->files[computer->active_files_amount];
			file_remove_section(file, file->string.len - 1, 1);

			current_section = SECTION_GFX;
			continue;
		}

		if (string_eq(line_string, STR("__spr__"))) {
			current_section = SECTION_SPR;
			continue;
		}

		if (string_eq(line_string, STR("__map__"))) {
			current_section = SECTION_MAP;
			continue;
		}

		if (current_section != SECTION_LUA && line_string.len == 0) {
			printf("line empty?\n");
			continue;
		}

		if (current_section == SECTION_LUA) {
			bool is_seperator = string_eq(line_string, STR("-->8"));

			if (is_seperator && !last_line_was_seperator) {
				// Remove last newline
				file_t *file = &computer->files[computer->active_files_amount];
				file_remove_section(file, file->string.len - 1, 1);
				
				computer->active_files_amount++;
				last_line_was_seperator = is_seperator;

				continue;
			}

			last_line_was_seperator = is_seperator;
		}

		switch (current_section) {
			case SECTION_LUA: {
				// Add the line directly to the text file data structure
				file_t *file = &computer->files[computer->active_files_amount];
				file_append_string(file, line_string);
				file_append_string(file, STR("\n"));
				break;
			}

			case SECTION_GFX: {
				if (_hex_string_to_raw(line_string, (uint8_t *)computer->ram->spritesheet.data + gfx_offset, SPRITESHEET_PAGE_WIDTH * sizeof(color_t)) > 0) {
					printf("Line %zu in section __gfx__ does not have the correct size\n", i);
				}
				gfx_offset += line_string.len / 2;
				break;
			}

			case SECTION_SPR: {
				if (_hex_string_to_raw(line_string, (uint8_t *)computer->ram->sprites, TOTAL_SPRITES * sizeof(sprite_t)) > 0) {
					printf("Line %zu in section __spr__ does not have the correct size\n", i);
				}
				break;
			}

			case SECTION_MAP: {
				if (_hex_string_to_raw(line_string, (uint8_t *)(computer->ram->map.layers[0].data) + map_offset, MAP_WIDTH * sizeof(uint16_t)) > 0) {
					printf("Line %zu in section __map__ does not have the correct size\n", i);
				}
				map_offset += line_string.len / 2;
				break;
			}
		}
	}

	if (!last_line_was_seperator) {
		computer->active_files_amount++;
	}

	array_deinit(&lines);

	dealloc(get_heap_allocator(), string.data);

	printf("Game loaded!\n");

	return 0;
}
