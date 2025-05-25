#include "gui.h"

#include <string.h>

#include "input.h"
#include "gfx.h"

// TODO: for each font set a color_key and divider color so you can have funky fonts idk
void gui_draw_string(ram_t *ram, int font_index, string_t string, point_t pos, int color) {
	font_t *font = &ram->fonts[font_index];

	int new_x = pos.x;
	int new_y = pos.y;

	for (size_t i = 0; i < string.len; i++) {
		// Commented this out for now, might add it back later not sure yet
		if (string.data[i] == '\n') {
			new_x = pos.x;
			new_y += font->height + font->vertical_space;
			continue;
		}

		if (string.data[i] == '\t') {
			if (font->monospace) {
				new_x += (font->width + font->horizontal_space) * TAB_SIZE;
			} else {
				new_x += (font->widths[string.data[' '] - ' '] + font->horizontal_space) * TAB_SIZE;
			}

			continue;
		}

		// Inline sprites
		if (string.data[i] == '`') {
			int sprite_index = 0;

			// Read the digits after
			size_t index = i + 1;
			while (string.data[index] >= '0' && string.data[index] <= '9') {
				sprite_index *= 10;
				sprite_index += string.data[index] - '0';

				index++;
			}

			gfx_draw_sprites(ram, sprite_index, POINT(new_x, new_y), font->h_sprites, font->v_sprites);
			new_x += 16 + font->horizontal_space;

			i = index - 1;

			continue;
		}

		// Get the correct sprite index keeping in mind some fonts could have multiple sprites per character (not tested for more than 1 horizontal sprite)
		int char_index = string.data[i] - ' ';
		int x_offset = (char_index % (SPRITES_PER_ROW / font->h_sprites)) * font->h_sprites;
		int y_offset = (char_index / (SPRITES_PER_ROW / font->h_sprites)) * font->v_sprites;
		int sprite_index = font->sprite_index + x_offset + (y_offset * SPRITES_PER_ROW);

		rect_t rect = sprite_index_to_spritesheet_rect(sprite_index, font->h_sprites, font->v_sprites);

		for (int i = 0; i < rect.h; i++) {
			for (int j = 0; j < rect.w; j++) {
				// uint8_t font_color = computer->ram->spritesheet.data[(rect.y + i) * SPRITESHEET_WIDTH + (rect.x + j)];
				color_t font_color = gfx_spritesheet_get_pixel(&ram->spritesheet, (point_t){rect.x + j, rect.y + i});
				if (font_color == 15) {
					gfx_set_pixel(&ram->framebuffer, new_x + j, new_y + i, color);
				}
			}
		}

		if (font->monospace) {
			new_x += font->width + font->horizontal_space;
		} else {
			new_x += font->widths[string.data[i] - ' '] + font->horizontal_space;
		}
	}
}

void gui_draw_text(ram_t *ram, int font_index, const char text[], point_t pos, int color) {
	size_t len = strlen(text);
	gui_draw_string(ram, font_index, (string_t){.data = text, .len = len}, pos, color);
}

// TODO: make versions of these such that the rect is both in and out if that makes sense
// just make it nicer to use bc now it's pretty bad
void gui_outset_frame(ram_t *ram, rect_t rect) {
	int x = rect.x;
	int y = rect.y;
	int w = rect.w;
	int h = rect.h;

	// Because we draw lines it will include x + w or y + h in the pixels drawn
	// Which we don't want so subtract 1
	w--;
	h--;

	surface_t fb_surf = FB_SURF(&ram->framebuffer.data);

	// Top gray line
	gfx_draw_line(fb_surf, POINT(x, y), POINT(x + w - 1, y), ram->gui_colors.frame_edge_neutral);
	// Left gray line
	gfx_draw_line(fb_surf, POINT(x, y), POINT(x, y + h - 1), ram->gui_colors.frame_edge_neutral);

	// Top white line
	gfx_draw_line(fb_surf, POINT(x + 1, y + 1), POINT(x + w - 1, y + 1), ram->gui_colors.frame_edge_light);
	// Left white line
	gfx_draw_line(fb_surf, POINT(x + 1, y + 1), POINT(x + 1, y + h - 1), ram->gui_colors.frame_edge_light);

	// Bottom black line
	gfx_draw_line(fb_surf, POINT(x, y + h), POINT(x + w, y + h), ram->gui_colors.frame_edge_darker);
	// Right black line
	gfx_draw_line(fb_surf, POINT(x + w, y), POINT(x + w, y + h), ram->gui_colors.frame_edge_darker);

	// Bottom gray line
	gfx_draw_line(fb_surf, POINT(x + 1, y + h - 1), POINT(x + w - 1, y + h - 1), ram->gui_colors.frame_edge_dark);
	// Right gray line
	gfx_draw_line(fb_surf, POINT(x + w - 1, y + 1), POINT(x + w - 1, y + h - 1), ram->gui_colors.frame_edge_dark);

	// Background
	gfx_draw_filled_rect(fb_surf, RECT(x + 2, y + 2, w - 3, h - 3), ram->gui_colors.outset_frame_background);
}

