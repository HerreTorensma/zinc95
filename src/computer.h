/*
Computer

Memory layout, global constants
*/

#pragma once

#define BACKEND_SDL2

#include <inttypes.h>
#include <stdbool.h>

// TODO: rename some stuff so it's all consistent, dont mix AMOUNT, MAX, TOTAL etc.

#define RAM_SIZE (32 * 1024 * 1024)
#define CODE_SIZE (8 * 1024 * 1024)

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define PALETTE_SIZE 256

// These are in pixels
// TODO: put them in terms of sprite width and height
#define SPRITESHEET_PAGE_WIDTH 384
#define SPRITESHEET_PAGE_HEIGHT 128
#define SPRITESHEET_WIDTH SPRITESHEET_PAGE_WIDTH
// 8 pages of sprites
// #define SPRITESHEET_PAGE_AMOUNT 16
#define SPRITESHEET_PAGE_AMOUNT 8
#define SPRITESHEET_HEIGHT (SPRITESHEET_PAGE_HEIGHT * SPRITESHEET_PAGE_AMOUNT)

#define SPRITE_WIDTH 8
#define SPRITE_HEIGHT 8
#define SPRITES_PER_ROW (SPRITESHEET_WIDTH / SPRITE_WIDTH)
#define SPRITES_PER_PAGE ((SPRITESHEET_PAGE_WIDTH * SPRITESHEET_PAGE_HEIGHT) / (SPRITE_WIDTH * SPRITE_HEIGHT))
#define TOTAL_SPRITES ((SPRITESHEET_WIDTH * SPRITESHEET_HEIGHT) / (SPRITE_WIDTH * SPRITE_HEIGHT))

#define VISIBLE_CHARACTERS_SIZE 96
#define MAX_CHARACTER_WIDTH 16
#define MAX_CHARACTER_HEIGHT 16

// TODO: should be a static global variable in code.c to be configured later
#define TAB_SIZE 4

#define COLOR_NONE 255
#define COLOR_BLACK 0
#define COLOR_WHITE 15

#define MAP_LAYERS_AMOUNT 4
#define MAP_WIDTH 80 * 16
#define MAP_HEIGHT 60 * 16
#define MAP_LAYER_SIZE (MAP_WIDTH * MAP_HEIGHT)

#define FPS 60
#define FRAME_DELAY 1000/FPS

typedef uint8_t color_t;

typedef struct rgb_color {
	color_t r;
	color_t g;
	color_t b;
} rgb_color_t;

typedef struct palette {
	rgb_color_t colors[PALETTE_SIZE];
} palette_t;

typedef struct framebuffer {
	color_t data[SCREEN_WIDTH * SCREEN_HEIGHT];
} framebuffer_t;

#define SPRITE_FLAGS_SIZE 32
typedef struct sprite {
	uint32_t flags;
	// The last 8 colors are not used anyway so we can use 255 as the value for the sprite not having a color key
	// So we don't need a larger integer
	color_t color_key;
} sprite_t;

// Rename every occurance of sprite_sheet to spritesheet
typedef struct spritesheet {
	color_t data[SPRITESHEET_WIDTH * SPRITESHEET_HEIGHT];
} spritesheet_t;

typedef struct map_layer {
	uint16_t data[MAP_LAYER_SIZE];
} map_layer_t;

typedef struct map {
	map_layer_t layers[MAP_LAYERS_AMOUNT];
} map_t;

typedef struct font {
	// Index of first visible ASCII character, which is space ( )
	uint16_t sprite_index;

	uint8_t width;
	uint8_t height;
	uint8_t h_sprites;
	uint8_t v_sprites;
	uint8_t horizontal_space;
	uint8_t vertical_space;

	// Bool
	uint8_t monospace;
	uint8_t widths[VISIBLE_CHARACTERS_SIZE];
} font_t;

typedef struct line {
	char *text;
} line_t;

typedef struct code {
	line_t *lines;
	int line_amount;
	
	int cursor_line;
	int cursor_pos;
	int target_pos;
} code_t;

// Should be reset before game is played
// typedef struct draw_state {
// 	int16_t cam_pos_x;
// 	int16_t cam_pos_y;
// } draw_state_t;

// Colors used by GUI
typedef struct gui_colors {
	// Since the borders of the screen are not in the framebuffer it can just be an rgb color
	rgb_color_t screen_background;

	color_t text;
	color_t inset_frame_background;
	color_t outset_frame_background;

	// From dark to bright
	color_t frame_edge_darker;
	color_t frame_edge_dark;
	color_t frame_edge_neutral;
	color_t frame_edge_light;

	color_t toggle_button_set_text;
	color_t toggle_button_unset_text;
} gui_colors_t;

// 8MB RAM (excluding what the lua code takes up)
typedef union ram {
	struct {
		framebuffer_t framebuffer;
		palette_t palette;
		spritesheet_t spritesheet;
		sprite_t sprites[TOTAL_SPRITES];
		font_t fonts[8];
		map_t map;
		char code_buffer[1024 * 1024];
		// draw_state_t draw_state;
		gui_colors_t gui_colors;
		uint64_t ticks;
	};

	uint8_t data[RAM_SIZE];
} ram_t;

typedef enum computer_state {
	STATE_EDITING,
	STATE_PLAYING,
} computer_state_t;

typedef struct computer {
	rgb_color_t rgb_framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
	ram_t *ram;
	computer_state_t state;
	code_t code;
} computer_t;

void set_global_computer(computer_t *computer);

computer_t *get_global_computer();

void computer_load_resouces(computer_t *computer);

// This function currently only allocates memory for the fantasy ram
void computer_init(computer_t *computer);

void computer_quit(computer_t *computer);

// Play the currently loaded game
void play_game(computer_t *computer);

// Quit the currently loaded game
void quit_game(computer_t *computer);

/*
int sprite_x_to_sprite_sheet_x(ram_t *ram, int sprite_sheet_index, int sprite_index, int x);

int sprite_y_to_sprite_sheet_y(ram_t *ram, int sprite_sheet_index, int sprite_index, int y);

int sprite_get_pixel(ram_t *ram, int sprite_sheet_index, int sprite_index, int x, int y);

void sprite_set_pixel(ram_t *ram, int sprite_sheet_index, int sprite_index, int x, int y, uint8_t color);
*/

void game_save(computer_t *computer, const char filename[]);

void game_load(computer_t *computer, const char filename[]);