#include "computer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "res.h"
#include "api/lua_api.h"

static computer_t *_computer;

void set_global_computer(computer_t *computer) {
	_computer = computer;
}

computer_t *get_global_computer() {
	return _computer;
}

void computer_load_assets(computer_t *computer) {
	// memcpy(computer->ram->spritesheet.data, builtin_spritesheet, SPRITESHEET_PAGE_WIDTH * SPRITESHEET_PAGE_HEIGHT);
	memcpy(computer->ram->spritesheet.data + ((SPRITESHEET_PAGE_WIDTH * SPRITESHEET_PAGE_HEIGHT) * (SPRITESHEET_PAGE_AMOUNT - 1)), builtin_spritesheet, SPRITESHEET_PAGE_WIDTH * SPRITESHEET_PAGE_HEIGHT);

	computer->ram->fonts[0] = (font_t){
		.sprite_index = 5376,
		.horizontal_space = 1,
		.vertical_space = 3,
		.width = 8,
		.height = 10,
		.h_sprites = 1,
		.v_sprites = 2,

		.monospace = false,
		.widths = {
			4, //  
			1, // !
			3, // "
			6, // #
			5, // $
			7, // %
			5, // &
			1, // '
			2, // (
			2, // )
			3, // *
			5, // +
			2, // ,
			4, // -
			1, // .
			4, // /

			5, // 0
			5, // 1
			5, // 2
			5, // 3
			5, // 4
			5, // 5
			5, // 6
			5, // 7
			5, // 8
			5, // 9
			1, // :
			2, // ;
			5, // <
			5, // =
			5, // >
			5, // ?

			8, // @
			7, // A
			5, // B
			6, // C
			6, // D
			5, // E
			5, // F
			6, // G
			6, // H
			1, // I
			4, // J
			6, // K
			5, // L
			7, // M
			6, // N
			6, // O

			6, // P
			6, // Q
			6, // R
			5, // S
			5, // T
			6, // U
			7, // V
			5, // W
			7, // X
			7, // Y
			7, // Z
			2, // [
			4, // backslash fuck it
			2, // ]
			5, // ^
			5, // _

			3, // `
			5, // a
			5, // b
			5, // c
			5, // d
			5, // e
			2, // f
			5, // g
			5, // h
			1, // i
			2, // j
			5, // k
			1, // l
			7, // m
			5, // n
			5, // o

			5, // p
			5, // q
			3, // r
			4, // s
			2, // t
			5, // u
			5, // v
			7, // w
			4, // x
			5, // y
			4, // z
			3, // {
			1, // |
			3, // }
			6, // ~
		},
	};

	computer->ram->fonts[1] = (font_t){
		.sprite_index = 5568,
		.horizontal_space = 1,
		.vertical_space = 0,
		.monospace = true,
		.width = 6,
		.height = 10,
		.h_sprites = 1,
		.v_sprites = 2,
	};

	computer->ram->fonts[2] = (font_t){
		.sprite_index = 5760,
		.horizontal_space = 0,
		.vertical_space = 0,
		.monospace = true,
		.width = 8,
		.height = 8,
		.h_sprites = 1,
		.v_sprites = 1,
	};
}

void computer_init(computer_t *computer) {
	// Allocate 8MB ram
	computer->ram = malloc(RAM_SIZE);
	if (computer->ram == NULL) {
		printf("Couldn't allocate memory for fantasy RAM.\n");
		exit(EXIT_FAILURE);
	}
	memset(computer->ram, 0, RAM_SIZE);

	computer->ram->palette = default_palette;
}

void code_free(code_t *code) {
	for (int i = 0; i < code->line_amount; i++) {
		free(code->lines[i].text);
	}
	free(code->lines);
}

void computer_quit(computer_t *computer) {
	// Free the code first
	code_free(&computer->code);

	free(computer->ram);
}

void generate_rgb_framebuffer(computer_t *computer) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			uint8_t pixel = computer->ram->framebuffer.data[y * SCREEN_WIDTH + x];
			computer->rgb_framebuffer[y * SCREEN_WIDTH + x] = computer->ram->palette.colors[pixel];
		}
	}
}

bool point_in_bounds(int x, int y) {
	if (x < 0) return false;
	if (x >= SCREEN_WIDTH) return false;
	if (y < 0) return false;
	if (y >= SCREEN_HEIGHT) return false;

	return true;
}

void set_pixel(computer_t *computer, int x, int y, int color) {
	if (point_in_bounds(x, y)) {
		computer->ram->framebuffer.data[y * SCREEN_WIDTH + x] = color;
	}
}

bool point_in_rect(int x, int y, rect_t rect) {
	if (x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h) {
		return true;
	}

	return false;
}

// Convert the code_t datastructure back to a string for saving
// the function assumes that passed buffer is large enough
static size_t code_to_string(code_t *code, char *buffer) {
	size_t offset = 0;

	for (int i = 0; i < code->line_amount; i++) {
		size_t line_len = strlen(code->lines[i].text);
		
		memcpy(buffer + offset, code->lines[i].text, (line_len + 1) * sizeof(char));
		
		if (i < code->line_amount - 1) {
			buffer[offset + line_len] = '\n';
		} else {
			buffer[offset + line_len] = '\0';
		}
		offset += line_len + 1;
	}

	return offset;
}

void play_game(computer_t *computer) {
	// Convert code to string
	code_to_string(&computer->code, computer->ram->code_buffer);

	// Init the lua stuff
	lua_init(computer);
	lua_call_init();

	// Set the state
	computer->state = STATE_PLAYING;
}

