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

void computer_init(computer_t *computer) {
	// Allocate 8MB ram
	computer->ram = malloc(RAM_SIZE);
	if (computer->ram == NULL) {
		printf("Couldn't allocate memory for fantasy RAM.\n");
		exit(EXIT_FAILURE);
	}
	memset(computer->ram, 0, RAM_SIZE);

	computer->ram->palette = default_palette;

	computer->ram->spritesheets[0].sprite_width = 16;
	computer->ram->spritesheets[0].sprite_height = 16;

	computer->ram->spritesheets[9].sprite_width = 16;
	computer->ram->spritesheets[9].sprite_height = 16;
	computer->ram->spritesheets[10].sprite_width = 8;
	computer->ram->spritesheets[10].sprite_height = 8;

	memcpy(computer->ram->spritesheets[9].data, system_spritesheet0, SPRITE_SHEET_WIDTH * SPRITE_SHEET_HEIGHT);
	memcpy(computer->ram->spritesheets[10].data, system_spritesheet1, SPRITE_SHEET_WIDTH * SPRITE_SHEET_HEIGHT);

	computer->ram->fonts[0] = (font_meta_t){
		.sprite_sheet_index = 9,
		.sprite_index = 0,
		.horizontal_space = 1,
		.vertical_space = 3,
		.height = 10,

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
			3, // 1
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

			10, // @
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
			11, // W
			7, // X
			7, // Y
			7, // Z
			2, // [
			4, // backslash fuck it
			2, // ]
			5, // ^
			4, // _

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

	computer->ram->fonts[1] = (font_meta_t){
		.sprite_sheet_index = 10,
		.sprite_index = 0,
		.horizontal_space = 0,
		.vertical_space = 0,
		.monospace = true,
		.width = 8,
		.height = 8,
	};
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

void play_game(computer_t *computer) {
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

int sprite_x_to_sprite_sheet_x(ram_t *ram, int sprite_sheet_index, int sprite_index, int x) {
	int sprite_width = ram->spritesheets[sprite_sheet_index].sprite_width;

	int sprite_x = sprite_index % (SPRITE_SHEET_WIDTH / sprite_width);

	return (sprite_x * sprite_width) + x;
}

int sprite_y_to_sprite_sheet_y(ram_t *ram, int sprite_sheet_index, int sprite_index, int y) {
	int sprite_width = ram->spritesheets[sprite_sheet_index].sprite_width;
	int sprite_height = ram->spritesheets[sprite_sheet_index].sprite_height;

	int sprite_y = sprite_index / (SPRITE_SHEET_WIDTH / sprite_width);

	return (sprite_y * sprite_height) + y;
}

int sprite_get_pixel(ram_t *ram, int sprite_sheet_index, int sprite_index, int x, int y) {
	int sprite_sheet_x = sprite_x_to_sprite_sheet_x(ram, sprite_sheet_index, sprite_index, x);
	int sprite_sheet_y = sprite_y_to_sprite_sheet_y(ram, sprite_sheet_index, sprite_index, y);

	return ram->spritesheets[sprite_sheet_index].data[sprite_sheet_y * SPRITE_SHEET_WIDTH + sprite_sheet_x];
}

void sprite_set_pixel(ram_t *ram, int sprite_sheet_index, int sprite_index, int x, int y, uint8_t color) {
	int sprite_sheet_x = sprite_x_to_sprite_sheet_x(ram, sprite_sheet_index, sprite_index, x);
	int sprite_sheet_y = sprite_y_to_sprite_sheet_y(ram, sprite_sheet_index, sprite_index, y);

	ram->spritesheets[sprite_sheet_index].data[sprite_sheet_y * SPRITE_SHEET_WIDTH + sprite_sheet_x] = color;
}

void draw_sprite_sheet_rect(computer_t *computer, int sprite_sheet_index, int x, int y, rect_t rect) {
	for (int i = 0; i < rect.h; i++) {
		for (int j = 0; j < rect.w; j++) {
			uint8_t color = computer->ram->spritesheets[sprite_sheet_index].data[(rect.y + i) * SPRITE_SHEET_WIDTH + (rect.x + j)];
			set_pixel(computer, x + j, y + i, color);
		}
	}
}

rect_t sprite_to_spritesheet_rect(ram_t *ram, int sprite_sheet_index, int sprite_index, int w, int h) {
	int sprite_width = ram->spritesheets[sprite_sheet_index].sprite_width;
	int sprite_height = ram->spritesheets[sprite_sheet_index].sprite_height;
	
	int sprites_per_row = SPRITE_SHEET_WIDTH / sprite_width;

	rect_t rect = {
		.x = (sprite_index % sprites_per_row) * sprite_width,
		.y = (sprite_index / sprites_per_row) * sprite_height,
		.w = w * sprite_width,
		.h = h * sprite_height,
	};

	return rect;
}