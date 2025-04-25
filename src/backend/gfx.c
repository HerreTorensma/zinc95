#include "gfx.h"

/*
void draw_sprite_sheet_rect(computer_t *computer, int x, int y, rect_t rect, uint8_t color_key) {
	for (int i = 0; i < rect.h; i++) {
		for (int j = 0; j < rect.w; j++) {
			// uint8_t color = computer->ram->spritesheet.data[(rect.y + i) * SPRITESHEET_WIDTH + (rect.x + j)];
			uint8_t color = spritesheet_get_pixel(computer, rect.x + j, rect.y + i);
			if (color != color_key) {
				gfx_set_pixel(computer, x + j, y + i, color);
			}
		}
	}
}

void draw_sprite_sheet_rect_scaled(computer_t *computer, int x, int y, rect_t rect, uint8_t color_key, int scale) {
	for (int i = 0; i < rect.h; i++) {
		for (int j = 0; j < rect.w; j++) {
			// uint8_t color = computer->ram->spritesheet.data[(rect.y + i) * SPRITESHEET_WIDTH + (rect.x + j)];
			uint8_t color = spritesheet_get_pixel(computer, rect.x + j, rect.y + i);
			// api_rectf(computer, x + (j * scale), y + (i * scale), scale, scale, color);
			rect_t new_rect = {x + (j * scale), y + (i * scale), scale, scale};
			draw_filled_rectangle(computer, new_rect, color);
		}
	}
}

// TODO: use this functions instead of the one above
void draw_sprite_sheet_rect_scaled_float(computer_t *computer, rect_t dest_rect, rect_t source_rect, uint8_t color_key) {
	for (int y = 0; y < dest_rect.h; y++) {
		for (int x = 0; x < dest_rect.w; x++) {
			// Calculate normalized coords
			float u = (float)x / (float)dest_rect.w;
			float v = (float)y / (float)dest_rect.h;

			// Then convert to source coords
			int source_x = source_rect.x + u * source_rect.w;
			int source_y = source_rect.y + v * source_rect.h;

			uint8_t color = spritesheet_get_pixel(computer, source_x, source_y);

			gfx_set_pixel(computer, dest_rect.x + x, dest_rect.y + y, color);
		}
	}
}

void draw_filled_rectangle(computer_t *computer, rect_t rect, uint8_t color) {
	// Using i and j to avoid conflict with the x and y parameters
	for (int i = rect.y; i < rect.y+rect.h; i++) {
		for (int j = rect.x; j < rect.x+rect.w; j++) {
			gfx_set_pixel(computer, j, i, color);
		}
	}
}
*/

#include <stdlib.h>

static bool point_in_screen(int x, int y) {
	if (x < 0) return false;
	if (x >= SCREEN_WIDTH) return false;
	if (y < 0) return false;
	if (y >= SCREEN_HEIGHT) return false;

	return true;
}

void gfx_generate_rgb_framebuffer(computer_t *computer) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			uint8_t pixel = computer->ram->framebuffer.data[y * SCREEN_WIDTH + x];
			computer->rgb_framebuffer[y * SCREEN_WIDTH + x] = computer->ram->palette.colors[pixel];
		}
	}
}


void gfx_set_pixel(framebuffer_t *fb, int x, int y, int color) {
	if (point_in_screen(x, y)) {
		fb->data[y * SCREEN_WIDTH + x] = color;
	}
}

color_t gfx_get_pixel(framebuffer_t *fb, int x, int y) {
	if (point_in_screen(x, y)) {
		return fb->data[y * SCREEN_WIDTH + x];
	}
	return 0;
}

void gfx_clear(framebuffer_t *fb, color_t color) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			gfx_set_pixel(fb, x, y, color);
		}
	}
}

// TODO: make these geometry functions take any uint8_t array and a vec2i of size
// so they can be used both for the framebuffer and the spritesheet
void gfx_draw_rect(framebuffer_t *fb, recti_t rect, color_t color) {
	for (int j = rect.x; j < rect.x+rect.w; j++) {
		gfx_set_pixel(fb, j, rect.y, color);
	}

	for (int j = rect.x; j < rect.x+rect.w; j++) {
		gfx_set_pixel(fb, j, rect.y+rect.h-1, color);
	}

	for (int i = rect.y; i < rect.y+rect.h; i++) {
		gfx_set_pixel(fb, rect.x, i, color);
	}

	for (int i = rect.y; i < rect.y+rect.h; i++) {
		gfx_set_pixel(fb, rect.x+rect.w - 1, i, color);
	}
}

void gfx_draw_filled_rect(framebuffer_t *fb, recti_t rect, color_t color) {
	for (int i = rect.y; i < rect.y+rect.h; i++) {
		for (int j = rect.x; j < rect.x+rect.w; j++) {
			gfx_set_pixel(fb, j, i, color);
		}
	}
}

