#include "music.h"

#include "../core/audio.h"
#include "gui.h"
#include "../res.h"
#include "menu.h"
#include "sound.h"
#include <stdio.h>

static struct {
	rect_t channel0_rect;
	rect_t channel1_rect;
	rect_t channel2_rect;
	rect_t channel3_rect;
	rect_t channel4_rect;
	rect_t channel5_rect;
	rect_t channel6_rect;
	rect_t channel7_rect;

	point_t channel_button_positions[8];

	point_t arrangement_picker_base_pos;
} _layout = {
	.channel0_rect = {{8 + 0*72, 88, 64, 384}},
	.channel1_rect = {{8 + 1*72, 88, 64, 384}},
	.channel2_rect = {{8 + 2*72, 88, 64, 384}},
	.channel3_rect = {{8 + 3*72, 88, 64, 384}},
	.channel4_rect = {{8 + 4*72, 88, 64, 384}},
	.channel5_rect = {{8 + 5*72, 88, 64, 384}},
	.channel6_rect = {{8 + 6*72, 88, 64, 384}},
	.channel7_rect = {{8 + 7*72, 88, 64, 384}},

	.channel_button_positions[0] = {6 + 72*0, 70},
	.channel_button_positions[1] = {6 + 72*1, 70},
	.channel_button_positions[2] = {6 + 72*2, 70},
	.channel_button_positions[3] = {6 + 72*3, 70},
	.channel_button_positions[4] = {6 + 72*4, 70},
	.channel_button_positions[5] = {6 + 72*5, 70},
	.channel_button_positions[6] = {6 + 72*6, 70},
	.channel_button_positions[7] = {6 + 72*7, 70},

	.arrangement_picker_base_pos = {8, 28},
};

static size_t _active_arrangement_index = 0;

static int _scroll_amount = 0;

void music_editor_init(computer_t *computer) {
	computer->ram->arrangements[_active_arrangement_index].pattern_indices[1] = -1;
	computer->ram->arrangements[_active_arrangement_index].pattern_indices[2] = 1;
	computer->ram->arrangements[_active_arrangement_index].pattern_indices[3] = 2;
}

static void _play_arrangement(computer_t *computer) {
	for (size_t i = 0; i < 8; i++) {
		audio_play_pattern(computer, computer->ram->arrangements[_active_arrangement_index].pattern_indices[i], i);
	}
}

static void _stop_arrangement(computer_t *computer) {
	for (size_t i = 0; i < 8; i++) {
		audio_cancel_channel(computer, i);
	}
}

void music_editor_update(computer_t *computer) {
	if (input_key_pressed(KEY_SPACE)) {
		if (audio_get_channel_current_step(computer, 0) > 0) { // TODO: make the default value -1 or something bc now it's annoying
			_stop_arrangement(computer);
		} else {
			_play_arrangement(computer);
		}
	}

	if (input_mouse_scrolled(SCROLL_DIR_DOWN)) {
		_scroll_amount += 3;
	}
	if (input_mouse_scrolled(SCROLL_DIR_UP)) {
		_scroll_amount -= 3;
	}
	_scroll_amount = clamp_int(_scroll_amount, 0, STEPS_IN_PATTERN - 1);
}

