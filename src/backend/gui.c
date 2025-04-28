#include "gui.h"

#include "input.h"
#include "gfx.h"

// TODO: for each font set a color_key and divider color so you can have funky fonts idk
void gui_draw_text(computer_t *computer, int font_index, char text[], vec2i_t pos, int color) {
	font_t *font = &computer->ram->fonts[font_index];

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
			// api_spr(computer, sprite_index, new_x, new_y, 1, 1, 1);
			api_spr(computer, sprite_index, new_x, new_y, 1, 1);
			new_x += 16 + font->horizontal_space;

			i = index - 1;

			continue;
		}

		// Get the correct sprite index keeping in mind some fonts could have multiple sprites per character (not tested for more than 1 horizontal sprite)
		int char_index = text[i] - VISIBLE_CHARACTERS_START;
		int x_offset = (char_index % (SPRITES_PER_ROW / font->h_sprites)) * font->h_sprites;
		int y_offset = (char_index / (SPRITES_PER_ROW / font->h_sprites)) * font->v_sprites;
		int sprite_index = font->sprite_index + x_offset + (y_offset * SPRITES_PER_ROW);

		recti_t rect = sprite_index_to_spritesheet_rect(sprite_index, font->h_sprites, font->v_sprites);

		for (int i = 0; i < rect.h; i++) {
			for (int j = 0; j < rect.w; j++) {
				// uint8_t font_color = computer->ram->spritesheet.data[(rect.y + i) * SPRITESHEET_WIDTH + (rect.x + j)];
				color_t font_color = gfx_spritesheet_get_pixel(&computer->ram->spritesheet, (vec2i_t){rect.x + j, rect.y + i});
				if (font_color == 15) {
					gfx_set_pixel(&computer->ram->framebuffer, new_x + j, new_y + i, color);
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

// TODO: make versions of these such that the rect is both in and out if that makes sense
// just make it nicer to use bc now it's pretty bad
void gui_outset_frame(computer_t *computer, recti_t rect) {
	int x = rect.x;
	int y = rect.y;
	int w = rect.w;
	int h = rect.h;

	// Because we draw lines it will include x + w or y + h in the pixels drawn
	// Which we don't want so subtract 1
	w--;
	h--;

	// Top gray line
	api_line(computer, x, y, x + w - 1, y, 7);
	// Left gray line
	api_line(computer, x, y, x, y + h - 1, 7);

	// Top white line
	api_line(computer, x + 1, y + 1, x + w - 1, y + 1, 15);
	// Left white line
	api_line(computer, x + 1, y + 1, x + 1, y + h - 1, 15);

	// Bottom black line
	api_line(computer, x, y + h, x + w, y + h, 0);
	// Right black line
	api_line(computer, x + w, y, x + w, y + h, 0);

	// Bottom gray line
	api_line(computer, x + 1, y + h - 1, x + w - 1, y + h - 1, 23);
	// Right gray line
	api_line(computer, x + w - 1, y + 1, x + w - 1, y + h - 1, 23);

	api_rectf(computer, x + 2, y + 2, w - 3, h - 3, 7);
}

void gui_inset_frame(computer_t *computer, recti_t rect) {
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
	api_line(computer, x, y, x + w - 1, y, 23);
	// Left gray line
	api_line(computer, x, y, x, y + h - 1, 23);

	// Top black line
	api_line(computer, x + 1, y + 1, x + w - 1, y + 1, 0);
	// Left black line
	api_line(computer, x + 1, y + 1, x + 1, y + h - 1, 0);

	// Bottom white line
	api_line(computer, x, y + h, x + w, y + h, 15);
	// Right white line
	api_line(computer, x + w, y, x + w, y + h, 15);

	// Bottom gray line
	api_line(computer, x + 1, y + h - 1, x + w - 1, y + h - 1, 7);
	// Right gray line
	api_line(computer, x + w - 1, y + 1, x + w - 1, y + h - 1, 7);

	// api_rectf(computer, x + 2, y + 2, w - 3, h - 3, 15);
}

bool gui_button_ex(computer_t *computer, char text[], recti_t rect, bool already_pressed) {
	// int x, y;
	// get_mouse_pos(&x, &y);
	vec2i_t mouse_pos = input_get_mouse_pos();

	bool return_value = false;
	
	if (point_in_recti(mouse_pos, rect)) {
		if (api_mouse_btn(computer, MOUSE_BUTTON_LEFT)) {
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
		
		gui_inset_frame(computer, new_rect);
		// api_text(computer, text, rect.x + 3, rect.y + 3, 0);
		api_text(computer, 0, text, rect.x + 3, rect.y + 3, 0);

	} else {
		gui_outset_frame(computer, rect);
		api_text(computer, 0, text, rect.x + 3, rect.y + 3, 0);
	}
	
	return return_value;
}

bool gui_button(computer_t *computer, char text[], recti_t rect) {
	return gui_button_ex(computer, text, rect, false);
}

// TODO: investigate why this function exists because apparantly I forgot
// aha it only returns true once when clicked, instead of as long as the mouse button is held
// I should probably refactor this a little bit
bool gui_press_button(computer_t *computer, char text[], recti_t rect) {
	// int x, y;
	// get_mouse_pos(&x, &y);
	vec2i_t mouse_pos = input_get_mouse_pos();

	bool pressed = false;
	bool held = false;
	
	if (point_in_recti(mouse_pos, rect)) {
		if (api_mouse_btn(computer, MOUSE_BUTTON_LEFT)) {
			held = true;
		}

		if (api_mouse_btnp(computer, MOUSE_BUTTON_LEFT)) {
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
		
		gui_inset_frame(computer, new_rect);
		api_text(computer, 2, text, rect.x + 2, rect.y + 2, 2);

	} else {
		gui_outset_frame(computer, rect);
		api_text(computer, 2, text, rect.x + 2, rect.y + 2, 4);
	}
	
	return pressed;
}

// TODO: move to GUI
bool gui_toggle_button(computer_t *computer, char text[], recti_t rect, bool *pressed) {
	vec2i_t mouse_pos = input_get_mouse_pos();
	
	if (point_in_recti(mouse_pos, rect)) {
		if (api_mouse_btnp(computer, MOUSE_BUTTON_LEFT)) {
			*pressed = !(*pressed);
		}
	}
	
	if (*pressed) {
		recti_t new_rect = {
			.x = rect.x + 2,
			.y = rect.y + 2,
			.w = rect.w - 4,
			.h = rect.h - 4,
		};
		
		gui_inset_frame(computer, new_rect);
		api_text(computer, 2, text, rect.x + 2, rect.y + 2, 2);

	} else {
		gui_outset_frame(computer, rect);
		api_text(computer, 2, text, rect.x + 2, rect.y + 2, 4);
	}
	
	return *pressed;
}

int gui_get_text_width(font_t *font, char text[], int max_offset) {
	int len = 0;

	for (int i = 0; i < max_offset; i++) {
		if (text[i] == '\t') {
			if (font->monospace) {
				len += (font->width + font->horizontal_space) * TAB_SIZE;
			} else {
				len += (font->widths[text[' '] - VISIBLE_CHARACTERS_START] + font->horizontal_space) * TAB_SIZE;
			}

			continue;
		}

		if (font->monospace) {
			len += font->width + font->horizontal_space;
		} else {
			len += font->widths[text[i] - VISIBLE_CHARACTERS_START] + font->horizontal_space;
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