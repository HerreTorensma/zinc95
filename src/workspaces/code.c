#include "code.h"

#include <string.h>

#include "../api/api.h"
#include "../util/util.h"

static rect_t code_rect = {
	.x = 4,
	.y = 22,
	.w = SCREEN_WIDTH - 8,
	.h = SCREEN_HEIGHT - 20 - 6,
};

static char sample_string[] =	"local x = 0;\n"
								"local y = 50;\n"
								"\n"
								"function _init()\n"
								"	print(\"Called the init function\")\n"
								"end\n"
								"\n"
								"function _update()\n"
								"	x = x + 1\n"
								"	y = y + 1\n"
								"end\n"
								"\n"
								"function _draw()\n"
								"	cls(2);\n"
								"	spr(0, x, y, 1, 1)\n"
								"end\0";

void code_editor_init(computer_t *computer) {
	memcpy(computer->code->buffer, sample_string, sizeof(sample_string));
}

void code_editor_update(computer_t *computer) {

}

void code_editor_draw(computer_t *computer) {
	draw_in_frame(computer, code_rect);
	api_rectf(computer, code_rect.x, code_rect.y, code_rect.w, code_rect.h, 15);

	api_text(computer, computer->code->buffer, code_rect.x + 2, code_rect.y + 2, 0);
}
