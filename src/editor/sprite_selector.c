#include "sprite_selector.h"

static rect_t spritesheet_rect = {0};
static rect_t visible_rect = {0};
static rect_t currently_editing_rect = {0};

static int selected_sprite_index_offset = 0;
static int selected_spritesheet_index = 0;

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

}

void sprite_selector_draw(computer_t *computer) {

}

void sprite_editor_set_selected_spritesheet_index(int index) {
	selected_spritesheet_index = index;
	visible_rect = (rect_t){
		.x = 0,
		.y = selected_spritesheet_index * SPRITESHEET_PAGE_HEIGHT,
		.w = SPRITESHEET_PAGE_WIDTH,
		.h = SPRITESHEET_PAGE_HEIGHT,
	};
}