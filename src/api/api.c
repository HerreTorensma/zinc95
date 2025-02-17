#include "api.h"

#include <stdio.h>
#include <math.h>

#include "../backend/input.h"

void api_cls(computer_t *computer, int color) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			computer->ram->framebuffer.data[y * SCREEN_WIDTH + x] = color;
		}
	}
}

static void draw_sprite(computer_t *computer, int index, int x, int y) {
	for (int i = 0; i < SPRITE_HEIGHT; i++) {
		for (int j = 0; j < SPRITE_WIDTH; j++) {
			uint8_t color = computer->ram->spritesheet.sprites[index].data[i * SPRITE_WIDTH + j];
			set_pixel(computer, x + j, y + i, color);
		}
	}
}

static void draw_sprite_scaled(computer_t *computer, int index, int x, int y, int scale) {
	for (int i = 0; i < SPRITE_HEIGHT; i++) {
		for (int j = 0; j < SPRITE_WIDTH; j++) {
			uint8_t color = computer->ram->spritesheet.sprites[index].data[i * SPRITE_WIDTH + j];
			api_rectf(computer, x + (j * scale), y + (i * scale), scale, scale, color);
		}
	}
}

void api_spr(computer_t *computer, int index, int x, int y, int width, int height) {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int new_index = index + i * SPRITE_SHEET_WIDTH + j;
			draw_sprite(computer, new_index, x + j, y + i);
		}
	}
}

void api_sspr(computer_t *computer, int index, int x, int y, int width, int height, int scale) {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int new_index = index + i * SPRITE_SHEET_WIDTH + j;
			draw_sprite_scaled(computer, new_index, x + j, y + i, scale);
		}
	}
}

// Using Bresemham's line algorithm
void api_line(computer_t *computer, int x1, int y1, int x2, int y2, int color) {
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int step_x = (x1 < x2) ? 1 : -1;
	int step_y = (y1 < y2) ? 1 : -1;
	int error = dx - dy;

	while (true) {
		set_pixel(computer, x1, y1, color);
		
		if (x1 == x2 && y1 == y2) break;

		int e2 = 2 * error;
		if (e2 > -dy) {
			error -= dy;
			x1 += step_x;
		}
		if (e2 < dx) {
			error += dx;
			y1 += step_y;
		}
	}
}

void api_rect(computer_t *computer, int x, int y, int w, int h, int color) {
	for (int j = x; j < x+w; j++) {
		set_pixel(computer, j, y, color);
	}

	for (int j = x; j < x+w; j++) {
		set_pixel(computer, j, y+h-1, color);
	}

	for (int i = y; i < y+h; i++) {
		set_pixel(computer, x, i, color);
	}

	for (int i = y; i < y+h; i++) {
		set_pixel(computer, x+w - 1, i, color);
	}
}

void api_rectf(computer_t *computer, int x, int y, int w, int h, int color) {
	// Using i and j to avoid conflict with the x and y parameters
	for (int i = y; i < y+h; i++) {
		for (int j = x; j < x+w; j++) {
			set_pixel(computer, j, i, color);
		}
	}
}

bool api_key(computer_t *computer, int key) {
	return input_key_held(key);
}

bool api_keyp(computer_t *computer, int key) {
	return input_key_pressed(key);
}

bool api_keyr(computer_t *computer, int key) {
	return input_key_released(key);
}

bool api_mouse_btn(computer_t *computer, int button) {
	return input_mouse_button_held(button);
}

bool api_mouse_btnp(computer_t *computer, int button) {
	return input_mouse_button_pressed(button);
}

bool api_mouse_btnr(computer_t *computer, int button) {
	return input_mouse_button_released(button);
}