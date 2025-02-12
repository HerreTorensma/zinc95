#include "sprite.h"

#include "../api/api.h"
#include "../backend/backend.h"

#define SPRITE_SHEET_X 0
// #define SPRITE_SHEET_Y 40
#define SPRITE_SHEET_Y 160

#define COLOR_PICKER_X 0
#define COLOR_PICKER_Y (SPRITE_SHEET_Y + 256)

static uint8_t selected_color = 63;
static int selected_index = 34;

void sprite_editor_update(computer_t *computer) {
	int x, y;
	get_mouse_pos(&x, &y);
}

void sprite_editor_draw(computer_t *computer) {
	api_cls(computer, 7);

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

	api_sspr(computer, 0, 384, 160, 1, 1, 16);
}