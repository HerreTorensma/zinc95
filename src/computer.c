#include "computer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "res.h"
#include "api/lua_api.h"
#include "common/mem.h"
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
		.background_color = COLOR_WHITE,
		.font_index = 2,
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

void computer_init(computer_t *computer) {
	computer->ram = malloc(RAM_SIZE);
	if (computer->ram == NULL) {
		printf("Couldn't allocate memory for fantasy RAM.\n");
		exit(EXIT_FAILURE);
	}
	memset(computer->ram, 0, RAM_SIZE);

	computer->ram->palette = builtin_palette;
}

void computer_quit(computer_t *computer) {
	// Free the code first
	file_free(&computer->file);

	// free(computer->code_buffer);

	free(computer->ram);
}

// Forward declaration so I don't have cyclic dependencies
string_t file_to_string(file_t *file, allocator_t allocator);

void play_game(computer_t *computer) {
	// Convert code to string
	// file_to_string(&computer->file, computer->code_buffer);
	// string_t code = file_to_string(&computer->file, );

	// Init the lua stuff
	lua_init(computer);
	lua_call_init();

	// Reset draw state
	// memset(&computer->ram->draw_state, 0, sizeof(draw_state_t));

	// Set the state
	computer->game_running = true;
	computer->state = STATE_IN_GAME;
}

void shell_new_command(ram_t *ram);

void quit_game(computer_t *computer) {
	computer->game_running = false;

	shell_new_command(computer->ram);
	
	lua_quit();
}

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

#define LUA_SECTION_STRING "<<< lua >>>\n"
#define GFX_SECTION_STRING "<<< gfx >>>\n"
#define SPR_SECTION_STRING "<<< spr >>>\n"
#define MAP_SECTION_STRING "<<< map >>>\n"
#define SECTION_END_STRING ">>> --- <<<\n"

static const char _hex_chars[] = "0123456789abcdef";

// hex should be twice as big as bytes
static void _bytes_to_hex(uint8_t bytes[], size_t len, char hex[]) {
	for (size_t i = 0; i < len; i++) {
		hex[i * 2] = _hex_chars[(bytes[i] >> 4) & 0x0f]; \
		hex[i * 2 + 1] = _hex_chars[(bytes[i] & 0x0f)]; \
	}
}

// #define STRING_BUILDER_APPEND_BYTE_AS_HEX(builder, byte) \
// 	do { \
// 		char buffer[2] = {0}; \
// 		buffer[0] = _hex_chars[(byte >> 4) & 0x0f]; \
// 		buffer[1] = _hex_chars[(byte & 0x0f)]; \
// 		string_builder_append(builder, (string_t){.data = buffer, .len = 2}); \
// 	} while (0);

// TODO: some kind of get allocator function that references stack memory
// Or create an arena for the hex encoding stuff

