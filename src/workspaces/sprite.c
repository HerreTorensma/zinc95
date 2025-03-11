#include "sprite.h"

#include <stdio.h>

#include "../api/api.h"
#include "../backend/backend.h"
#include "../backend/input.h"
#include "../util/util.h"

static rect_t sprite_sheet_rect = {0};
static rect_t color_picker_rect = {0};
static rect_t sprite_editor_rect = {0};

static uint8_t selected_color = 0;
static int selected_index = 0;
static int selected_spritesheet_index = 0;
static rect_t spritesheet_visible_rect = {0};

void sprite_editor_init(computer_t *computer) {
	int sprite_width = computer->ram->spritesheets[selected_spritesheet_index].sprite_width;
	int sprite_height = computer->ram->spritesheets[selected_spritesheet_index].sprite_height;

	sprite_sheet_rect = (rect_t){4, 152, 256, 256};
	color_picker_rect = (rect_t){4, sprite_sheet_rect.y + sprite_sheet_rect.h + 4, 256, 64};
	sprite_editor_rect = (rect_t){324, sprite_sheet_rect.y, 256, 256};

	spritesheet_visible_rect = (rect_t){
		.x = 0,
		.y = 0,
		.w = sprite_width,
		.h = sprite_height,
	};
}

void sprite_editor_update(computer_t *computer) {
	int sprite_width = computer->ram->spritesheets[selected_spritesheet_index].sprite_width;
	int sprite_height = computer->ram->spritesheets[selected_spritesheet_index].sprite_height;

	int x, y;
	get_mouse_pos(&x, &y);

	if (api_keyp(computer, KEY_MINUS)) {
		spritesheet_visible_rect.w -= sprite_width;
		spritesheet_visible_rect.h -= sprite_height;
		
		if (spritesheet_visible_rect.w <= 0) {
			spritesheet_visible_rect.w = sprite_width;
		}
		if (spritesheet_visible_rect.h <= 0) {
			spritesheet_visible_rect.h = sprite_height;
		}
	}
	if (api_keyp(computer, KEY_EQUALS)) {
		spritesheet_visible_rect.w += sprite_width;
		spritesheet_visible_rect.h += sprite_height;
	}

	if (point_in_rect(x, y, sprite_sheet_rect)) {
		if (api_mouse_btn(computer, 1)) {
			int cell_x = (x - sprite_sheet_rect.x) / sprite_width;
			int cell_y = (y - sprite_sheet_rect.y) / sprite_height;

			selected_index = cell_y * (SPRITE_SHEET_WIDTH / sprite_width) + cell_x;

			spritesheet_visible_rect.x = cell_x * sprite_width;
			spritesheet_visible_rect.y = cell_y * sprite_height;

			// Free movement
			// spritesheet_visible_rect.x = x - sprite_sheet_rect.x;
			// spritesheet_visible_rect.y = y - sprite_sheet_rect.y;
		}
	}

	if (point_in_rect(x, y, color_picker_rect)) {
		if (api_mouse_btn(computer, 1)) {
			int cell_x = (x - color_picker_rect.x) / 8;
			int cell_y = (y - color_picker_rect.y) / 8;

			selected_color = cell_y * 32 + cell_x;
		}
	}

	if (point_in_rect(x, y, sprite_editor_rect)) {
		// int cell_x = (x - sprite_editor_rect.x) / (sprite_editor_rect.w / sprite_width);
		// int cell_y = (y - sprite_editor_rect.y) / (sprite_editor_rect.h / sprite_height);
		int cell_x = (x - sprite_editor_rect.x) / (sprite_editor_rect.w / spritesheet_visible_rect.w);
		int cell_y = (y - sprite_editor_rect.y) / (sprite_editor_rect.h / spritesheet_visible_rect.h);
		
		if (api_mouse_btn(computer, 1)) {
			if (api_key(computer, KEY_LALT) || api_key(computer, KEY_RALT)) {
				selected_color = computer->ram->spritesheets[selected_spritesheet_index].data[(spritesheet_visible_rect.y + cell_y) * SPRITE_SHEET_WIDTH + (spritesheet_visible_rect.x + cell_x)];
			}

			computer->ram->spritesheets[selected_spritesheet_index].data[(spritesheet_visible_rect.y + cell_y) * SPRITE_SHEET_WIDTH + (spritesheet_visible_rect.x + cell_x)] = selected_color;

		}

		if (api_mouse_btn(computer, 3)) {
			computer->ram->spritesheets[selected_spritesheet_index].data[(spritesheet_visible_rect.y + cell_y) * SPRITE_SHEET_WIDTH + (spritesheet_visible_rect.x + cell_x)] = 0;
		}
	}
}

void sprite_editor_draw(computer_t *computer) {
	int sprite_width = computer->ram->spritesheets[selected_spritesheet_index].sprite_width;
	int sprite_height = computer->ram->spritesheets[selected_spritesheet_index].sprite_height;

	draw_in_frame(computer, sprite_sheet_rect);
	draw_sprite_sheet_rect(computer, selected_spritesheet_index, sprite_sheet_rect.x, sprite_sheet_rect.y, (rect_t){0, 0, SPRITE_SHEET_WIDTH, SPRITE_SHEET_HEIGHT});

	api_rect(
		computer,
		sprite_sheet_rect.x + spritesheet_visible_rect.x,
		sprite_sheet_rect.y + spritesheet_visible_rect.y,
		spritesheet_visible_rect.w,
		spritesheet_visible_rect.h,
		15
	);

	draw_in_frame(computer, color_picker_rect);
	for (int i = 0; i < PALETTE_SIZE; i++) {
		api_rectf(computer, color_picker_rect.x + (i % 32) * 8, color_picker_rect.y + (i / 32) * 8, 8, 8, i);
	}

	api_rect(
		computer,
		color_picker_rect.x + (selected_color % 32) * 8 - 1,
		color_picker_rect.y + (selected_color / 32) * 8 - 1,
		8 + 2,
		8 + 2,
		15
	);

	draw_in_frame(computer, sprite_editor_rect);
	draw_sprite_sheet_rect_scaled(computer, selected_spritesheet_index, sprite_editor_rect.x, sprite_editor_rect.y, spritesheet_visible_rect, 256 / spritesheet_visible_rect.w);

	char buffer[32];
	draw_in_frame(computer, (rect_t){sprite_sheet_rect.x, sprite_sheet_rect.y - 40, 16, 16});
	api_rectf(computer, sprite_sheet_rect.x, sprite_sheet_rect.y - 40, 16, 16, selected_color);
	sprintf(buffer, "#%03d\n", selected_color);
	api_text(computer, 0, buffer, sprite_sheet_rect.x + 20, sprite_sheet_rect.y - 36, 0);

	draw_in_frame(computer, (rect_t){sprite_sheet_rect.x, sprite_sheet_rect.y - 20, 16, 16});

	// Idk what this is anymore but I might need it later
	// api_rectf(computer, sprite_sheet_rect.x, sprite_sheet_rect.y - 16, 16, 16, selected_color);
	// api_spr(computer, selected_index, sprite_sheet_rect.x, sprite_sheet_rect.y - 20, 1, 1);
	
	// Preview
	// api_spr(computer, selected_spritesheet_index, selected_index, sprite_sheet_rect.x, sprite_sheet_rect.y - 20, 1, 1);
	
	sprintf(buffer, "#%04d\n", selected_index);
	api_text(computer, 0, buffer, sprite_sheet_rect.x + 20, sprite_sheet_rect.y - 16, 0);
}