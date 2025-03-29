#include "map.h"
#include "menu.h"

#include "../api/api.h"

void map_editor_init(computer_t *computer) {

}

void map_editor_update(computer_t *computer) {

}

void map_editor_draw(computer_t *computer) {
	api_rectf(computer, workspace_rect.x, workspace_rect.y, workspace_rect.w, workspace_rect.h, 8);
}