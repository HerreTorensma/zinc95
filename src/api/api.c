#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../backend/input.h"

void api_cls(computer_t *computer, int color) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			computer->ram->framebuffer.data[y * SCREEN_WIDTH + x] = color;
		}
	}
}

// void api_spr(computer_t *computer, int sprite_index, int x, int y, int width, int height) {
// 	rect_t rect = sprite_index_to_spritesheet_rect(computer->ram, sprite_index, width, height);
// 	draw_sprite_sheet_rect(computer, x, y, rect, computer->ram->sprites[sprite_index].color_key);
// }

// void api_sspr(computer_t *computer, int sprite_index, int x, int y, int width, int height, int scale) {
// 	rect_t rect = sprite_index_to_spritesheet_rect(computer->ram, sprite_index, width, height);
// 	draw_sprite_sheet_rect_scaled(computer, x, y, rect, computer->ram->sprites[sprite_index].color_key, scale);
// }

// void api_spr(computer_t *computer, int sprite_index, int x, int y, int width, int height) {
// 	rect_t rect = sprite_index_to_spritesheet_rect(computer->ram, sprite_index, width, height);
// 	draw_sprite_sheet_rect(computer, x, y, rect, computer->ram->sprites[sprite_index].color_key);
// }

void api_spr(computer_t *computer, int sprite_index, int x, int y, int width, int height, int scale) {
	rect_t rect = sprite_index_to_spritesheet_rect(computer->ram, sprite_index, width, height);
	
	if (scale == 1) {
		draw_sprite_sheet_rect(computer, x, y, rect, computer->ram->sprites[sprite_index].color_key);
	} else {
		draw_sprite_sheet_rect_scaled(computer, x, y, rect, computer->ram->sprites[sprite_index].color_key, scale);
	}
}

void api_sspr(computer_t *computer, int x, int y, int rx, int ry, int rw, int rh, int color_key, int scale) {
	rect_t rect = {rx, ry, rw, rh};

	if (scale == 1) {
		draw_sprite_sheet_rect(computer, x, y, rect, color_key);
	} else {
		draw_sprite_sheet_rect_scaled(computer, x, y, rect, color_key, scale);
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
	draw_filled_rectangle(computer, (rect_t){x, y, w, h}, color);
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

bool api_mouse_scrolled(computer_t *computer, int direction) {
	return input_mouse_scrolled(direction);
}

void api_text(computer_t *computer, int font_index, char text[], int x, int y, int color) {
	font_t *font = &computer->ram->fonts[font_index];

	int new_x = x;
	int new_y = y;

	for (size_t i = 0; i < strlen(text); i++) {
		// Commented this out for now, might add it back later not sure yet
		if (text[i] == '\n') {
			new_x = 0;
			new_y += font->height + font->vertical_space;
			continue;
		}

		if (text[i] == '\t') {
			if (font->monospace) {
				new_x += (font->width + font->horizontal_space) * TAB_SIZE;
			} else {
				new_x += (font->widths[text[' '] - VISIBLE_CHARACTERS_START] + font->horizontal_space) * TAB_SIZE;
			}

			continue;
		}

		// Inline sprites
		if (text[i] == '~') {
			int sprite_index = 0;

			// Read the digits after
			size_t index = i + 1;
			while (text[index] >= '0' && text[index] <= '9') {
				sprite_index *= 10;
				sprite_index += text[index] - '0';

				index++;
			}

			// Only sprite sheet 0 now
			api_spr(computer, sprite_index, new_x, new_y, 1, 1, 1);
			new_x += 16 + font->horizontal_space;

			i = index - 1;

			continue;
		}

		// Get the correct sprite index keeping in mind some fonts could have multiple sprites per character (not tested for more than 1 horizontal sprite)
		int char_index = text[i] - VISIBLE_CHARACTERS_START;
		int x_offset = (char_index % (SPRITES_PER_ROW / font->h_sprites)) * font->h_sprites;
		int y_offset = (char_index / (SPRITES_PER_ROW / font->h_sprites)) * font->v_sprites;
		int sprite_index = font->sprite_index + x_offset + (y_offset * SPRITES_PER_ROW);

		rect_t rect = sprite_index_to_spritesheet_rect(computer->ram, sprite_index, font->h_sprites, font->v_sprites);

		for (int i = 0; i < rect.h; i++) {
			for (int j = 0; j < rect.w; j++) {
				uint8_t font_color = computer->ram->spritesheet.data[(rect.y + i) * SPRITESHEET_WIDTH + (rect.x + j)];
				if (font_color == 15) {
					set_pixel(computer, new_x + j, new_y + i, color);
				}
			}
		}

		if (font->monospace) {
			new_x += font->width + font->horizontal_space;
		} else {
			new_x += font->widths[text[i] - VISIBLE_CHARACTERS_START] + font->horizontal_space;
		}
	}
}

// Midpoint circle algorithm
void api_circ(computer_t *computer, int x, int y, int radius, uint8_t color) {
	// Initial 4 points
	set_pixel(computer, x + radius, y, color);
	set_pixel(computer, x - radius, y, color);
	set_pixel(computer, x, y + radius, color);
	set_pixel(computer, x, y - radius, color);

	int x_offset = radius;
	int y_offset = 0;

	int d = 1 - radius;

	while (x_offset > y_offset) {
		y_offset++;
		
		if (d <= 0) {
			d = d + 2 * y_offset + 1;
		} else {
			x_offset--;
			d = d + 2 * (y_offset - x_offset) + 1;
		}
		
		if (x_offset < y_offset) {
			break;
		}

		set_pixel(computer, x + x_offset, y + y_offset, color);
		set_pixel(computer, x - x_offset, y + y_offset, color);
		set_pixel(computer, x + x_offset, y - y_offset, color);
		set_pixel(computer, x - x_offset, y - y_offset, color);

		if (x_offset != y_offset) {
			set_pixel(computer, x + y_offset, y + x_offset, color);
			set_pixel(computer, x - y_offset, y + x_offset, color);
			set_pixel(computer, x + y_offset, y - x_offset, color);
			set_pixel(computer, x - y_offset, y - x_offset, color);
		}
	}
}

int api_ticks(computer_t *computer) {
	return computer->ticks;
}