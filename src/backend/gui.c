#include "gui.h"

#include <string.h>

#include "input.h"
#include "gfx.h"

// TODO: for each font set a color_key and divider color so you can have funky fonts idk
void gui_draw_text(ram_t *ram, int font_index, char text[], vec2i_t pos, int color) {
	font_t *font = &ram->fonts[font_index];

	int new_x = pos.x;
	int new_y = pos.y;

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
				new_x += (font->widths[text[' '] - ' '] + font->horizontal_space) * TAB_SIZE;
			}

			continue;
		}

		// Inline sprites
		if (text[i] == '`') {
			int sprite_index = 0;

			// Read the digits after
			size_t index = i + 1;
			while (text[index] >= '0' && text[index] <= '9') {
				sprite_index *= 10;
				sprite_index += text[index] - '0';

				index++;
			}

			gfx_draw_sprites(ram, sprite_index, VEC2I(new_x, new_y), font->h_sprites, font->v_sprites);
			new_x += 16 + font->horizontal_space;

			i = index - 1;

			continue;
		}

		// Get the correct sprite index keeping in mind some fonts could have multiple sprites per character (not tested for more than 1 horizontal sprite)
		int char_index = text[i] - ' ';
		int x_offset = (char_index % (SPRITES_PER_ROW / font->h_sprites)) * font->h_sprites;
		int y_offset = (char_index / (SPRITES_PER_ROW / font->h_sprites)) * font->v_sprites;
		int sprite_index = font->sprite_index + x_offset + (y_offset * SPRITES_PER_ROW);

		recti_t rect = sprite_index_to_spritesheet_rect(sprite_index, font->h_sprites, font->v_sprites);

		for (int i = 0; i < rect.h; i++) {
			for (int j = 0; j < rect.w; j++) {
				// uint8_t font_color = computer->ram->spritesheet.data[(rect.y + i) * SPRITESHEET_WIDTH + (rect.x + j)];
				color_t font_color = gfx_spritesheet_get_pixel(&ram->spritesheet, (vec2i_t){rect.x + j, rect.y + i});
				if (font_color == 15) {
					gfx_set_pixel(&ram->framebuffer, new_x + j, new_y + i, color);
				}
			}
		}

		if (font->monospace) {
			new_x += font->width + font->horizontal_space;
		} else {
			new_x += font->widths[text[i] - ' '] + font->horizontal_space;
		}
	}
}

// TODO: make versions of these such that the rect is both in and out if that makes sense
// just make it nicer to use bc now it's pretty bad
void gui_outset_frame(ram_t *ram, recti_t rect) {
	int x = rect.x;
	int y = rect.y;
	int w = rect.w;
	int h = rect.h;

	// Because we draw lines it will include x + w or y + h in the pixels drawn
	// Which we don't want so subtract 1
	w--;
	h--;

	// Top gray line
	gfx_draw_line(&ram->framebuffer, VEC2I(x, y), VEC2I(x + w - 1, y), ram->gui_colors.frame_edge_neutral);
	// Left gray line
	gfx_draw_line(&ram->framebuffer, VEC2I(x, y), VEC2I(x, y + h - 1), ram->gui_colors.frame_edge_neutral);

	// Top white line
	gfx_draw_line(&ram->framebuffer, VEC2I(x + 1, y + 1), VEC2I(x + w - 1, y + 1), ram->gui_colors.frame_edge_light);
	// Left white line
	gfx_draw_line(&ram->framebuffer, VEC2I(x + 1, y + 1), VEC2I(x + 1, y + h - 1), ram->gui_colors.frame_edge_light);

	// Bottom black line
	gfx_draw_line(&ram->framebuffer, VEC2I(x, y + h), VEC2I(x + w, y + h), ram->gui_colors.frame_edge_darker);
	// Right black line
	gfx_draw_line(&ram->framebuffer, VEC2I(x + w, y), VEC2I(x + w, y + h), ram->gui_colors.frame_edge_darker);

	// Bottom gray line
	gfx_draw_line(&ram->framebuffer, VEC2I(x + 1, y + h - 1), VEC2I(x + w - 1, y + h - 1), ram->gui_colors.frame_edge_dark);
	// Right gray line
	gfx_draw_line(&ram->framebuffer, VEC2I(x + w - 1, y + 1), VEC2I(x + w - 1, y + h - 1), ram->gui_colors.frame_edge_dark);

	// Background
	gfx_draw_filled_rect(&ram->framebuffer, RECTI(x + 2, y + 2, w - 3, h - 3), ram->gui_colors.outset_frame_background);
}

