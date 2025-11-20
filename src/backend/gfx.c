#include "gfx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef BACKEND_SDL2
#include "sdl2.h"
#endif

void gfx_generate_rgb_framebuffer(computer_t *computer) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			color_t pixel = computer->ram->framebuffer.data[y * SCREEN_WIDTH + x];
			computer->rgb_framebuffer[y * SCREEN_WIDTH + x] = computer->ram->palette.colors[pixel];
		}
	}
}

// TODO: don't do linear search
// TODO: get the closes possible color to be able to load any image
color_t gfx_rgb_color_to_color(palette_t *palette, rgb_color_t rgb_color, color_t undefined_color) {
	for (size_t i = 0; i < PALETTE_SIZE; i++) {
		if (palette->colors[i].r == rgb_color.r && palette->colors[i].g == rgb_color.g && palette->colors[i].b == rgb_color.b) {
			return i;
		}
	}

	printf("Color not found\n");
	return undefined_color;
}

void gfx_clear(surface_t surface, color_t color) {
	for (int y = 0; y < surface.height; y++) {
		for (int x = 0; x < surface.width; x++) {
			surf_set_pixel(surface, x, y, color);
		}
	}
}

// TODO: make these geometry functions take any uint8_t array and a vec2i of size
// so they can be used both for the framebuffer and the spritesheet
void gfx_draw_rect(surface_t surf, rect_t rect, color_t color) {
	for (int j = rect.x; j < rect.x+rect.w; j++) {
		surf_set_pixel(surf, j, rect.y, color);
	}

	for (int j = rect.x; j < rect.x+rect.w; j++) {
		surf_set_pixel(surf, j, rect.y+rect.h-1, color);
	}

	for (int i = rect.y; i < rect.y+rect.h; i++) {
		surf_set_pixel(surf, rect.x, i, color);
	}

	for (int i = rect.y; i < rect.y+rect.h; i++) {
		surf_set_pixel(surf, rect.x+rect.w - 1, i, color);
	}
}

void gfx_draw_filled_rect(surface_t surf, rect_t rect, color_t color) {
	for (int i = rect.y; i < rect.y+rect.h; i++) {
		for (int j = rect.x; j < rect.x+rect.w; j++) {
			surf_set_pixel(surf, j, i, color);
		}
	}
}

// Using Bresemham's line algorithm
void gfx_draw_line(surface_t surf, point_t start, point_t end, color_t color) {
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
		surf_set_pixel(surf, x1, y1, color);
		
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

// Midpoint circle algorithm
// TODO: adopt for ellipses
// TODO: filled bool
void gfx_draw_circle(surface_t surf, point_t pos, int radius, color_t color) {
	int x = pos.x;
	int y = pos.y;
	
	// Initial 4 points
	surf_set_pixel(surf, x + radius, y, color);
	surf_set_pixel(surf, x - radius, y, color);
	surf_set_pixel(surf, x, y + radius, color);
	surf_set_pixel(surf, x, y - radius, color);

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

		surf_set_pixel(surf, x + x_offset, y + y_offset, color);
		surf_set_pixel(surf, x - x_offset, y + y_offset, color);
		surf_set_pixel(surf, x + x_offset, y - y_offset, color);
		surf_set_pixel(surf, x - x_offset, y - y_offset, color);

		if (x_offset != y_offset) {
			surf_set_pixel(surf, x + y_offset, y + x_offset, color);
			surf_set_pixel(surf, x - y_offset, y + x_offset, color);
			surf_set_pixel(surf, x + y_offset, y - x_offset, color);
			surf_set_pixel(surf, x - y_offset, y - x_offset, color);
		}
	}
}

