#include "gui.h"

#include <string.h>

#include "input.h"
#include "gfx.h"

// Get the rect of the char in the spritesheet
static void _get_char_rect(font_t *font, char c) {

}

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
			new_x += (font->widths[' ' - VISIBLE_CHARACTERS_START] + font->horizontal_space) * TAB_SIZE;
			continue;
		}

		// Inline sprites, will probably remove in favor of making 256 characters available in the fonts
		// if (string.data[i] == '`') {
		// 	int sprite_index = 0;

		// 	// Read the digits after
		// 	size_t index = i + 1;
		// 	while (string.data[index] >= '0' && string.data[index] <= '9') {
		// 		sprite_index *= 10;
		// 		sprite_index += string.data[index] - '0';

		// 		index++;
		// 	}

		// 	gfx_draw_sprites(ram, sprite_index, POINT(new_x, new_y), font->sprite_width, font->sprite_height);
		// 	new_x += 16 + font->horizontal_space;

		// 	i = index - 1;

		// 	continue;
		// }

		// Get the correct sprite index keeping in mind some fonts could have multiple sprites per character (not tested for more than 1 horizontal sprite)
		int char_index = string.data[i] - VISIBLE_CHARACTERS_START;
		int x_offset = (char_index % (SPRITES_PER_ROW / font->sprite_width)) * font->sprite_width;
		int y_offset = (char_index / (SPRITES_PER_ROW / font->sprite_width)) * font->sprite_height;
		int sprite_index = font->sprite_index + x_offset + (y_offset * SPRITES_PER_ROW);

		rect_t rect = sprite_index_to_spritesheet_rect(sprite_index, font->sprite_width, font->sprite_height);

		for (int i = 0; i < rect.h; i++) {
			for (int j = 0; j < rect.w; j++) {
				color_t pixel_color = gfx_spritesheet_get_pixel(&ram->spritesheet, (point_t){rect.x + j, rect.y + i});

				if (pixel_color != font->color_key && pixel_color != font->seperator_color) {
					if (color != COLOR_NONE) {
						gfx_set_pixel(&ram->framebuffer, new_x + j, new_y + i, color);
					} else {
						gfx_set_pixel(&ram->framebuffer, new_x + j, new_y + i, pixel_color);
					}
				}
			}
		}

		new_x += font->widths[string.data[i] - VISIBLE_CHARACTERS_START] + font->horizontal_space;
	}
}

void gui_draw_text(ram_t *ram, int font_index, const char text[], point_t pos, int color) {
	size_t len = strlen(text);
	
	// Dirty typecast, TODO look at this again maybe
	gui_draw_string(ram, font_index, (string_t){.data = (char *)text, .len = len}, pos, color);
}

// Uses the size of the unpressed rect for mouse detection
bool gui_button(ram_t *ram, point_t pos, button_t button, bool already_pressed) {
	point_t mouse_pos = input_get_mouse_pos();

	rect_t rect = {
		.x = pos.x,
		.y = pos.y,
		.w = button.unpressed_rect.w,
		.h = button.unpressed_rect.h,
	};
	
	if (point_in_rect(mouse_pos, rect)) {
		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			already_pressed = true;
		}
	}
	
	if (already_pressed) {
		gfx_draw_surface_rect(&ram->framebuffer, SKIN_SURF(ram->skin.data), pos, button.pressed_rect, COLOR_NONE);
	} else {
		gfx_draw_surface_rect(&ram->framebuffer, SKIN_SURF(ram->skin.data), pos, button.unpressed_rect, COLOR_NONE);
	}
	
	return already_pressed;
}

bool gui_press_button(ram_t *ram, point_t pos, button_t button) {
	point_t mouse_pos = input_get_mouse_pos();

	rect_t rect = {
		.x = pos.x,
		.y = pos.y,
		.w = button.unpressed_rect.w,
		.h = button.unpressed_rect.h,
	};

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
		gfx_draw_surface_rect(&ram->framebuffer, SKIN_SURF(ram->skin.data), pos, button.pressed_rect, COLOR_NONE);
	} else {
		gfx_draw_surface_rect(&ram->framebuffer, SKIN_SURF(ram->skin.data), pos, button.unpressed_rect, COLOR_NONE);
	}
	
	return pressed;
}

bool gui_toggle_button(ram_t *ram, point_t pos, button_t button, bool set) {
	point_t mouse_pos = input_get_mouse_pos();

	rect_t rect = {
		.x = pos.x,
		.y = pos.y,
		.w = button.unpressed_rect.w,
		.h = button.unpressed_rect.h,
	};
	
	if (point_in_rect(mouse_pos, rect)) {
		if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
			set = !set;
		}
	}
	
	if (set) {
		gfx_draw_surface_rect(&ram->framebuffer, SKIN_SURF(ram->skin.data), pos, button.pressed_rect, COLOR_NONE);
	} else {
		gfx_draw_surface_rect(&ram->framebuffer, SKIN_SURF(ram->skin.data), pos, button.unpressed_rect, COLOR_NONE);
	}
	
	return set;
}