// New implementation with length based strings
void game_save(computer_t *computer, string_t filename) {
	printf("Saving game...\n");

	string_builder_t builder = {0};

	// 8MB should be enough for most games
	// Also make it heap allocated to not overload the temporary memory
	string_builder_init(&builder, get_heap_allocator(), MB(8));

	// Lua code
	string_t code = file_to_string(&computer->file, get_heap_allocator());
	string_builder_append(&builder, code);
	dealloc(get_heap_allocator(), code.data);

	// The rest of the game is inside a lua multiline comment so it can be opened in an IDE without syntax errors
	string_builder_append(&builder, STR("--[[\n"));

	// Spritesheet
	string_builder_append(&builder, STR("<<< gfx >>>\n"));
	for (size_t y = 0; y < SPRITESHEET_HEIGHT; y++) {
		for (size_t x = 0; x < SPRITESHEET_WIDTH; x++) {
			color_t color = computer->ram->spritesheet.data[y * SPRITESHEET_WIDTH + x];
			
			char hex[2] = {0};
			_bytes_to_hex((uint8_t *)&color, 1, hex);
			string_builder_append(&builder, (string_t){.data = hex, .len = 2});

			// STRING_BUILDER_APPEND_BYTE_AS_HEX(&builder, color);
		}
		string_builder_append(&builder, STR("\n"));
	}
	string_builder_append(&builder, STR(">>> --- <<<\n\n"));

	// Sprite flags and color keys
	string_builder_append(&builder, STR("<<< spr >>>\n"));
	for (size_t i = 0; i < TOTAL_SPRITES; i++) {
		char hex[sizeof(sprite_t) * 2] = {0};
		_bytes_to_hex((uint8_t *)&computer->ram->sprites[i], sizeof(sprite_t), hex);
		string_builder_append(&builder, (string_t){.data = hex, .len = 2});
	}
	string_builder_append(&builder, STR("\n"));
	string_builder_append(&builder, STR(">>> --- <<<\n\n"));

	// Map
	string_builder_append(&builder, STR("<<< map >>>\n"));
	for (int i = 0; i < MAP_LAYERS_AMOUNT; i++) {
		for (int y = 0; y < MAP_HEIGHT; y++) {
			for (int x = 0; x < MAP_WIDTH; x++) {
				char hex[sizeof(uint16_t) * 2] = {0};
				_bytes_to_hex((uint8_t *)&computer->ram->map.layers[i].data, sizeof(uint16_t), hex);
				string_builder_append(&builder, (string_t){.data = hex, .len = 2});
			}
			string_builder_append(&builder, STR("\n"));
		}
	}
	string_builder_append(&builder, STR(">>> --- <<<\n\n"));

	// End of lua comment
	string_builder_append(&builder, STR("--]]\n"));

	// Write it to disk
	FILE *file = fopen(string_to_c_string(get_temp_allocator(), filename), "w");
	fwrite(builder.string.data, sizeof(char), builder.string.len, file);
	fclose(file);

	string_builder_deinit(&builder);

	printf("Game saved!\n");
}

typedef enum file_section {
	SECTION_NONE,
	SECTION_LUA,
	SECTION_GFX,
	SECTION_SPR,
	SECTION_MAP,
} file_section_t;

#define LINE_SIZE RAM_SIZE

void game_load(computer_t *computer, const char filename[]) {
	char *line = calloc(LINE_SIZE, sizeof(char));

	file_section_t current_section = SECTION_LUA;

	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		printf("Unable to open file\n");
		return;
	}

	size_t code_offset = 0;
	size_t spritesheet_offset = 0;
	size_t map_offset = 0;

	// Read every line
	while (fgets(line, LINE_SIZE * sizeof(char), file)) {
		if (strcmp(line, "--[[\n") == 0) {
			current_section = SECTION_NONE;
		}

		// Update the current section
		// Start of section
		if (line[0] == '<' && line[1] == '<' && line[2] == '<') {
			if (strcmp(line, LUA_SECTION_STRING) == 0) {
				current_section = SECTION_LUA;
				continue;
			} else if (strcmp(line, GFX_SECTION_STRING) == 0) {
				current_section = SECTION_GFX;
				continue;
			} else if (strcmp(line, SPR_SECTION_STRING) == 0) {
				current_section = SECTION_SPR;
				continue;
			} else if (strcmp(line, MAP_SECTION_STRING) == 0) {
				current_section = SECTION_MAP;
				continue;
			}
		}

		// End of section
		else if (line[0] == '>' && line[1] == '>' && line[2] == '>') {
			if (strcmp(line, SECTION_END_STRING) == 0) {
				current_section = SECTION_NONE;
				continue;
			}
		}

		// Actually read data
		switch (current_section) {
			case SECTION_NONE: {
				// Continue to next line
				continue;
			}

			case SECTION_LUA: {
				// Read line into buffer
				size_t len = strlen(line);

				if (code_offset + len >= (1024 * 1024 * sizeof(char))) {
					fprintf(stderr, "Buffer overflow at offset %zu with len %zu\n", code_offset, len);
					break;
				}

				// TODO: Slightly unsafe since I'm still relying on null-termination, might rewrite
				strncpy(computer->code_buffer + code_offset, line, len);
				// strcpy(computer->code_buffer + code_offset, line);
				
				code_offset += len;
				computer->code_buffer[code_offset] = '\0';

				break;
			}

			case SECTION_GFX: {
				// Read line into spritesheet
				for (int i = 0; i < SPRITESHEET_WIDTH; i++) {
					sscanf(line + (i * 2), "%02x", &computer->ram->spritesheet.data[spritesheet_offset]);
					spritesheet_offset++;
				}
				break;
			}

			case SECTION_SPR: {
				// Read line into sprites
				for (int i = 0; i < TOTAL_SPRITES; i++) {
					sscanf(line + (i * 10), "%08x", &computer->ram->sprites[i].flags);
					sscanf(line + (i * 10 + 8), "%02x", &computer->ram->sprites[i].color_key);
				}
				break;
			}

			case SECTION_MAP: {
				for (int i = 0; i < MAP_WIDTH; i++) {
					sscanf(line + (i * 4), "%04x", &computer->ram->map.layers[0].data[map_offset]);
					map_offset++;
				}
				break;
			}
		}
	}

	fclose(file);

	free(line);
}