// TODO: make this also be able to draw ellipses without a midpoint
void gfx_draw_ellipse(surface_t surf, rect_t bound, color_t color) {
	int rx = bound.w / 2;
	int ry = bound.h / 2;

	int cx = bound.x + bound.w / 2;
	int cy = bound.y + bound.h / 2;
	
	int x = 0;
	int y = ry;

	// Decision parameters
	long rx2 = rx * rx;
	long ry2 = ry * ry;
	long tworx2 = 2 * rx2;
	long twory2 = 2 * ry2;

	long px = 0;
	long py = tworx2 * y;

	// Region 1
	long p = (long)(ry2 - (rx2 * ry) + (0.25 * rx2));
	while (px < py) {
		surf_set_pixel(surf, cx + x, cy + y, color);
		surf_set_pixel(surf, cx - x, cy + y, color);
		surf_set_pixel(surf, cx + x, cy - y, color);
		surf_set_pixel(surf, cx - x, cy - y, color);

		x++;
		px += twory2;
		if (p < 0) {
			p += ry2 + px;
		} else {
			y--;
			py -= tworx2;
			p += ry2 + px - py;
		}
	}

	// Region 2
	p = (long)(ry2 * (x + 0.5) * (x + 0.5) +
			   rx2 * (y - 1) * (y - 1) -
			   rx2 * ry2);
	while (y >= 0) {
		surf_set_pixel(surf, cx + x, cy + y, color);
		surf_set_pixel(surf, cx - x, cy + y, color);
		surf_set_pixel(surf, cx + x, cy - y, color);
		surf_set_pixel(surf, cx - x, cy - y, color);

		y--;
		py -= tworx2;
		if (p > 0) {
			p += rx2 - py;
		} else {
			x++;
			px += twory2;
			p += rx2 - py + px;
		}
	}
}

// TODO: implement
void gfx_flood_fill(surface_t surf, point_t start, color_t color) {

}

color_t gfx_spritesheet_get_pixel(spritesheet_t *spritesheet, point_t point) {
	// TODO: clip the wanted rect instead of this check ???
	if (point_in_rect(point, RECT(0, 0, SPRITESHEET_WIDTH, SPRITESHEET_HEIGHT))) {
		return spritesheet->data[point.y * SPRITESHEET_WIDTH + point.x];
	}
	// TODO: should this be 0 or COLOR_NONE ???
	return COLOR_NONE;
}

void gfx_spritesheet_set_pixel(spritesheet_t *spritesheet, point_t point, color_t color) {
	if (point_in_rect(point, RECT(0, 0, SPRITESHEET_WIDTH, SPRITESHEET_HEIGHT))) {
		spritesheet->data[point.y * SPRITESHEET_WIDTH + point.x] = color;
	}
}

rect_t sprite_index_to_spritesheet_rect(int sprite_index, int w, int h) {
	rect_t rect = {
		.x = (sprite_index % SPRITES_PER_ROW) * SPRITE_WIDTH,
		.y = (sprite_index / SPRITES_PER_ROW) * SPRITE_HEIGHT,
		.w = w * SPRITE_WIDTH,
		.h = h * SPRITE_HEIGHT,
	};

	return rect;
}

// TODO: make everything take a bounds rect, anything outside of it will get clipped
// either that or it's state-based, where by default the clip rect is the screen and you can set it
// but who likes unnecessary state machines
// I can actually just make a macro or wrapper function for clip bounds being the screen

void gfx_copy_surface_rect(surface_t dest, surface_t src, point_t pos, rect_t rect, color_t color_key) {
	for (int i = 0; i < rect.h; i++) {
		for (int j = 0; j < rect.w; j++) {
			color_t color = surf_get_pixel(src, rect.x + j, rect.y + i);
			if (color != color_key) {
				surf_set_pixel(dest, pos.x + j, pos.y + i, color);
			}
		}
	}
}

void gfx_draw_surface_rect(framebuffer_t *fb, surface_t surf, point_t pos, rect_t rect, color_t color_key) {
	for (int i = 0; i < rect.h; i++) {
		for (int j = 0; j < rect.w; j++) {
			color_t color = surf_get_pixel(surf, rect.x + j, rect.y + i);
			if (color != color_key) {
				gfx_set_pixel(fb, pos.x + j, pos.y + i, color);
			}
		}
	}
}