int gui_get_string_width(font_t *font, string_t string, int max_offset) {
	int len = 0;

	for (int i = 0; i < (int)string.len && i < max_offset; i++) {
		if (string.data[i] == '\t') {
			len += (font->widths[' ' - VISIBLE_CHARACTERS_START] + font->horizontal_space) * TAB_SIZE;
			continue;
		}

		len += font->widths[string.data[i] - VISIBLE_CHARACTERS_START] + font->horizontal_space;
	}

	return len;
}

int gui_get_text_width(font_t *font, char text[], int max_offset) {
	size_t len = strlen(text);
	return gui_get_string_width(font, (string_t){.data = text, .len = len}, max_offset);
}

// Only works on monospace fonts, uses the width of the space character for the whole string
int gui_x_to_string_index(font_t *font, string_t string, int x) {
	int index = x / (font->widths[0] + font->horizontal_space);
	// int index = x / 8;

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

void gui_init_monospace_font_widths(ram_t *ram, int font_index, int width) {
	for (int i = 0; i < VISIBLE_CHARACTERS_SIZE; i++) {
		ram->fonts[font_index].widths[i] = width;
	}
}

// Init the widths using the seperator lines
// TODO: maybe do this and the height on the fly instead of during loading, will get back to this
void gui_init_font_widths(ram_t *ram, int font_index) {
	font_t *font = &ram->fonts[font_index];

	for (int i = 0; i < VISIBLE_CHARACTERS_SIZE; i++) {
		// Get the correct sprite index keeping in mind some fonts could have multiple sprites per character (not tested for more than 1 horizontal sprite)
		int x_offset = (i % (SPRITES_PER_ROW / font->sprite_width)) * font->sprite_width;
		int y_offset = (i / (SPRITES_PER_ROW / font->sprite_width)) * font->sprite_height;
		int sprite_index = font->sprite_index + x_offset + (y_offset * SPRITES_PER_ROW);

		rect_t rect = sprite_index_to_spritesheet_rect(sprite_index, font->sprite_width, font->sprite_height);
		int width = 0;

		for (int x = 0; x < rect.w; x++) {
			color_t pixel_color = gfx_spritesheet_get_pixel(&ram->spritesheet, (point_t){rect.x + x, rect.y});
			if (pixel_color == font->seperator_color) {
				break;
			}

			width++;
		}

		font->widths[i] = width;
	}
}

rect_t gui_rect_to_outset_frame_rect(rect_t rect) {
	rect.x -= GUI_BORDER_WIDTH;
	rect.y -= GUI_BORDER_WIDTH;
	rect.w += GUI_BORDER_WIDTH * 2;
	rect.h += GUI_BORDER_WIDTH * 2;

	return rect;
}

button_t button_array_get(button_array_t *array, int index) {
	button_t button = array->base;
	
	button.unpressed_rect.x += array->increase.x * index;
	button.unpressed_rect.y += array->increase.y * index;
	button.pressed_rect.x += array->increase.x * index;
	button.pressed_rect.y += array->increase.y * index;

	return button;
}

point_t button_array_get_pos(button_array_t *array, point_t base_pos, int index) {
	point_t pos = base_pos;
	
	pos.x += array->increase.x * index;
	pos.y += array->increase.y * index;

	return pos;
}

void gui_load_skin(ram_t *ram, const char filename[], color_t color_key, color_t font_color) {
	gfx_load_surface(&ram->palette, SKIN_SURF(ram->skin.data), filename);

	// Copy fonts
	gfx_copy_surface_rect(SPR_SURF(ram->spritesheet.data), SKIN_SURF(ram->skin.data), (point_t){0, 896}, skin_layout.gui_font_rect, COLOR_NONE);
	gfx_copy_surface_rect(SPR_SURF(ram->spritesheet.data), SKIN_SURF(ram->skin.data), (point_t){0, 928}, skin_layout.code_editor_font_rect, COLOR_NONE);

	ram->skin.color_key = color_key;
	ram->skin.font_color = font_color;
}

int64_t gui_slider(ram_t *ram, int font_index, rect_t rect, int64_t min, int64_t max, int64_t value) {
	gui_draw_string(ram, font_index, int_to_string(get_temp_allocator(), value), rect.pos, ram->skin.font_color);

	point_t mouse_pos = input_get_mouse_pos();
	if (point_in_rect(mouse_pos, rect)) {
		if (value < max && input_mouse_scrolled(SCROLL_DIR_DOWN)) {
			value++;
		} else if (value > min && input_mouse_scrolled(SCROLL_DIR_UP)) {
			value--;
		}
	}

	return value;
}
