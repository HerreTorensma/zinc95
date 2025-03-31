#include "sprite.h"

#include <stdio.h>

#include "../api/api.h"
#include "../backend/backend.h"
#include "../backend/input.h"
#include "../util/util.h"
#include "menu.h"

#define COLOR_SQUARE_SIZE 8

static rect_t spritesheet_rect = {0};
static rect_t color_picker_rect = {0};
static rect_t sprite_editor_rect = {0};

static uint8_t selected_color = 0;
static int selected_sprite_index_offset = 0;
static int selected_spritesheet_index = 0;

static rect_t visible_rect = {0};
static rect_t currently_editing_rect = {0};

static void set_selected_spritesheet_index(int index) {
	selected_spritesheet_index = index;
	visible_rect = (rect_t){
		.x = 0,
		.y = selected_spritesheet_index * SPRITESHEET_PAGE_HEIGHT,
		.w = SPRITESHEET_PAGE_WIDTH,
		.h = SPRITESHEET_PAGE_HEIGHT,
	};
}

void sprite_editor_init(computer_t *computer) {
	spritesheet_rect = (rect_t){200, 348, 384, 128};
	color_picker_rect = (rect_t){4, 388, 192, 88};
	sprite_editor_rect = (rect_t){192, 56, 256, 256};

	visible_rect = (rect_t){
		.x = 0,
		.y = selected_spritesheet_index * SPRITESHEET_PAGE_HEIGHT,
		.w = SPRITESHEET_PAGE_WIDTH,
		.h = SPRITESHEET_PAGE_HEIGHT,
	};

	currently_editing_rect = (rect_t){
		.x = 0,
		.y = 0,
		.w = SPRITE_WIDTH,
		.h = SPRITE_HEIGHT,
	};
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
	int x, y;
	get_mouse_pos(&x, &y);

	if (api_keyp(computer, KEY_MINUS) || api_mouse_scrolled(computer, SCROLL_UP)) {
		currently_editing_rect.w -= SPRITE_WIDTH;
		currently_editing_rect.h -= SPRITE_HEIGHT;
		
		if (currently_editing_rect.w <= 0) {
			currently_editing_rect.w = SPRITE_WIDTH;
		}
		if (currently_editing_rect.h <= 0) {
			currently_editing_rect.h = SPRITE_HEIGHT;
		}
	}
	if (api_keyp(computer, KEY_EQUALS) || api_mouse_scrolled(computer, SCROLL_DOWN)) {
		currently_editing_rect.w += SPRITE_WIDTH;
		currently_editing_rect.h += SPRITE_HEIGHT;
	}

	if (point_in_rect(x, y, spritesheet_rect)) {
		if (api_mouse_btn(computer, MOUSE_BUTTON_LEFT)) {
			int cell_x = (x - spritesheet_rect.x) / SPRITE_WIDTH;
			int cell_y = (y - spritesheet_rect.y) / SPRITE_HEIGHT;

			selected_sprite_index_offset = cell_y * (SPRITESHEET_WIDTH / SPRITE_WIDTH) + cell_x;

			currently_editing_rect.x = cell_x * SPRITE_WIDTH;
			currently_editing_rect.y = cell_y * SPRITE_HEIGHT;

			// Free movement
			// currently_editing_rect.x = x - spritesheet_rect.x;
			// currently_editing_rect.y = y - spritesheet_rect.y;
		}
	}

	if (point_in_rect(x, y, color_picker_rect)) {
		if (api_mouse_btn(computer, MOUSE_BUTTON_LEFT)) {
			selected_color = coords_to_color(x, y);
		}
	}

	if (point_in_rect(x, y, sprite_editor_rect)) {
		// int cell_x = (x - sprite_editor_rect.x) / (sprite_editor_rect.w / sprite_width);
		// int cell_y = (y - sprite_editor_rect.y) / (sprite_editor_rect.h / sprite_height);
		int cell_x = (x - sprite_editor_rect.x) / (sprite_editor_rect.w / currently_editing_rect.w);
		int cell_y = (y - sprite_editor_rect.y) / (sprite_editor_rect.h / currently_editing_rect.h);
		
		if (api_mouse_btn(computer, MOUSE_BUTTON_LEFT)) {
			if (api_key(computer, KEY_LALT) || api_key(computer, KEY_RALT)) {
				selected_color = computer->ram->spritesheet.data[(visible_rect.y + currently_editing_rect.y + cell_y) * SPRITESHEET_WIDTH + (visible_rect.x + currently_editing_rect.x + cell_x)];
			}

			computer->ram->spritesheet.data[(visible_rect.y + currently_editing_rect.y + cell_y) * SPRITESHEET_WIDTH + (visible_rect.x + currently_editing_rect.x + cell_x)] = selected_color;

		}

		if (api_mouse_btn(computer, MOUSE_BUTTON_RIGHT)) {
			computer->ram->spritesheet.data[(visible_rect.y + currently_editing_rect.y + cell_y) * SPRITESHEET_WIDTH + (visible_rect.x + currently_editing_rect.x + cell_x)] = 0;
		}
	}
}