static void _draw_pattern(computer_t *computer, size_t index, point_t base_pos) {
	if (computer->ram->arrangements[_active_arrangement_index].pattern_indices[index] == -1) {
		return;
	}

	for (size_t i = 0; i < _layout.channel0_rect.h / 12; i++) {
		size_t real_index = i + _scroll_amount;

		if (real_index >= STEPS_IN_PATTERN) {
			break;
		}

		int progress = audio_get_channel_current_step(computer, index);
		if (real_index == progress) {
			rect_t progress_rect = {
				.x = base_pos.x,
				.y = base_pos.y + i * 12,
				.w = 64,
				.h = 12,
			};
			gfx_draw_filled_rect(FB_SURF(computer->ram->framebuffer.data), progress_rect, COLOR_LIGHTGRAY);
		}

		point_t pos = {
			base_pos.x + 2,
			base_pos.y + 2 + i * 12,
		};

		pattern_step_t *step = &computer->ram->patterns[computer->ram->arrangements[_active_arrangement_index].pattern_indices[index]].steps[real_index + _scroll_amount];

		gfx_draw_text(computer->ram, 1, note_to_string_map[step->pitch % 12], pos, COLOR_WHITE);

		// TODO: shared draw_note function between sound and music editor
		// Instrument
		{
			uint8_t waveform = step->instrument_index;
			pos.x += 2 * (computer->ram->fonts[1].widths[0] + computer->ram->fonts[1].horizontal_space) + computer->ram->fonts[1].horizontal_space;
			string_t string = int_to_string_formatted(get_temp_allocator(), step->instrument_index, 2, ' ');
			gfx_draw_string(computer->ram, 1, string, pos, 1 + step->instrument_index); // TODO: use new colors
		}

		// Volume
		{
			pos.x += 2 * (computer->ram->fonts[1].widths[0] + computer->ram->fonts[1].horizontal_space) + computer->ram->fonts[1].horizontal_space;
			string_t string = int_to_string_formatted(get_temp_allocator(), step->volume, 2, ' ');
			gfx_draw_string(computer->ram, 1, string, pos, 14);
		}
	}

	size_t new_active_arrangement = gui_button_matrix(computer->ram, _layout.arrangement_picker_base_pos, g_skin_layout.music_editor.arrangements_button_matrix, _active_arrangement_index);
	if (new_active_arrangement != _active_arrangement_index) { // TODO: make the default value -1 or something bc now it's annoying
		_stop_arrangement(computer);
	}
	_active_arrangement_index = new_active_arrangement;
}

void music_editor_draw(computer_t *computer) {

	// gui_button(ram_t *ram, point_t pos, button_t button, bool already_pressed)
	_draw_pattern(computer, 0, _layout.channel0_rect.pos);
	_draw_pattern(computer, 1, _layout.channel1_rect.pos);
	_draw_pattern(computer, 2, _layout.channel2_rect.pos);
	_draw_pattern(computer, 3, _layout.channel3_rect.pos);
	_draw_pattern(computer, 4, _layout.channel4_rect.pos);
	_draw_pattern(computer, 5, _layout.channel5_rect.pos);
	_draw_pattern(computer, 6, _layout.channel6_rect.pos);
	_draw_pattern(computer, 7, _layout.channel7_rect.pos);

	arrangement_t *active_arrangement = &computer->ram->arrangements[_active_arrangement_index];
	
	// Pattern number and edit button
	for (size_t i = 0; i < 8; i++) {
		if (gui_press_button(computer->ram, _layout.channel_button_positions[i], g_skin_layout.music_editor.edit_pattern_button)) {
			sound_set_current_pattern(active_arrangement->pattern_indices[i]);
			switch_to_workspace(WORKSPACE_SOUND);
		}

		point_t inc_button_pos = {_layout.channel_button_positions[i].x + g_skin_layout.music_editor.edit_pattern_button.pressed_rect.w, _layout.channel_button_positions[i].y};
		if (gui_press_button(computer->ram, inc_button_pos, g_skin_layout.music_editor.pattern_inc_button)) {
			int temp = active_arrangement->pattern_indices[i];
			temp = clamp_int(temp + 1, -1, MAX_ARRANGEMENTS - 1);
			active_arrangement->pattern_indices[i] = temp;
		}

		point_t dec_button_pos = {_layout.channel_button_positions[i].x + g_skin_layout.music_editor.edit_pattern_button.pressed_rect.w, _layout.channel_button_positions[i].y + 7};
		if (gui_press_button(computer->ram, dec_button_pos, g_skin_layout.music_editor.pattern_dec_button)) {
			int temp = active_arrangement->pattern_indices[i];
			temp = clamp_int(temp - 1, -1, MAX_ARRANGEMENTS - 1);
			active_arrangement->pattern_indices[i] = temp;
		}

		point_t text_pos = {
			_layout.channel_button_positions[i].x + 3,
			_layout.channel_button_positions[i].y + 3,
		};

		if (active_arrangement->pattern_indices[i] == -1) {
			gfx_draw_text(computer->ram, 1, "None", text_pos, COLOR_BLACK);
		} else {
			char buffer[32];
			sprintf(buffer, "#%03d\n", active_arrangement->pattern_indices[i]);
	
			gfx_draw_text(computer->ram, 1, buffer, text_pos, COLOR_BLACK);
		}
	}
}
