#include "shared.h"

#include <stdio.h>

#include "../api/api.h"
#include "../backend/input.h"
#include "../backend/backend.h"
#include "../util/util.h"

rect_t spritesheet_rect = {0};
rect_t visible_rect = {0};
rect_t currently_editing_rect = {0};

int selected_sprite_index_offset = 0;
int selected_spritesheet_index = 0;

static void set_selected_spritesheet_index(int index) {
	selected_spritesheet_index = index;
	visible_rect = (rect_t){
		.x = 0,
		.y = selected_spritesheet_index * SPRITESHEET_PAGE_HEIGHT,
		.w = SPRITESHEET_PAGE_WIDTH,
		.h = SPRITESHEET_PAGE_HEIGHT,
	};
}

void sprite_selector_init(computer_t *computer) {
	spritesheet_rect = (rect_t){200, 348, 384, 128};

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

void sprite_selector_update(computer_t *computer) {
	int x, y;
	get_mouse_pos(&x, &y);

	if (point_in_rect(x, y, spritesheet_rect)) {
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
}

void sprite_selector_draw(computer_t *computer) {
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
}

int get_selected_sprite_index() {
	return (selected_spritesheet_index * SPRITES_PER_PAGE) + selected_sprite_index_offset;
}