void quit_game(computer_t *computer) {
	computer->state = STATE_EDITING;
	
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

void draw_sprite_sheet_rect(computer_t *computer, int x, int y, rect_t rect, uint8_t color_key) {
	for (int i = 0; i < rect.h; i++) {
		for (int j = 0; j < rect.w; j++) {
			uint8_t color = computer->ram->spritesheet.data[(rect.y + i) * SPRITESHEET_WIDTH + (rect.x + j)];
			if (color != color_key) {
				set_pixel(computer, x + j, y + i, color);
			}
		}
	}
}

void draw_sprite_sheet_rect_scaled(computer_t *computer, int x, int y, rect_t rect, uint8_t color_key, int scale) {
	for (int i = 0; i < rect.h; i++) {
		for (int j = 0; j < rect.w; j++) {
			uint8_t color = computer->ram->spritesheet.data[(rect.y + i) * SPRITESHEET_WIDTH + (rect.x + j)];
			// api_rectf(computer, x + (j * scale), y + (i * scale), scale, scale, color);
			rect_t new_rect = {x + (j * scale), y + (i * scale), scale, scale};
			draw_filled_rectangle(computer, new_rect, color);
		}
	}
}

void draw_filled_rectangle(computer_t *computer, rect_t rect, uint8_t color) {
	// Using i and j to avoid conflict with the x and y parameters
	for (int i = rect.y; i < rect.y+rect.h; i++) {
		for (int j = rect.x; j < rect.x+rect.w; j++) {
			set_pixel(computer, j, i, color);
		}
	}
}

rect_t sprite_index_to_spritesheet_rect(ram_t *ram, int sprite_index, int w, int h) {
	rect_t rect = {
		.x = (sprite_index % SPRITES_PER_ROW) * SPRITE_WIDTH,
		.y = (sprite_index / SPRITES_PER_ROW) * SPRITE_HEIGHT,
		.w = w * SPRITE_WIDTH,
		.h = h * SPRITE_HEIGHT,
	};

	return rect;
}

int get_text_width(font_t *font, char text[], int max_offset) {
	int len = 0;

	for (int i = 0; i < max_offset; i++) {
		if (text[i] == '\t') {
			if (font->monospace) {
				len += (font->width + font->horizontal_space) * TAB_SIZE;
			} else {
				len += (font->widths[text[' '] - VISIBLE_CHARACTERS_START] + font->horizontal_space) * TAB_SIZE;
			}

			continue;
		}

		if (font->monospace) {
			len += font->width + font->horizontal_space;
		} else {
			len += font->widths[text[i] - VISIBLE_CHARACTERS_START] + font->horizontal_space;
		}
	}

	return len;
}

int x_to_text_index(font_t *font, char text[], int x) {
	int index = x / (font->width + font->horizontal_space);
	int len = strlen(text);

	int real_index = index;

	for (int i = 0; i < index && i < len; i++) {
		if (text[i] == '\t') {
			real_index -= TAB_SIZE - 1;
		}

		if (real_index < 0) {
			real_index = 0;
			break;
		}
	}
	
	
	if (real_index >= len) {
		real_index = len;
	}

	return real_index;
}

#define LUA_SECTION_STRING "<<< lua >>>\n"
#define GFX_SECTION_STRING "<<< gfx >>>\n"
#define SPR_SECTION_STRING "<<< spr >>>\n"
#define SECTION_END_STRING ">>> --- <<<\n"

void game_save(computer_t *computer, const char filename[]) {
	// Since we zero-initialize we don't need a \0 at the end (but I still do)
	char *buffer = calloc(RAM_SIZE, sizeof(char));
	size_t offset = 0;

	// Lua code
	offset += code_to_string(&computer->code, buffer + offset);
	// Replace \0 with \n so the string doesn't terminate
	buffer[offset - 1] = '\n';

	// Lua comment start
	strncpy(buffer + offset, "--[[\n", 5);
	offset += 5;

	// Spritesheet
	strncpy(buffer + offset, "<<< gfx >>>\n", 12);
	offset += 12;
	for (int y = 0; y < SPRITESHEET_HEIGHT; y++) {
		for (int x = 0; x < SPRITESHEET_WIDTH; x++) {
			uint8_t color = computer->ram->spritesheet.data[y * SPRITESHEET_WIDTH + x];
			// Print the color in hex
			sprintf(buffer + offset, "%02x", color);
			offset += 2;
		}
		buffer[offset] = '\n';
		offset++;
	}
	strncpy(buffer + offset, ">>> --- <<<\n\n", 13);
	offset += 13;
	
	// Sprite meta
	strncpy(buffer + offset, "<<< spr >>>\n", 12);
	offset += 12;

	// TODO: rows and columns
	for (int i = 0; i < TOTAL_SPRITES; i++) {
		// 4 bytes of flags
		// 1 byte of colorkey
		sprintf(buffer + offset, "%08x", computer->ram->sprites[i].flags);
		offset += 8;

		sprintf(buffer + offset, "%02x", computer->ram->sprites[i].color_key);
		offset += 2;
	}
	buffer[offset] = '\n';
	offset++;
	
	strncpy(buffer + offset, ">>> --- <<<\n", 12);
	offset += 12;

	strncpy(buffer + offset, "--]]\n", 5);
	offset += 5;
	
	// Null terminate the string
	buffer[offset] = '\0';

	// Write to file
	FILE *file = fopen(filename, "w");
	fprintf(file, buffer);
	fclose(file);

	free(buffer);
}

typedef enum file_section {
	SECTION_NONE,
	SECTION_LUA,
	SECTION_GFX,
	SECTION_SPR,
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
				// TODO: Slightly unsafe since I'm still relying on null-termination, might rewrite
				strncpy(computer->ram->code_buffer + code_offset, line, len);
				code_offset += len;

				computer->ram->code_buffer[code_offset] = '\0';

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
		}
	}

	fclose(file);

	free(line);
}