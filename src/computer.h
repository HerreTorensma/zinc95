#pragma once

#define BACKEND_SDL2

#include <inttypes.h>
#include <stdbool.h>

#define RAM_SIZE (8 * 1024 * 1024)
#define CODE_SIZE (8 * 1024 * 1024)

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define PALETTE_SIZE 256

// These are in pixels
#define SPRITE_SHEET_WIDTH 256
#define SPRITE_SHEET_HEIGHT 256

#define VISIBLE_CHARACTERS_SIZE 96
#define VISIBLE_CHARACTERS_START 32
#define MAX_CHARACTER_WIDTH 16
#define MAX_CHARACTER_HEIGHT 16

#define TAB_SIZE 4

#define FPS 60

typedef struct rect {
	int x;
	int y;
	int w;
	int h;
} rect_t;

typedef struct rgb_color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_color_t;

typedef struct palette {
	rgb_color_t colors[PALETTE_SIZE];
} palette_t;

typedef struct framebuffer {
	uint8_t data[SCREEN_WIDTH * SCREEN_HEIGHT];
} framebuffer_t;

// Rename every occurance of sprite_sheet to spritesheet
typedef struct spritesheet {
	uint16_t sprite_width;
	uint16_t sprite_height;
	uint8_t data[SPRITE_SHEET_WIDTH * SPRITE_SHEET_HEIGHT];
} spritesheet_t;

typedef struct font_meta {
	uint8_t sprite_sheet_index;
	// Index of first character, space ( )
	uint8_t sprite_index;

	uint8_t width;
	uint8_t height;
	uint8_t horizontal_space;
	uint8_t vertical_space;

	// Bool
	uint8_t monospace;
	uint8_t widths[VISIBLE_CHARACTERS_SIZE];
} font_meta_t;

typedef struct line {
	char *text;
} line_t;

typedef struct code {
	line_t *lines;
	uint64_t line_amount;
	
	uint64_t cursor_line;
	uint64_t cursor_pos;
	uint64_t target_pos;
} code_t;

// 8MB RAM (excluding what the lua code takes up)
typedef union ram {
	struct {
		framebuffer_t framebuffer;
		palette_t palette;
		spritesheet_t spritesheets[12];
		font_meta_t fonts[8];
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
	uint64_t ticks;
} computer_t;

void set_global_computer(computer_t *computer);

computer_t *get_global_computer();

// This function currently only allocates memory for the fantasy ram
void computer_init(computer_t *computer);

// Generate a buffer of rgb_color_t using palette so it can be rendered by a backend later
void generate_rgb_framebuffer(computer_t *computer);

// Check if a given point is within the bounds of the framebuffer
bool point_in_bounds(int x, int y);

// Set a pixel in the framebuffer
void set_pixel(computer_t *computer, int x, int y, int color);

// Check if an x, y coordinate is inside a given rect
bool point_in_rect(int x, int y, rect_t rect);

// Play the currently loaded game
void play_game(computer_t *computer);

// Quit the currently loaded game
void quit_game(computer_t *computer);

int sprite_x_to_sprite_sheet_x(ram_t *ram, int sprite_sheet_index, int sprite_index, int x);

int sprite_y_to_sprite_sheet_y(ram_t *ram, int sprite_sheet_index, int sprite_index, int y);

int sprite_get_pixel(ram_t *ram, int sprite_sheet_index, int sprite_index, int x, int y);

void sprite_set_pixel(ram_t *ram, int sprite_sheet_index, int sprite_index, int x, int y, uint8_t color);

void draw_sprite_sheet_rect(computer_t *computer, int sprite_sheet_index, int x, int y, rect_t rect);

rect_t sprite_to_spritesheet_rect(ram_t *ram, int sprite_sheet_index, int sprite_index, int w, int h);