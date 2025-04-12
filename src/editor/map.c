#include "map.h"
#include "menu.h"

#include "../api/api.h"
#include "../util/util.h"

static rect_t spritesheet_rect = {0};
static rect_t visible_rect = {0};

void map_editor_init(computer_t *computer) {
	spritesheet_rect = (rect_t){200, 348, 384, 128};

	visible_rect = (rect_t){
		.x = 0,
		// .y = selected_spritesheet_index * SPRITESHEET_PAGE_HEIGHT,
		.y = 0,
		.w = SPRITESHEET_PAGE_WIDTH,
		.h = SPRITESHEET_PAGE_HEIGHT,
	};
}

void map_editor_update(computer_t *computer) {

}

void map_editor_draw(computer_t *computer) {
	api_rectf(computer, workspace_rect.x, workspace_rect.y, workspace_rect.w, workspace_rect.h, 8);

	draw_in_frame(computer, spritesheet_rect);
	draw_sprite_sheet_rect(computer, spritesheet_rect.x, spritesheet_rect.y, visible_rect, 255);
}