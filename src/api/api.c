#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../backend/input.h"
#include "../backend/gfx.h"
#include "../backend/gui.h"

// TODO: remove api calls anywhere else in the code
// idk I want it to exist in a bubble I guess

void api_cls(computer_t *computer, int color) {
	// for (int y = 0; y < SCREEN_HEIGHT; y++) {
	// 	for (int x = 0; x < SCREEN_WIDTH; x++) {
	// 		computer->ram->framebuffer.data[y * SCREEN_WIDTH + x] = color;
	// 	}
	// }
	gfx_clear(&computer->ram->framebuffer, color);
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

// void api_spr(computer_t *computer, int sprite_index, int x, int y, int width, int height, int scale) {
void api_spr(computer_t *computer, int sprite_index, int x, int y, int width, int height) {
	// x -= computer->ram->draw_state.cam_pos_x;
	// y -= computer->ram->draw_state.cam_pos_y;

	// recti_t rect = sprite_index_to_spritesheet_rect(computer->ram, sprite_index, width, height);
	
	// if (scale == 1) {
	// 	draw_sprite_sheet_rect(computer, x, y, rect, computer->ram->sprites[sprite_index].color_key);
	// } else {
	// 	draw_sprite_sheet_rect_scaled(computer, x, y, rect, computer->ram->sprites[sprite_index].color_key, scale);
	// }

	gfx_draw_sprites(computer->ram, sprite_index, (vec2i_t){x, y}, width, height);
}

// void api_sspr(computer_t *computer, int x, int y, int rx, int ry, int rw, int rh, int color_key, int scale) {
// 	x -= computer->ram->draw_state.cam_pos_x;
// 	y -= computer->ram->draw_state.cam_pos_y;

// 	recti_t rect = {rx, ry, rw, rh};

// 	if (scale == 1) {
// 		draw_sprite_sheet_rect(computer, x, y, rect, color_key);
// 	} else {
// 		draw_sprite_sheet_rect_scaled(computer, x, y, rect, color_key, scale);
// 	}
// }

// Using Bresemham's line algorithm
void api_line(computer_t *computer, int x1, int y1, int x2, int y2, int color) {
	// x1 -= computer->ram->draw_state.cam_pos_x;
	// y1 -= computer->ram->draw_state.cam_pos_y;
	// x2 -= computer->ram->draw_state.cam_pos_x;
	// y2 -= computer->ram->draw_state.cam_pos_y;

	// int dx = abs(x2 - x1);
	// int dy = abs(y2 - y1);
	// int step_x = (x1 < x2) ? 1 : -1;
	// int step_y = (y1 < y2) ? 1 : -1;
	// int error = dx - dy;

	// while (true) {
	// 	gfx_set_pixel(computer, x1, y1, color);
		
	// 	if (x1 == x2 && y1 == y2) break;

	// 	int e2 = 2 * error;
	// 	if (e2 > -dy) {
	// 		error -= dy;
	// 		x1 += step_x;
	// 	}
	// 	if (e2 < dx) {
	// 		error += dx;
	// 		y1 += step_y;
	// 	}
	// }

	gfx_draw_line(&computer->ram->framebuffer, (vec2i_t){x1, y1}, (vec2i_t){x2, y2}, color);
}

void api_rect(computer_t *computer, int x, int y, int w, int h, int color) {
	// x -= computer->ram->draw_state.cam_pos_x;
	// y -= computer->ram->draw_state.cam_pos_y;

	// for (int j = x; j < x+w; j++) {
	// 	gfx_set_pixel(computer, j, y, color);
	// }

	// for (int j = x; j < x+w; j++) {
	// 	gfx_set_pixel(computer, j, y+h-1, color);
	// }

	// for (int i = y; i < y+h; i++) {
	// 	gfx_set_pixel(computer, x, i, color);
	// }

	// for (int i = y; i < y+h; i++) {
	// 	gfx_set_pixel(computer, x+w - 1, i, color);
	// }

	gfx_draw_rect(&computer->ram->framebuffer, (recti_t){x, y, w, h}, color);
}

void api_rectf(computer_t *computer, int x, int y, int w, int h, int color) {
	// x -= computer->ram->draw_state.cam_pos_x;
	// y -= computer->ram->draw_state.cam_pos_y;

	gfx_draw_filled_rect(&computer->ram->framebuffer, (recti_t){x, y, w, h}, color);
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

bool api_mouse_btn(computer_t *computer, int gui_button) {
	return input_mouse_button_held(gui_button);
}

bool api_mouse_btnp(computer_t *computer, int gui_button) {
	return input_mouse_button_pressed(gui_button);
}

bool api_mouse_btnr(computer_t *computer, int gui_button) {
	return input_mouse_button_released(gui_button);
}

bool api_mouse_scrolled(computer_t *computer, int direction) {
	return input_mouse_scrolled(direction);
}

void api_text(computer_t *computer, int font_index, char text[], int x, int y, int color) {
	gui_draw_text(computer->ram, font_index, text, (vec2i_t){x, y}, color);

	// // x -= computer->ram->draw_state.cam_pos_x;
	// // y -= computer->ram->draw_state.cam_pos_y;

	// font_t *font = &computer->ram->fonts[font_index];

	// int new_x = x;
	// int new_y = y;

	// for (size_t i = 0; i < strlen(text); i++) {
	// 	// Commented this out for now, might add it back later not sure yet
	// 	if (text[i] == '\n') {
	// 		new_x = 0;
	// 		new_y += font->height + font->vertical_space;
	// 		continue;
	// 	}

	// 	if (text[i] == '\t') {
	// 		if (font->monospace) {
	// 			new_x += (font->width + font->horizontal_space) * TAB_SIZE;
	// 		} else {
	// 			new_x += (font->widths[text[' '] - VISIBLE_CHARACTERS_START] + font->horizontal_space) * TAB_SIZE;
	// 		}

	// 		continue;
	// 	}

	// 	// Inline sprites
	// 	if (text[i] == '~') {
	// 		int sprite_index = 0;

	// 		// Read the digits after
	// 		size_t index = i + 1;
	// 		while (text[index] >= '0' && text[index] <= '9') {
	// 			sprite_index *= 10;
	// 			sprite_index += text[index] - '0';

	// 			index++;
	// 		}

	// 		// Only sprite sheet 0 now
	// 		// api_spr(computer, sprite_index, new_x, new_y, 1, 1, 1);
	// 		api_spr(computer, sprite_index, new_x, new_y, 1, 1);
	// 		new_x += 16 + font->horizontal_space;

	// 		i = index - 1;

	// 		continue;
	// 	}

	// 	// Get the correct sprite index keeping in mind some fonts could have multiple sprites per character (not tested for more than 1 horizontal sprite)
	// 	int char_index = text[i] - VISIBLE_CHARACTERS_START;
	// 	int x_offset = (char_index % (SPRITES_PER_ROW / font->h_sprites)) * font->h_sprites;
	// 	int y_offset = (char_index / (SPRITES_PER_ROW / font->h_sprites)) * font->v_sprites;
	// 	int sprite_index = font->sprite_index + x_offset + (y_offset * SPRITES_PER_ROW);

	// 	recti_t rect = sprite_index_to_spritesheet_rect(sprite_index, font->h_sprites, font->v_sprites);

	// 	for (int i = 0; i < rect.h; i++) {
	// 		for (int j = 0; j < rect.w; j++) {
	// 			// uint8_t font_color = computer->ram->spritesheet.data[(rect.y + i) * SPRITESHEET_WIDTH + (rect.x + j)];
	// 			color_t font_color = gfx_spritesheet_get_pixel(&computer->ram->spritesheet, (vec2i_t){rect.x + j, rect.y + i});
	// 			if (font_color == 15) {
	// 				gfx_set_pixel(&computer->ram->framebuffer, new_x + j, new_y + i, color);
	// 			}
	// 		}
	// 	}

	// 	if (font->monospace) {
	// 		new_x += font->width + font->horizontal_space;
	// 	} else {
	// 		new_x += font->widths[text[i] - VISIBLE_CHARACTERS_START] + font->horizontal_space;
	// 	}
	// }
}

// Midpoint circle algorithm
// TODO: adopt for ellipses
void api_circ(computer_t *computer, int x, int y, int radius, uint8_t color) {
	// x -= computer->ram->draw_state.cam_pos_x;
	// y -= computer->ram->draw_state.cam_pos_y;

	// // Initial 4 points
	// gfx_set_pixel(computer, x + radius, y, color);
	// gfx_set_pixel(computer, x - radius, y, color);
	// gfx_set_pixel(computer, x, y + radius, color);
	// gfx_set_pixel(computer, x, y - radius, color);

	// int x_offset = radius;
	// int y_offset = 0;

	// int d = 1 - radius;

	// while (x_offset > y_offset) {
	// 	y_offset++;
		
	// 	if (d <= 0) {
	// 		d = d + 2 * y_offset + 1;
	// 	} else {
	// 		x_offset--;
	// 		d = d + 2 * (y_offset - x_offset) + 1;
	// 	}
		
	// 	if (x_offset < y_offset) {
	// 		break;
	// 	}

	// 	gfx_set_pixel(computer, x + x_offset, y + y_offset, color);
	// 	gfx_set_pixel(computer, x - x_offset, y + y_offset, color);
	// 	gfx_set_pixel(computer, x + x_offset, y - y_offset, color);
	// 	gfx_set_pixel(computer, x - x_offset, y - y_offset, color);

	// 	if (x_offset != y_offset) {
	// 		gfx_set_pixel(computer, x + y_offset, y + x_offset, color);
	// 		gfx_set_pixel(computer, x - y_offset, y + x_offset, color);
	// 		gfx_set_pixel(computer, x + y_offset, y - x_offset, color);
	// 		gfx_set_pixel(computer, x - y_offset, y - x_offset, color);
	// 	}
	// }

	gfx_draw_circle(&computer->ram->framebuffer, (vec2i_t){x, y}, radius, color);
}

void api_draw_map_layer(computer_t *computer, int layer, int x, int y, int cell_x, int cell_y, int cell_w, int cell_h) {
	// x -= computer->ram->draw_state.cam_pos_x;
	// y -= computer->ram->draw_state.cam_pos_y;

	if (cell_x < 0) {
		cell_x = 0;
	}
	if (cell_y < 0) {
		cell_y = 0;
	}

	for (int i = cell_y; i < cell_y + cell_h; i++) {
		for (int j = cell_x; j < cell_x + cell_w; j++) {
			int sprite_index = computer->ram->map.layers[layer].data[i * MAP_WIDTH + j];
			// api_spr(computer, sprite_index, x + j * SPRITE_WIDTH, y + i * SPRITE_HEIGHT, 1, 1, 1);
			api_spr(computer, sprite_index, x + j * SPRITE_WIDTH, y + i * SPRITE_HEIGHT, 1, 1);
		}
	}
}

int api_ticks(computer_t *computer) {
	return computer->ticks;
}

// void api_camera(computer_t *computer, int x, int y) {
// 	computer->ram->draw_state.cam_pos_x = x;
// 	computer->ram->draw_state.cam_pos_y = y;
// }

// void api_reset_camera(computer_t *computer) {
// 	computer->ram->draw_state.cam_pos_x = 0;
// 	computer->ram->draw_state.cam_pos_y = 0;
// }

// void api_screen_to_world(computer_t *computer, int screen_x, int screen_y, int *world_x, int *world_y) {
// 	*world_x = screen_x + computer->ram->draw_state.cam_pos_x;
// 	*world_y = screen_y + computer->ram->draw_state.cam_pos_y;
// }

// void api_world_to_screen(computer_t *computer, int world_x, int world_y, int *screen_x, int *screen_y) {
// 	*screen_x = world_x - computer->ram->draw_state.cam_pos_x;
// 	*screen_y = world_y - computer->ram->draw_state.cam_pos_y;
// }

// void api_world_to_grid(computer_t *computer, int world_x, int world_y, int *grid_x, int *grid_y) {
// 	*grid_x = world_x / SPRITE_WIDTH;
// 	*grid_y = world_y / SPRITE_HEIGHT;
// }

// void api_grid_to_world(computer_t *computer, int grid_x, int grid_y, int *world_x, int *world_y) {
// 	*world_x = grid_x * SPRITE_WIDTH;
// 	*world_y = grid_y * SPRITE_HEIGHT;
// }