void gfx_draw_line(framebuffer_t *fb, vec2i_t start, vec2i_t end, color_t color) {
	int x1 = start.x;
	int y1 = start.y;
	int x2 = end.x;
	int y2 = end.y;
	
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int step_x = (x1 < x2) ? 1 : -1;
	int step_y = (y1 < y2) ? 1 : -1;
	int error = dx - dy;
	
	while (true) {
		gfx_set_pixel(fb, x1, y1, color);
		
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

void gfx_draw_circle(framebuffer_t *fb, vec2i_t pos, int radius, color_t color) {
	int x = pos.x;
	int y = pos.y;
	
	// Initial 4 points
	gfx_set_pixel(fb, x + radius, y, color);
	gfx_set_pixel(fb, x - radius, y, color);
	gfx_set_pixel(fb, x, y + radius, color);
	gfx_set_pixel(fb, x, y - radius, color);

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

		gfx_set_pixel(fb, x + x_offset, y + y_offset, color);
		gfx_set_pixel(fb, x - x_offset, y + y_offset, color);
		gfx_set_pixel(fb, x + x_offset, y - y_offset, color);
		gfx_set_pixel(fb, x - x_offset, y - y_offset, color);

		if (x_offset != y_offset) {
			gfx_set_pixel(fb, x + y_offset, y + x_offset, color);
			gfx_set_pixel(fb, x - y_offset, y + x_offset, color);
			gfx_set_pixel(fb, x + y_offset, y - x_offset, color);
			gfx_set_pixel(fb, x - y_offset, y - x_offset, color);
		}
	}
}

color_t gfx_spritesheet_get_pixel(spritesheet_t *spritesheet, vec2i_t point) {
	if (point_in_recti(point, (recti_t){0, 0, SPRITESHEET_WIDTH, SPRITESHEET_HEIGHT})) {
		return spritesheet->data[point.y * SPRITESHEET_WIDTH + point.x];
	}
	return 0;
}

void gfx_spritesheet_set_pixel(spritesheet_t *spritesheet, vec2i_t point, color_t color) {
	if (point_in_recti(point, (recti_t){0, 0, SPRITESHEET_WIDTH, SPRITESHEET_HEIGHT})) {
		spritesheet->data[point.y * SPRITESHEET_WIDTH + point.x] = color;
	}
}

recti_t sprite_index_to_spritesheet_rect(int sprite_index, int w, int h) {
	recti_t rect = {
		.x = (sprite_index % SPRITES_PER_ROW) * SPRITE_WIDTH,
		.y = (sprite_index / SPRITES_PER_ROW) * SPRITE_HEIGHT,
		.w = w * SPRITE_WIDTH,
		.h = h * SPRITE_HEIGHT,
	};

	return rect;
}

void gfx_draw_spritesheet_rect(ram_t *ram, vec2i_t pos, recti_t rect, color_t color_key) {
	for (int i = 0; i < rect.h; i++) {
		for (int j = 0; j < rect.w; j++) {
			uint8_t color = gfx_spritesheet_get_pixel(&ram->spritesheet, (vec2i_t){rect.x + j, rect.y + i});
			if (color != color_key) {
				gfx_set_pixel(&ram->framebuffer, pos.x + j, pos.y + i, color);
			}
		}
	}
}

void gfx_draw_spritesheet_pro(ram_t *ram, recti_t source_rect, recti_t dest_rect, color_t color_key) {
	for (int y = 0; y < dest_rect.h; y++) {
		for (int x = 0; x < dest_rect.w; x++) {
			// Calculate normalized coords
			float u = (float)x / (float)dest_rect.w;
			float v = (float)y / (float)dest_rect.h;

			// Then convert to source coords
			int source_x = source_rect.x + u * source_rect.w;
			int source_y = source_rect.y + v * source_rect.h;

			uint8_t color = gfx_spritesheet_get_pixel(&ram->spritesheet, (vec2i_t){source_x, source_y});
			
			if (color != color_key) {
				gfx_set_pixel(&ram->framebuffer, dest_rect.x + x, dest_rect.y + y, color);
			}
		}
	}
}

// TODO: implement flip_x, flip_y
void gfx_draw_sprites(ram_t *ram, int index, vec2i_t pos, int width, int height) {
	recti_t rect = sprite_index_to_spritesheet_rect(index, width, height);
	
	gfx_draw_spritesheet_rect(ram, pos, rect, ram->sprites[index].color_key);
}

void gfx_draw_sprites_page(ram_t *ram, int page_index, int relative_index, vec2i_t pos, int width, int height) {
	int absolute_index = page_index * SPRITES_PER_PAGE + relative_index;

	recti_t rect = sprite_index_to_spritesheet_rect(absolute_index, width, height);

	gfx_draw_spritesheet_rect(ram, pos, rect, ram->sprites[absolute_index].color_key);
}