void gui_inset_frame(ram_t *ram, rect_t rect) {
	int x = rect.x;
	int y = rect.y;
	int w = rect.w;
	int h = rect.h;

	x -= 2;
	y -= 2;
	w += 4;
	h += 4;

	// Again, because we draw lines it will include x + w or y + h in the pixels drawn
	// Which we don't want so subtract 1
	w--;
	h--;

	surface_t fb_surf = FB_SURF(&ram->framebuffer.data);

	// Top gray line
	gfx_draw_line(fb_surf, POINT(x, y), POINT(x + w - 1, y), ram->gui_colors.frame_edge_dark);
	// Left gray line
	gfx_draw_line(fb_surf, POINT(x, y), POINT(x, y + h - 1), ram->gui_colors.frame_edge_dark);

	// Top black line
	gfx_draw_line(fb_surf, POINT(x + 1, y + 1), POINT(x + w - 1, y + 1), ram->gui_colors.frame_edge_darker);
	// Left black line
	gfx_draw_line(fb_surf, POINT(x + 1, y + 1), POINT(x + 1, y + h - 1), ram->gui_colors.frame_edge_darker);

	// Bottom white line
	gfx_draw_line(fb_surf, POINT(x, y + h), POINT(x + w, y + h), ram->gui_colors.frame_edge_light);
	// Right white line
	gfx_draw_line(fb_surf, POINT(x + w, y), POINT(x + w, y + h), ram->gui_colors.frame_edge_light);

	// Bottom gray line
	gfx_draw_line(fb_surf, POINT(x + 1, y + h - 1), POINT(x + w - 1, y + h - 1), ram->gui_colors.frame_edge_neutral);
	// Right gray line
	gfx_draw_line(fb_surf, POINT(x + w - 1, y + 1), POINT(x + w - 1, y + h - 1), ram->gui_colors.frame_edge_neutral);

	// Background
	gfx_draw_filled_rect(fb_surf, RECT(x + 2, y + 2, w - 3, h - 3), ram->gui_colors.inset_frame_background);
}

bool gui_button_ex(ram_t *ram, char text[], rect_t rect, bool already_pressed) {
	point_t mouse_pos = input_get_mouse_pos();

	bool return_value = false;
	
	if (point_in_rect(mouse_pos, rect)) {
		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			already_pressed = true;
			return_value = true;
		}
	}
	
	if (already_pressed) {
		rect_t new_rect = {
			.x = rect.x + 2,
			.y = rect.y + 2,
			.w = rect.w - 4,
			.h = rect.h - 4,
		};
		
		gui_inset_frame(ram, new_rect);
		gui_draw_text(ram, 0, text, POINT(rect.x + 3, rect.y + 3), ram->gui_colors.text);

	} else {
		gui_outset_frame(ram, rect);
		gui_draw_text(ram, 0, text, POINT(rect.x + 3, rect.y + 3), ram->gui_colors.text);
	}
	
	return return_value;
}

bool gui_button(ram_t *ram, char text[], rect_t rect) {
	return gui_button_ex(ram, text, rect, false);
}

