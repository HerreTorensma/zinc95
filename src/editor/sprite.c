#include "sprite.h"

#include <stdio.h>

#include "../backend/input.h"
#include "../backend/gfx.h"
#include "../backend/gui.h"
#include "menu.h"
#include "shared.h"

#define COLOR_SQUARE_SIZE 8

static rect_t color_picker_rect = {0};
static rect_t sprite_editor_rect = {0};

static uint8_t selected_color = 0;

void sprite_editor_init(computer_t *computer) {
	color_picker_rect = RECT(4, 388, 192, 88);
	sprite_editor_rect = RECT(192, 56, 256, 256);
}

static void color_to_coords(uint8_t color, int *x, int *y) {
	if (color < 32) {
		*x = color_picker_rect.x + (color % 16) * COLOR_SQUARE_SIZE;
		*y = color_picker_rect.y + (color / 16) * COLOR_SQUARE_SIZE;
	} else {
		*x = color_picker_rect.x + ((color - 32) % 24) * COLOR_SQUARE_SIZE;
		*y = color_picker_rect.y + 2 * COLOR_SQUARE_SIZE + ((color - 32) / 24) * COLOR_SQUARE_SIZE;
	}
}

static uint8_t coords_to_color(int x, int y) {
	int cell_x = (x - color_picker_rect.x) / COLOR_SQUARE_SIZE;
	int cell_y = (y - color_picker_rect.y) / COLOR_SQUARE_SIZE;

	if ((cell_y == 0 || cell_y == 1) && cell_x >= 16) {
		return 0;
	}

	if (cell_y == 0) {
		return cell_x;
	} else if (cell_y == 1) {
		return 16 + cell_x;
	} else {
		return 32 + ((cell_y - 2) * 24 + cell_x);
	}
}

void sprite_editor_update(computer_t *computer) {
	point_t mouse_pos = input_get_mouse_pos();
	
	sprite_selector_update(computer, SNAP_MODE_ZOOM);

	if (point_in_rect(mouse_pos, color_picker_rect)) {
		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			selected_color = coords_to_color(mouse_pos.x, mouse_pos.y);
		}
	}

	if (point_in_rect(mouse_pos, sprite_editor_rect)) {
		int cell_x = (mouse_pos.x - sprite_editor_rect.x) / (sprite_editor_rect.w / currently_editing_rect.w);
		int cell_y = (mouse_pos.y - sprite_editor_rect.y) / (sprite_editor_rect.h / currently_editing_rect.h);
		
		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			if (input_key_held(KEY_LALT) || input_key_held(KEY_RALT)) {
				// TODO: make a function for this
				selected_color = computer->ram->spritesheet.data[(visible_rect.y + currently_editing_rect.y + cell_y) * SPRITESHEET_WIDTH + (visible_rect.x + currently_editing_rect.x + cell_x)];
			}

			computer->ram->spritesheet.data[(visible_rect.y + currently_editing_rect.y + cell_y) * SPRITESHEET_WIDTH + (visible_rect.x + currently_editing_rect.x + cell_x)] = selected_color;

		}

		if (input_mouse_button_held(MOUSE_BUTTON_RIGHT)) {
			computer->ram->spritesheet.data[(visible_rect.y + currently_editing_rect.y + cell_y) * SPRITESHEET_WIDTH + (visible_rect.x + currently_editing_rect.x + cell_x)] = 0;
		}
	}

	// Delete sprite
	if (input_key_pressed(KEY_DELETE)) {
		// TODO: add visible rect stuff
		for (int y = currently_editing_rect.y; y < currently_editing_rect.y + currently_editing_rect.h; y++) {
			for (int x = currently_editing_rect.x; x < currently_editing_rect.x + currently_editing_rect.w; x++) {
				computer->ram->spritesheet.data[y * SPRITESHEET_WIDTH + x] = 0;
			}
		}

		// TODO: clear this stuff for every sprite in the selection
		computer->ram->sprites[get_selected_sprite_index()].color_key = 0;
		computer->ram->sprites[get_selected_sprite_index()].flags = 0U;
	}
}