void gui_inset_frame(ram_t *ram, recti_t rect) {
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

	// Top gray line
	gfx_draw_line(&ram->framebuffer, VEC2I(x, y), VEC2I(x + w - 1, y), ram->gui_colors.frame_edge_dark);
	// Left gray line
	gfx_draw_line(&ram->framebuffer, VEC2I(x, y), VEC2I(x, y + h - 1), ram->gui_colors.frame_edge_dark);

	// Top black line
	gfx_draw_line(&ram->framebuffer, VEC2I(x + 1, y + 1), VEC2I(x + w - 1, y + 1), ram->gui_colors.frame_edge_darker);
	// Left black line
	gfx_draw_line(&ram->framebuffer, VEC2I(x + 1, y + 1), VEC2I(x + 1, y + h - 1), ram->gui_colors.frame_edge_darker);

	// Bottom white line
	gfx_draw_line(&ram->framebuffer, VEC2I(x, y + h), VEC2I(x + w, y + h), ram->gui_colors.frame_edge_light);
	// Right white line
	gfx_draw_line(&ram->framebuffer, VEC2I(x + w, y), VEC2I(x + w, y + h), ram->gui_colors.frame_edge_light);

	// Bottom gray line
	gfx_draw_line(&ram->framebuffer, VEC2I(x + 1, y + h - 1), VEC2I(x + w - 1, y + h - 1), ram->gui_colors.frame_edge_neutral);
	// Right gray line
	gfx_draw_line(&ram->framebuffer, VEC2I(x + w - 1, y + 1), VEC2I(x + w - 1, y + h - 1), ram->gui_colors.frame_edge_neutral);

	// Background
	gfx_draw_filled_rect(&ram->framebuffer, RECTI(x + 2, y + 2, w - 3, h - 3), ram->gui_colors.inset_frame_background);
}

bool gui_button_ex(ram_t *ram, char text[], recti_t rect, bool already_pressed) {
	vec2i_t mouse_pos = input_get_mouse_pos();

	bool return_value = false;
	
	if (point_in_recti(mouse_pos, rect)) {
		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			already_pressed = true;
			return_value = true;
		}
	}
	
	if (already_pressed) {
		recti_t new_rect = {
			.x = rect.x + 2,
			.y = rect.y + 2,
			.w = rect.w - 4,
			.h = rect.h - 4,
		};
		
		gui_inset_frame(ram, new_rect);
		gui_draw_text(ram, 0, text, VEC2I(rect.x + 3, rect.y + 3), ram->gui_colors.text);

	} else {
		gui_outset_frame(ram, rect);
		gui_draw_text(ram, 0, text, VEC2I(rect.x + 3, rect.y + 3), ram->gui_colors.text);
	}
	
	return return_value;
}

bool gui_button(ram_t *ram, char text[], recti_t rect) {
	return gui_button_ex(ram, text, rect, false);
}

// TODO: investigate why this function exists because apparantly I forgot
// aha it only returns true once when clicked, instead of as long as the mouse button is held
// I should probably refactor this a little bit
bool gui_press_button(ram_t *ram, char text[], recti_t rect) {
	vec2i_t mouse_pos = input_get_mouse_pos();

	bool pressed = false;
	bool held = false;
	
	if (point_in_recti(mouse_pos, rect)) {
		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			held = true;
		}

		if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
			pressed = true;
		}
	}
	
	if (held) {
		recti_t new_rect = {
			.x = rect.x + 2,
			.y = rect.y + 2,
			.w = rect.w - 4,
			.h = rect.h - 4,
		};
		
		gui_inset_frame(ram, new_rect);
		gui_draw_text(ram, 0, text, VEC2I(rect.x + 3, rect.y + 3), ram->gui_colors.text);
	} else {
		gui_outset_frame(ram, rect);
		gui_draw_text(ram, 0, text, VEC2I(rect.x + 3, rect.y + 3), ram->gui_colors.text);
	}
	
	return pressed;
}

bool gui_toggle_button(ram_t *ram, char text[], recti_t rect, bool set) {
	vec2i_t mouse_pos = input_get_mouse_pos();
	
	if (point_in_recti(mouse_pos, rect)) {
		if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
			set = !set;
		}
	}
	
	if (set) {
		recti_t new_rect = {
			.x = rect.x + 2,
			.y = rect.y + 2,
			.w = rect.w - 4,
			.h = rect.h - 4,
		};
		
		gui_inset_frame(ram, new_rect);
		gui_draw_text(ram, 2, text, VEC2I(rect.x + 2, rect.y + 2), ram->gui_colors.toggle_button_set_text);
	} else {
		gui_outset_frame(ram, rect);
		gui_draw_text(ram, 2, text, VEC2I(rect.x + 2, rect.y + 2), ram->gui_colors.toggle_button_unset_text);
	}
	
	return set;
}

int gui_get_text_width(font_t *font, char text[], int max_offset) {
	int len = 0;

	for (int i = 0; i < max_offset; i++) {
		if (text[i] == '\t') {
			if (font->monospace) {
				len += (font->width + font->horizontal_space) * TAB_SIZE;
			} else {
				len += (font->widths[text[' '] - ' '] + font->horizontal_space) * TAB_SIZE;
			}

			continue;
		}

		if (font->monospace) {
			len += font->width + font->horizontal_space;
		} else {
			len += font->widths[text[i] - ' '] + font->horizontal_space;
		}
	}

	return len;
}

int gui_x_to_text_index(font_t *font, char text[], int x) {
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