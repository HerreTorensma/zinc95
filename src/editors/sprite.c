#include "sprite.h"

#include <stdio.h>

#include "../api/api.h"
#include "../backend/backend.h"
#include "../util/util.h"

static rect_t screen_rect = {0};
static rect_t sprite_sheet_rect = {0};
static rect_t color_picker_rect = {0};
static rect_t sprite_editor_rect = {0};

static uint8_t selected_color = 0;
static int selected_index = 0;

void sprite_editor_init(computer_t *computer) {
	screen_rect = (rect_t){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	sprite_sheet_rect = (rect_t){4, 152, 256, 256};
	color_picker_rect = (rect_t){4, sprite_sheet_rect.y + sprite_sheet_rect.h + 4, 256, 64};
	sprite_editor_rect = (rect_t){324, sprite_sheet_rect.y, 256, 256};
}

void sprite_editor_update(computer_t *computer) {
	int x, y;
	get_mouse_pos(&x, &y);

	if (point_in_rect(x, y, sprite_sheet_rect)) {
		if (api_mouse_btn(computer, 1)) {
			int cell_x = (x - sprite_sheet_rect.x) / SPRITE_WIDTH;
			int cell_y = (y - sprite_sheet_rect.y) / SPRITE_HEIGHT;

			selected_index = cell_y * SPRITE_SHEET_WIDTH + cell_x;
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
		int cell_x = (x - sprite_editor_rect.x) / 16;
		int cell_y = (y - sprite_editor_rect.y) / 16;
		
		if (api_mouse_btn(computer, 1)) {
			computer->ram->spritesheet.sprites[selected_index].data[cell_y * SPRITE_WIDTH + cell_x] = selected_color;
		}

		if (api_mouse_btn(computer, 3)) {
			computer->ram->spritesheet.sprites[selected_index].data[cell_y * SPRITE_WIDTH + cell_x] = 0;
		}
	}
}

void sprite_editor_draw(computer_t *computer) {
	api_cls(computer, 7);

	draw_out_frame(computer, screen_rect);

	draw_in_frame(computer, sprite_sheet_rect);
	for (int i = 0; i < SPRITE_PAGE_HEIGHT; i++) {
		for (int j = 0; j < SPRITE_SHEET_WIDTH; j++) {
			int index = i * SPRITE_SHEET_WIDTH + j;
			int x = sprite_sheet_rect.x + (j * SPRITE_WIDTH);
			int y = sprite_sheet_rect.y + i * SPRITE_HEIGHT;
			api_spr(computer, index, x, y, 1, 1);
		}
	}
	api_rect(
		computer,
		sprite_sheet_rect.x + (selected_index % SPRITE_SHEET_WIDTH) * SPRITE_WIDTH - 1,
		sprite_sheet_rect.y + (selected_index / SPRITE_SHEET_WIDTH) * SPRITE_HEIGHT - 1,
		SPRITE_WIDTH + 2,
		SPRITE_HEIGHT + 2,
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
	api_sspr(computer, selected_index, sprite_editor_rect.x, sprite_editor_rect.y, 1, 1, 16);

	api_text(computer, "Font rendering works let's go!!!", 32, 32, 0);

	char buffer[32];
	sprintf(buffer, "spr: %04d\n", selected_index);
	api_text(computer, buffer, sprite_sheet_rect.x, sprite_sheet_rect.y - 16, 0);
}