void gfx_draw_spritesheet_rect(ram_t *ram, point_t pos, rect_t rect, color_t color_key) {
	gfx_draw_surface_rect(&ram->framebuffer, SPR_SURF(ram->spritesheet.data), pos, rect, color_key);
}

void gfx_draw_surface_pro(framebuffer_t *fb, surface_t surf, rect_t source_rect, rect_t dest_rect, color_t color_key, rect_t clip_rect) {
	rect_t clipped_dest_rect = rect_clip(clip_rect, dest_rect);

	for (int y = 0; y < clipped_dest_rect.h; y++) {
		for (int x = 0; x < clipped_dest_rect.w; x++) {
			// Calculate normalized coords
			float u = (float)(x + clipped_dest_rect.x - dest_rect.x) / (float)dest_rect.w;
			float v = (float)(y + clipped_dest_rect.y - dest_rect.y) / (float)dest_rect.h;

			// Then convert to source coords
			int source_x = source_rect.x + u * source_rect.w;
			int source_y = source_rect.y + v * source_rect.h;

			// TODO: look at this again later
			// It's the same concept but only integer math so it scales less flexibally but also does not have incorrect pixels
			// and it's faster
			// I should also look into fixed point arithmatic, the 2 lines below are just integer math but that's probably not sufficient
			// int source_x = source_rect.x + (x * source_rect.w) / dest_rect.w;
			// int source_y = source_rect.y + (y * source_rect.h) / dest_rect.h;

			color_t color = surf_get_pixel(surf, source_x, source_y);
			
			if (color != color_key) {
				gfx_set_pixel(fb, clipped_dest_rect.x + x, clipped_dest_rect.y + y, color);
			}
		}
	}
}

void gfx_draw_spritesheet_pro(ram_t *ram, rect_t source_rect, rect_t dest_rect, color_t color_key, rect_t clip_rect) {
	gfx_draw_surface_pro(&ram->framebuffer, SPR_SURF(ram->spritesheet.data), source_rect, dest_rect, color_key, clip_rect);
}

// TODO: implement flip_x, flip_y
void gfx_draw_sprites(ram_t *ram, int index, point_t pos, int width, int height) {
	rect_t rect = sprite_index_to_spritesheet_rect(index, width, height);
	
	gfx_draw_spritesheet_rect(ram, pos, rect, ram->sprites[index].color_key);
}

void gfx_draw_sprites_page(ram_t *ram, int page_index, int relative_index, point_t pos, int width, int height) {
	int absolute_index = page_index * SPRITES_PER_PAGE + relative_index;

	rect_t rect = sprite_index_to_spritesheet_rect(absolute_index, width, height);

	gfx_draw_spritesheet_rect(ram, pos, rect, ram->sprites[absolute_index].color_key);
}

void gfx_draw_map(ram_t *ram, int layer_index, point_t pos, rect_t section, float scale, color_t color_key) {
	section = rect_clip(RECT(0, 0, MAP_WIDTH, MAP_HEIGHT), section);

	for (int i = section.y; i < section.y + section.h; i++) {
		for (int j = section.x; j < section.x + section.w; j++) {

			int sprite_index = ram->map.layers[layer_index].data[i * MAP_WIDTH + j];

			rect_t source_rect = sprite_index_to_spritesheet_rect(sprite_index, 1, 1);

			int scaled_sprite_width = (int)(SPRITE_WIDTH * scale);
			int scaled_sprite_height = (int)(SPRITE_HEIGHT * scale);
			
			rect_t dest_rect = RECT(
				pos.x + j * scaled_sprite_width,
				pos.y + i * scaled_sprite_height,
				scaled_sprite_width,
				scaled_sprite_height
			);

			gfx_draw_spritesheet_pro(ram, source_rect, dest_rect, color_key, RECT(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)); // TODO: pass argument for clip rect
		}
	}
}

void gfx_load_surface(palette_t *palette, surface_t surface, string_t path) {
	#ifdef BACKEND_SDL2
	sdl2_load_bmp_to_surface(palette, surface, path);
	#endif
}