const skin_layout_t skin_layout = {
	.code_button = {
		.unpressed_rect = {{2560, 44, 64, 16}},
		.pressed_rect = {{2560, 60, 64, 16}},
	},

	.sprite_button = {
		.unpressed_rect = {{2624, 44, 64, 16}},
		.pressed_rect = {{2624, 60, 64, 16}},
	},

	.map_button = {
		.unpressed_rect = {{2688, 44, 64, 16}},
		.pressed_rect = {{2688, 60, 64, 16}},
	},

	.sound_button = {
		.unpressed_rect = {{2752, 44, 64, 16}},
		.pressed_rect = {{2752, 60, 64, 16}},
	},

	.save_button = {
		.unpressed_rect = {{2816, 44, 16, 16}},
		.pressed_rect = {{2816, 60, 16, 16}},
	},

	.play_button = {
		.unpressed_rect = {{2832, 44, 16, 16}},
		.pressed_rect = {{2832, 60, 16, 16}},
	},

	.stop_button = {
		.unpressed_rect = {{2848, 44, 16, 16}},
		.pressed_rect = {{2848, 60, 16, 16}},
	},

	.sprite_flag_buttons = {
		.base = {
			.unpressed_rect = {{2560, 20, 12, 12}},
			.pressed_rect = {{2560, 32, 12, 12}},
		},
		.increase = {12, 0},
		.amount = 32,
	},

	.color_key_button = {
		.unpressed_rect = {{2944, 20, 12, 12}},
		.pressed_rect = {{2944, 32, 12, 12}},
	},

	.spritesheet_page_buttons = {
		.base = {
			.unpressed_rect = {{2560, 76, 48, 16}},
			.pressed_rect = {{2608, 76, 48, 16}},
		},
		.increase = {0, 16},
		.amount = 8,
	},

	.sprite_tool_buttons = {
		.base = {
			.unpressed_rect = {{2560, 204, 16, 16}},
			.pressed_rect = {{2560, 220, 16, 16}},
		},
		.increase = {16, 0},
		.amount = 7,
	},

	.map_entity_layer_button = {
		.unpressed_rect = {{2560, 236, 48, 16}},
		.pressed_rect = {{2608, 236, 48, 16}},
	},

	.map_layer_buttons = {
		.base = {
			.unpressed_rect = {{2560, 252, 48, 16}},
			.pressed_rect = {{2608, 252, 48, 16}},
		},
		.increase = {0, 16},
		.amount = 4,
	},

	.gui_font_rect = {{2560, 432, 384, 32}},
	.code_editor_font_rect = {{2560, 464, 384, 16}},
};
