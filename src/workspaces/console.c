#include "console.h"
#include "../api/api.h"
#include "menu.h"

void console_init(computer_t *computer) {
	
}

void console_update(computer_t *computer) {

}

void console_draw(computer_t *computer) {
	api_rectf(computer, workspace_rect.x, workspace_rect.y, workspace_rect.w, workspace_rect.h, 0);
}