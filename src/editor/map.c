#include "map.h"
#include "menu.h"

#include "../api/api.h"
#include "../util/util.h"
#include "shared.h"

void map_editor_init(computer_t *computer) {

}

void map_editor_update(computer_t *computer) {
	sprite_selector_update(computer);
}

void map_editor_draw(computer_t *computer) {
	api_rectf(computer, workspace_rect.x, workspace_rect.y, workspace_rect.w, workspace_rect.h, 8);
	sprite_selector_draw(computer);
}