static inline int get_sprite_index() {
	return (selected_spritesheet_index * SPRITES_PER_PAGE) + selected_sprite_index_offset;
}

static bool toggle_button(computer_t *computer, char text[], rect_t rect, bool *pressed) {
	int x, y;
	get_mouse_pos(&x, &y);
	
	if (point_in_rect(x, y, rect)) {
		if (api_mouse_btnp(computer, MOUSE_BUTTON_LEFT)) {
			*pressed = !(*pressed);
		}
	}
	
	if (*pressed) {
		rect_t new_rect = {
			.x = rect.x + 2,
			.y = rect.y + 2,
			.w = rect.w - 4,
			.h = rect.h - 4,
		};
		
		draw_in_frame(computer, new_rect);
		api_text(computer, 2, text, rect.x + 2, rect.y + 2, 2);

	} else {
		draw_out_frame(computer, rect);
		api_text(computer, 2, text, rect.x + 2, rect.y + 2, 4);
	}
	
	return *pressed;
}

void sprite_editor_draw(computer_t *computer) {
	draw_in_frame(computer, spritesheet_rect);
	draw_sprite_sheet_rect(computer, spritesheet_rect.x, spritesheet_rect.y, visible_rect, 255);

	for (int i = 0; i < 8; i++) {
		rect_t rect = {
			.x = spritesheet_rect.x + spritesheet_rect.w + 4,
			.y = spritesheet_rect.y + i * 16,
			.w = 48,
			.h = 16,
		};

		char buffer[3];
		sprintf(buffer, "%d", i + 1);
		
		if (button_ex(computer, buffer, rect, selected_spritesheet_index == i)) {
			set_selected_spritesheet_index(i);
		}
	}

	/*
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 2; x++) {
			rect_t rect = {
				.x = spritesheet_rect.x + spritesheet_rect.w + 4 + x * 24,
				.y = spritesheet_rect.y + y * 16,
				.w = 24,
				.h = 16,
			};

			int index = y * 2 + x;
	
			char buffer[3];
			sprintf(buffer, "%d", index + 1);
			
			if (button_ex(computer, buffer, rect, selected_spritesheet_index == index)) {
				set_selected_spritesheet_index(index);
			}
		}
	}
	*/

	api_rect(
		computer,
		spritesheet_rect.x + currently_editing_rect.x - 1,
		spritesheet_rect.y + currently_editing_rect.y - 1,
		currently_editing_rect.w + 2,
		currently_editing_rect.h + 2,
		15
	);

	draw_in_frame(computer, color_picker_rect);
	api_rectf(computer, color_picker_rect.x, color_picker_rect.y, color_picker_rect.w, color_picker_rect.h, 0);

	// Draw colors
	for (int i = 0; i < PALETTE_SIZE - 8; i++) {
		int color_square_x, color_square_y;
		color_to_coords(i, &color_square_x, &color_square_y);
		api_rectf(computer, color_square_x, color_square_y, COLOR_SQUARE_SIZE, COLOR_SQUARE_SIZE, i);
	}

	// Draw selected color square
	int color_square_x, color_square_y;
	color_to_coords(selected_color, &color_square_x, &color_square_y);
	api_rect(computer, color_square_x - 1, color_square_y - 1, COLOR_SQUARE_SIZE + 2, COLOR_SQUARE_SIZE + 2, 15);

	// Sprite editor
	draw_in_frame(computer, sprite_editor_rect);
	rect_t idk = {
		.x = visible_rect.x + currently_editing_rect.x,
		.y = visible_rect.y + currently_editing_rect.y,
		.w = currently_editing_rect.w,
		.h = currently_editing_rect.h,
	};
	// draw_sprite_sheet_rect_scaled(computer, sprite_editor_rect.x, sprite_editor_rect.y, idk, sprite_editor_rect.w / currently_editing_rect.w);
	draw_sprite_sheet_rect_scaled(computer, sprite_editor_rect.x, sprite_editor_rect.y, idk, COLOR_NONE, sprite_editor_rect.w / currently_editing_rect.w);

	// draw_sprite_sheet_rect_scaled()
	// api_sspr(
	// 	computer,
	// 	sprite_editor_rect.x, sprite_editor_rect.y,
		
	// 	visible_rect.x + currently_editing_rect.x,
	// 	visible_rect.y + currently_editing_rect.y,
	// 	currently_editing_rect.w,
	// 	currently_editing_rect.h,

	// 	COLOR_NONE,

	// 	sprite_editor_rect.w / currently_editing_rect.w
	// );

	// Selected color
	char buffer[32];
	draw_in_frame(computer, (rect_t){color_picker_rect.x, color_picker_rect.y - 20, 16, 16});
	api_rectf(computer, color_picker_rect.x, color_picker_rect.y - 20, 16, 16, selected_color);
	sprintf(buffer, "#%03d\n", selected_color);
	api_text(computer, 0, buffer, color_picker_rect.x + 20, color_picker_rect.y - 16, 0);
	
	// Selected sprite preview
	draw_in_frame(computer, (rect_t){color_picker_rect.x, color_picker_rect.y - 40, 16, 16});
	api_spr(computer, get_sprite_index(), color_picker_rect.x, color_picker_rect.y - 40, 1, 1, 2);
	sprintf(buffer, "#%04d\n", get_sprite_index());
	api_text(computer, 0, buffer, color_picker_rect.x + 20, color_picker_rect.y - 36, 0);

	// Sprite flags and color key
	sprite_t *selected_sprite = &computer->ram->sprites[get_sprite_index()];
	for (int i = 0; i < SPRITE_FLAGS_SIZE; i++) {
			sprintf(buffer, "%c", i < 10 ? '0' + i : 'a' + i - 10);
			bool set = selected_sprite->flags & (1U << i);

			toggle_button(computer, buffer, (rect_t){
				.x = spritesheet_rect.x + i * 12,
				.y = spritesheet_rect.y - 4 - 12,
				.w = 12,
				.h = 12,
				},
				&set
			);
			if (set) {
				selected_sprite->flags |= (1U << i);
			} else {
				selected_sprite->flags &= ~(1U << i);
			}
		}

	if (button(computer, "", (rect_t){
		.x = SCREEN_WIDTH - 2 - 12 - 4,
		.y = spritesheet_rect.y - 12 - 4,
		.w = 12,
		.h = 12,
		})) {
		selected_sprite->color_key = selected_color;
	}

	api_rectf(computer, SCREEN_WIDTH - 2 - 12 - 4 + 2, spritesheet_rect.y - 12 - 4 + 2, 8, 8, selected_sprite->color_key);
	api_text(computer, 2, "Key:", spritesheet_rect.x + spritesheet_rect.w + 4 + 2, spritesheet_rect.y - 12 - 4 + 2, 0);
}