// TODO: investigate why this function exists because apparantly I forgot
// aha it only returns true once when clicked, instead of as long as the mouse button is held
// I should probably refactor this a little bit
bool gui_press_button(ram_t *ram, char text[], rect_t rect) {
	point_t mouse_pos = input_get_mouse_pos();

	bool pressed = false;
	bool held = false;
	
	if (point_in_rect(mouse_pos, rect)) {
		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			held = true;
		}

		if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
			pressed = true;
		}
	}
	
	if (held) {
		rect_t new_rect = {
			.x = rect.x + 2,
			.y = rect.y + 2,
			.w = rect.w - 4,
			.h = rect.h - 4,
		};
		
		gui_inset_frame(ram, new_rect);
		gui_draw_text(ram, 0, text, POINT(rect.x + 3, rect.y + 3), ram->gui_colors.text);
	} else {
		gui_outset_frame(ram, rect);
		gui_draw_text(ram, 0, text, POINT(rect.x + 3, rect.y + 3), ram->gui_colors.text);
	}
	
	return pressed;
}

bool gui_toggle_button(ram_t *ram, char text[], rect_t rect, bool set) {
	point_t mouse_pos = input_get_mouse_pos();
	
	if (point_in_rect(mouse_pos, rect)) {
		if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
			set = !set;
		}
	}
	
	if (set) {
		rect_t new_rect = {
			.x = rect.x + 2,
			.y = rect.y + 2,
			.w = rect.w - 4,
			.h = rect.h - 4,
		};
		
		gui_inset_frame(ram, new_rect);
		gui_draw_text(ram, 2, text, POINT(rect.x + 2, rect.y + 2), ram->gui_colors.toggle_button_set_text);
	} else {
		gui_outset_frame(ram, rect);
		gui_draw_text(ram, 2, text, POINT(rect.x + 2, rect.y + 2), ram->gui_colors.toggle_button_unset_text);
	}
	
	return set;
}

int gui_get_string_width(font_t *font, string_t string, int max_offset) {
	int len = 0;

	for (int i = 0; i < (int)string.len && i < max_offset; i++) {
		if (string.data[i] == '\t') {
			if (font->monospace) {
				len += (font->width + font->horizontal_space) * TAB_SIZE;
			} else {
				len += (font->widths[string.data[' '] - ' '] + font->horizontal_space) * TAB_SIZE;
			}

			continue;
		}

		if (font->monospace) {
			len += font->width + font->horizontal_space;
		} else {
			len += font->widths[string.data[i] - ' '] + font->horizontal_space;
		}
	}

	return len;
}

int gui_get_text_width(font_t *font, char text[], int max_offset) {
	size_t len = strlen(text);
	return gui_get_string_width(font, (string_t){.data = text, .len = len}, max_offset);
}

// int gui_x_to_text_index(font_t *font, char text[], int x) {
// 	int index = x / (font->width + font->horizontal_space);
// 	int len = strlen(text);

// 	int real_index = index;

// 	for (int i = 0; i < index && i < len; i++) {
// 		if (text[i] == '\t') {
// 			real_index -= TAB_SIZE - 1;
// 		}

// 		if (real_index < 0) {
// 			real_index = 0;
// 			break;
// 		}
// 	}
	
	
// 	if (real_index >= len) {
// 		real_index = len;
// 	}

// 	return real_index;
// }
int gui_x_to_string_index(font_t *font, string_t string, int x) {
	int index = x / (font->width + font->horizontal_space);

	int real_index = index;

	for (size_t i = 0; i < index && i < string.len; i++) {
		if (string.data[i] == '\t') {
			real_index -= TAB_SIZE - 1;
		}

		if (real_index < 0) {
			real_index = 0;
			break;
		}
	}
	
	
	if (real_index >= string.len) {
		real_index = string.len;
	}

	return real_index;
}

// TODO: finish and use this function
void gui_init_font(ram_t *ram, int index, int start_sprite_index, int horizontal_space, int height, int h_sprites, int v_sprites) {
	ram->fonts[index] = (font_t){
		.sprite_index = start_sprite_index,
		.horizontal_space = horizontal_space,
		.height = height,
		.h_sprites = h_sprites,
		.v_sprites = v_sprites,
	};

	// Fill in widths based on drawn lines in spritesheet

}

rect_t gui_rect_to_outset_frame_rect(rect_t rect) {
	rect.x -= GUI_BORDER_WIDTH;
	rect.y -= GUI_BORDER_WIDTH;
	rect.w += GUI_BORDER_WIDTH * 2;
	rect.h += GUI_BORDER_WIDTH * 2;

	return rect;
}