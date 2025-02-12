#include "sprite.h"

#include "../api/api.h"
#include "../backend/backend.h"
#include "../util/util.h"

#define SPRITE_SHEET_X 4
#define SPRITE_SHEET_Y 152

#define COLOR_PICKER_X 4
#define COLOR_PICKER_Y (SPRITE_SHEET_Y + 256 + 4)

static uint8_t selected_color = 63;
static int selected_index = 34;

void sprite_editor_update(computer_t *computer) {
	int x, y;
	get_mouse_pos(&x, &y);
}

void sprite_editor_draw(computer_t *computer) {
	api_cls(computer, 7);

	draw_out_frame(computer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	draw_in_frame(computer, SPRITE_SHEET_X - 2, SPRITE_SHEET_Y - 2, 256+4, 256+4);
	for (int i = 0; i < SPRITE_PAGE_HEIGHT; i++) {
		for (int j = 0; j < SPRITE_SHEET_WIDTH; j++) {
			int index = i * SPRITE_SHEET_WIDTH + j;
			int x = SPRITE_SHEET_X + (j * SPRITE_WIDTH);
			int y = SPRITE_SHEET_Y + i * SPRITE_HEIGHT;
			api_spr(computer, index, x, y, 1, 1);
		}
	}
	api_rect(
		computer,
		SPRITE_SHEET_X + (selected_index % SPRITE_SHEET_WIDTH) * SPRITE_WIDTH - 1,
		SPRITE_SHEET_Y + (selected_index / SPRITE_SHEET_WIDTH) * SPRITE_HEIGHT - 1,
		SPRITE_WIDTH + 2,
		SPRITE_HEIGHT + 2,
		15
	);

	draw_in_frame(computer, COLOR_PICKER_X - 2, COLOR_PICKER_Y - 2, 256 + 4, 64 + 4);
	for (int i = 0; i < PALETTE_SIZE; i++) {
		api_rectf(computer, COLOR_PICKER_X + (i % 32) * 8, COLOR_PICKER_Y + (i / 32) * 8, 8, 8, i);
	}

	api_rect(
		computer,
		COLOR_PICKER_X + (selected_color % 32) * 8 - 1,
		COLOR_PICKER_Y + (selected_color / 32) * 8 - 1,
		8 + 2,
		8 + 2,
		15
	);

	// draw_in_frame(computer, 324 - 2, 186 - 2, 256+4, 256+4);
	// api_sspr(computer, 0, 324, 186, 1, 1, 16);
	draw_in_frame(computer, 324 - 2, SPRITE_SHEET_Y - 2, 256+4, 256+4);
	api_sspr(computer, 0, 324, SPRITE_SHEET_Y, 1, 1, 16);

	// api_rectf(computer, 256, 256, 64, 64, 2);
	// draw_out_frame(computer, 260, 260, 32, 32);
	// draw_in_frame(computer, 260+32, 260, 32, 32);
}