void sprite_editor_draw(computer_t *computer) {
	framebuffer_t *fb = &computer->ram->framebuffer;

	// Spritesheet / sprite selector
	sprite_selector_draw(computer);

	// Color picker frame
	gui_inset_frame(computer->ram, color_picker_rect);
	gfx_draw_filled_rect(fb, color_picker_rect, 0);

	// Draw colors
	for (int i = 0; i < PALETTE_SIZE - 8; i++) {
		int color_square_x, color_square_y;
		color_to_coords(i, &color_square_x, &color_square_y);
		gfx_draw_filled_rect(fb, RECT(color_square_x, color_square_y, COLOR_SQUARE_SIZE, COLOR_SQUARE_SIZE), i);
	}

	// Draw selected color square
	int color_square_x, color_square_y;
	color_to_coords(selected_color, &color_square_x, &color_square_y);
	gfx_draw_rect(fb, RECT(color_square_x - 1, color_square_y - 1, COLOR_SQUARE_SIZE + 2, COLOR_SQUARE_SIZE + 2), 15);

	// Sprite editor
	gui_inset_frame(computer->ram, sprite_editor_rect);
	rect_t sprite_editing_rect = {
		.x = visible_rect.x + currently_editing_rect.x,
		.y = visible_rect.y + currently_editing_rect.y,
		.w = currently_editing_rect.w,
		.h = currently_editing_rect.h,
	};

	int scale = sprite_editor_rect.w / currently_editing_rect.w;
	rect_t real_editor_rect = {
		.x = sprite_editor_rect.x,
		.y = sprite_editor_rect.y,
		.w = currently_editing_rect.w * scale,
		.h = currently_editing_rect.h * scale,
	};
	gfx_draw_filled_rect(fb, sprite_editor_rect, 151);
	gfx_draw_spritesheet_pro(computer->ram, sprite_editing_rect, real_editor_rect, COLOR_NONE);

	// Selected color
	char buffer[32];
	gui_inset_frame(computer->ram, RECT(color_picker_rect.x, color_picker_rect.y - 20, 16, 16));
	gfx_draw_filled_rect(fb, RECT(color_picker_rect.x, color_picker_rect.y - 20, 16, 16), selected_color);
	sprintf(buffer, "#%03d\n", selected_color);
	gui_draw_text(computer->ram, 0, buffer, POINT(color_picker_rect.x + 20, color_picker_rect.y - 16), 0);
	
	// Selected sprite preview
	gui_inset_frame(computer->ram, RECT(color_picker_rect.x, color_picker_rect.y - 40, 16, 16));
	gfx_draw_sprites(computer->ram, get_selected_sprite_index(), POINT(color_picker_rect.x, color_picker_rect.y - 40), 1, 1);
	sprintf(buffer, "#%04d\n", get_selected_sprite_index());
	gui_draw_text(computer->ram, 0, buffer, POINT(color_picker_rect.x + 20, color_picker_rect.y - 36), 0);

	// Sprite flags and color key
	sprite_t *selected_sprite = &computer->ram->sprites[get_selected_sprite_index()];
	for (int i = 0; i < SPRITE_FLAGS_SIZE; i++) {
		sprintf(buffer, "%c", i < 10 ? '0' + i : 'a' + i - 10);
		
		bool set = selected_sprite->flags & (1U << i);
		set = gui_toggle_button(computer->ram, buffer, RECT(spritesheet_rect.x + i * 12, spritesheet_rect.y - 4 - 12, 12, 12), set);

		if (set) {
			selected_sprite->flags |= (1U << i);
		} else {
			selected_sprite->flags &= ~(1U << i);
		}
	}

	// Color key
	if (gui_button(computer->ram, "", RECT(SCREEN_WIDTH - 2 - 12 - 4, spritesheet_rect.y - 12 - 4, 12, 12))) {
		selected_sprite->color_key = selected_color;
	}

	gfx_draw_filled_rect(fb, RECT(SCREEN_WIDTH - 2 - 12 - 4 + 2, spritesheet_rect.y - 12 - 4 + 2, 8, 8), selected_sprite->color_key);
	gui_draw_text(computer->ram, 2, "Key:", POINT(spritesheet_rect.x + spritesheet_rect.w + 4 + 2, spritesheet_rect.y - 12 - 4 + 2), 0);
}