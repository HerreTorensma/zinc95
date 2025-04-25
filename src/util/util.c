#include "util.h"
#include "../api/api.h"
#include "../backend/input.h"

#include <stdio.h>

// TODO: make versions of these such that the rect is both in and out if that makes sense
// just make it nicer to use bc now it's pretty bad
void draw_out_frame(computer_t *computer, recti_t rect) {
	int x = rect.x;
	int y = rect.y;
	int w = rect.w;
	int h = rect.h;

	// Because we draw lines it will include x + w or y + h in the pixels drawn
	// Which we don't want so subtract 1
	w--;
	h--;

	// Top gray line
	api_line(computer, x, y, x + w - 1, y, 7);
	// Left gray line
	api_line(computer, x, y, x, y + h - 1, 7);

	// Top white line
	api_line(computer, x + 1, y + 1, x + w - 1, y + 1, 15);
	// Left white line
	api_line(computer, x + 1, y + 1, x + 1, y + h - 1, 15);

	// Bottom black line
	api_line(computer, x, y + h, x + w, y + h, 0);
	// Right black line
	api_line(computer, x + w, y, x + w, y + h, 0);

	// Bottom gray line
	api_line(computer, x + 1, y + h - 1, x + w - 1, y + h - 1, 23);
	// Right gray line
	api_line(computer, x + w - 1, y + 1, x + w - 1, y + h - 1, 23);

	api_rectf(computer, x + 2, y + 2, w - 3, h - 3, 7);
}

void draw_in_frame(computer_t *computer, recti_t rect) {
	int x = rect.x;
	int y = rect.y;
	int w = rect.w;
	int h = rect.h;

	x -= 2;
	y -= 2;
	w += 4;
	h += 4;

	// Again, because we draw lines it will include x + w or y + h in the pixels drawn
	// Which we don't want so subtract 1
	w--;
	h--;

	// Top gray line
	api_line(computer, x, y, x + w - 1, y, 23);
	// Left gray line
	api_line(computer, x, y, x, y + h - 1, 23);

	// Top black line
	api_line(computer, x + 1, y + 1, x + w - 1, y + 1, 0);
	// Left black line
	api_line(computer, x + 1, y + 1, x + 1, y + h - 1, 0);

	// Bottom white line
	api_line(computer, x, y + h, x + w, y + h, 15);
	// Right white line
	api_line(computer, x + w, y, x + w, y + h, 15);

	// Bottom gray line
	api_line(computer, x + 1, y + h - 1, x + w - 1, y + h - 1, 7);
	// Right gray line
	api_line(computer, x + w - 1, y + 1, x + w - 1, y + h - 1, 7);

	// api_rectf(computer, x + 2, y + 2, w - 3, h - 3, 15);
}

// TODO: rename appear_pressed to already_pressed
bool button_ex(computer_t *computer, char text[], recti_t rect, bool appear_pressed) {
	// int x, y;
	// get_mouse_pos(&x, &y);
	vec2i_t mouse_pos = input_mouse_pos();

	bool return_value = false;
	
	if (point_in_recti(mouse_pos, rect)) {
		if (api_mouse_btn(computer, MOUSE_BUTTON_LEFT)) {
			appear_pressed = true;
			return_value = true;
		}
	}
	
	if (appear_pressed) {
		recti_t new_rect = {
			.x = rect.x + 2,
			.y = rect.y + 2,
			.w = rect.w - 4,
			.h = rect.h - 4,
		};
		
		draw_in_frame(computer, new_rect);
		// api_text(computer, text, rect.x + 3, rect.y + 3, 0);
		api_text(computer, 0, text, rect.x + 3, rect.y + 3, 0);

	} else {
		draw_out_frame(computer, rect);
		api_text(computer, 0, text, rect.x + 3, rect.y + 3, 0);
	}
	
	return return_value;
}

bool button(computer_t *computer, char text[], recti_t rect) {
	return button_ex(computer, text, rect, false);
}

// TODO: investigate why this function exists because apparantly I forgot
bool press_button(computer_t *computer, char text[], recti_t rect) {
	// int x, y;
	// get_mouse_pos(&x, &y);
	vec2i_t mouse_pos = input_mouse_pos();

	bool pressed = false;
	bool held = false;
	
	if (point_in_recti(mouse_pos, rect)) {
		if (api_mouse_btn(computer, MOUSE_BUTTON_LEFT)) {
			held = true;
		}

		if (api_mouse_btnp(computer, MOUSE_BUTTON_LEFT)) {
			pressed = true;
		}
	}
	
	if (held) {
		recti_t new_rect = {
			.x = rect.x + 2,
			.y = rect.y + 2,
			.w = rect.w - 4,
			.h = rect.h - 4,
		};
		
		draw_in_frame(computer, new_rect);
		api_text(computer, 2, text, rect.x + 2, rect.y + 2, 2);

	} else {
		draw_out_frame(computer, rect);
		api_text(computer, 2, text, rect.x + 2, rect.y + 2, 4);
	}
	
	return pressed;
}

// TODO: move to GUI
bool toggle_button(computer_t *computer, char text[], recti_t rect, bool *pressed) {
	vec2i_t mouse_pos = input_mouse_pos();
	
	if (point_in_recti(mouse_pos, rect)) {
		if (api_mouse_btnp(computer, MOUSE_BUTTON_LEFT)) {
			*pressed = !(*pressed);
		}
	}
	
	if (*pressed) {
		recti_t new_rect = {
			.x = rect.x + 2,
			.y = rect.y + 2,
			.w = rect.w - 4,
			.h = rect.h - 4,
		};
		
		draw_in_frame(computer, new_rect);
		api_text(computer, 2, text, rect.x + 2, rect.y + 2, 2);

	} else {
		draw_out_frame(computer, rect);
		api_text(computer, 2, text, rect.x + 2, rect.y + 2, 4);
	}
	
	return *pressed;
}