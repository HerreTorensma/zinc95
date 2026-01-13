#include "sound.h"
#include "../backend/input.h"
#include "../backend/audio.h"

#include <stdio.h>
#include <SDL2/SDL.h>

#include "../common/math2d.h"
#include "../backend/gfx.h"
#include "../backend/input.h"
#include "../backend/gui.h"
#include "../res.h"

static struct {
	rect_t note_list_rect;
	point_t note_list_text_offset;

	rect_t pitch_graph_rect;
	point_t sine_wave_button_pos;
	point_t square_wave_button_pos;
	point_t triangle_wave_button_pos;
	point_t sawtooth_wave_button_pos;
	point_t noise_wave_button_pos;

	point_t speed_knob_center;
	int speed_knob_radius;
	point_t speed_knob_text_pos;
	
	rect_t volume_graph_rect;

	point_t pattern_picker_pos;
}
_layout = {
	.note_list_rect = {{124, 24 , 512, 96}},
	.note_list_text_offset = {2, 2},

	.pitch_graph_rect = {{123, 124, 512, 96}},
	
	.sine_wave_button_pos = {478, 274},
	.square_wave_button_pos = {510, 274},
	.triangle_wave_button_pos = {542, 274},
	.sawtooth_wave_button_pos = {574, 274},
	.noise_wave_button_pos = {606, 274},

	.speed_knob_center = {130, 283},
	.speed_knob_radius = 8,
	.speed_knob_text_pos = {140, 279},
	
	.volume_graph_rect = {{123, 224, 512, 48}},

	.pattern_picker_pos = {508, 412},
};

static int _current_pattern = 0;
static int _selected_waveform = WAVEFORM_SINE;

static gui_knob_state_t _speed_knob_state = {0};

// This is a function because later there will be more knobs and then I need to loop something
static bool _is_any_knob_held() {
	return (_speed_knob_state.held);
}

void sound_editor_init(computer_t *computer) {

}

void sound_editor_update(computer_t *computer) {
	if (input_key_pressed(KEY_SPACE)) {
		audio_play_pattern(computer, _current_pattern);
	}
	
	if (_is_any_knob_held()) {
		return;
	}

	point_t mouse_pos = input_get_mouse_pos();
	point_t adjusted_mouse_pos = POINT(mouse_pos.x - _layout.pitch_graph_rect.x, mouse_pos.y - _layout.pitch_graph_rect.y);

	if (point_in_rect(mouse_pos, _layout.pitch_graph_rect)) {
		int step = adjusted_mouse_pos.x / (_layout.pitch_graph_rect.w / STEPS_IN_PATTERN);

		int pitch = computer->ram->patterns[_current_pattern].steps[step].pitch;

		if (input_mouse_scrolled(SCROLL_DIR_DOWN)) {
			pitch--;
		}
		if (input_mouse_scrolled(SCROLL_DIR_UP)) {
			pitch++;
		}

		pitch = clamp_int(pitch, 0, MAX_PITCH);

		computer->ram->patterns[_current_pattern].steps[step].pitch = pitch;
	}

	if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
		if (point_in_rect(mouse_pos, _layout.pitch_graph_rect)) {
			int step = adjusted_mouse_pos.x / (_layout.pitch_graph_rect.w / STEPS_IN_PATTERN);
			// int pitch = 47 - (mouse_pos.y - _layout.pitch_graph_rect.y) / 2;
			int pitch = MAX_PITCH-1 - adjusted_mouse_pos.y / (_layout.pitch_graph_rect.h / MAX_PITCH);
			// printf("step: %d, pitch: %d\n", step, pitch);
			computer->ram->patterns[_current_pattern].steps[step].pitch = pitch;
			computer->ram->patterns[_current_pattern].steps[step].waveform = _selected_waveform;
		}
	}

	if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
		if (point_in_rect(mouse_pos, _layout.volume_graph_rect)) {
			point_t adjusted_mouse_pos = POINT(mouse_pos.x - _layout.volume_graph_rect.x, mouse_pos.y - _layout.volume_graph_rect.y);

			int step = adjusted_mouse_pos.x / (_layout.volume_graph_rect.w / STEPS_IN_PATTERN);
			int volume = MAX_VOLUME-1 - adjusted_mouse_pos.y / (_layout.volume_graph_rect.h / MAX_VOLUME);

			computer->ram->patterns[_current_pattern].steps[step].volume = volume;
		}
	}
}

void sound_editor_draw(computer_t *computer) {
	// Notes
	for (size_t i = 0; i < STEPS_IN_PATTERN; i++) {
		point_t pos = {
			.x = _layout.note_list_rect.x + (i / 8) * 64,
			.y = _layout.note_list_rect.y + (i % 8) * 12,
		};
		int progress = get_current_step_of_sound_editor_pattern();
		if (progress > 0 && i == progress) {
			gfx_draw_filled_rect(FB_SURF(computer->ram->framebuffer.data), RECT(pos.x, pos.y, 64, 12), COLOR_LIGHTGRAY);
		}

		pos.x += _layout.note_list_text_offset.x;
		pos.y += _layout.note_list_text_offset.y;

		pattern_step_t *step = &computer->ram->patterns[_current_pattern].steps[i];

		gui_draw_text(computer->ram, 1, note_to_string_map[step->pitch % 12], pos, COLOR_WHITE);

		// Instrument
		{
			uint8_t waveform = computer->ram->patterns[_current_pattern].steps[i].waveform;
			pos.x += 2 * (computer->ram->fonts[1].widths[0] + computer->ram->fonts[1].horizontal_space) + computer->ram->fonts[1].horizontal_space;
			string_t string = int_to_string(get_temp_allocator(), step->waveform);
			gui_draw_string(computer->ram, 1, string, pos, 9 + step->waveform);
		}

		// Volume
		{
			pos.x += 2 * (computer->ram->fonts[1].widths[0] + computer->ram->fonts[1].horizontal_space) + computer->ram->fonts[1].horizontal_space;
			string_t string = int_to_string(get_temp_allocator(), step->volume);
			gui_draw_string(computer->ram, 1, string, pos, 14);
		}
	}
	
	// Pitch graph
	for (size_t i = 0; i < STEPS_IN_PATTERN; i++) {
		gfx_draw_filled_rect(
			FB_SURF(computer->ram->framebuffer.data),
			RECT(
				_layout.pitch_graph_rect.x + i * (_layout.pitch_graph_rect.w / STEPS_IN_PATTERN) + 1,
				_layout.pitch_graph_rect.y + _layout.pitch_graph_rect.h - (computer->ram->patterns[_current_pattern].steps[i].pitch * (_layout.pitch_graph_rect.h / MAX_PITCH)) - 3,
				(_layout.pitch_graph_rect.w / STEPS_IN_PATTERN) - 1,
				3
			),
			9 + computer->ram->patterns[_current_pattern].steps[i].waveform
		);

		// The thing underneath
		{
			int y = _layout.pitch_graph_rect.y + _layout.pitch_graph_rect.h - (computer->ram->patterns[_current_pattern].steps[i].pitch * (_layout.pitch_graph_rect.h / MAX_PITCH)) + 1;
	
			gfx_draw_filled_rect(
				FB_SURF(computer->ram->framebuffer.data),
				RECT(
					_layout.pitch_graph_rect.x + i * (_layout.pitch_graph_rect.w / STEPS_IN_PATTERN) + 1,
					y,
					(_layout.pitch_graph_rect.w / STEPS_IN_PATTERN) - 1,
					_layout.pitch_graph_rect.y + _layout.pitch_graph_rect.h - y
				),
				COLOR_DARKGRAY
			);
		}
	}

	// Progress beam
	int progress = get_current_step_of_sound_editor_pattern();
	if (progress > 0) {
		gfx_draw_line(
			FB_SURF(computer->ram->framebuffer.data),
			POINT(_layout.pitch_graph_rect.x + progress * (_layout.pitch_graph_rect.w / STEPS_IN_PATTERN), _layout.pitch_graph_rect.y),
			POINT(_layout.pitch_graph_rect.x + progress * (_layout.pitch_graph_rect.w / STEPS_IN_PATTERN), _layout.pitch_graph_rect.y + _layout.pitch_graph_rect.h),
			COLOR_WHITE
		);
	}

	// Volume points
	for (size_t i = 0; i < STEPS_IN_PATTERN; i++) {
		gfx_draw_filled_rect(
			FB_SURF(computer->ram->framebuffer.data),
			RECT(
				_layout.volume_graph_rect.x + i * (_layout.volume_graph_rect.w / STEPS_IN_PATTERN) + 1,
				// _layout.volume_graph_rect.y + _layout.volume_graph_rect.h - (computer->ram->patterns[_current_pattern].steps[i].volume * (_layout.volume_graph_rect.h / MAX_VOLUME)) - 3,
				_layout.volume_graph_rect.y + _layout.volume_graph_rect.h - (computer->ram->patterns[_current_pattern].steps[i].volume * (_layout.volume_graph_rect.h / MAX_VOLUME)) - 3,
				// 7,
				(_layout.volume_graph_rect.w / STEPS_IN_PATTERN) - 1,
				3
			),
			14 // TODO: change shade based on volume?
		);
	}

	computer->ram->patterns[_current_pattern].speed = gui_knob(computer->ram, 0, _layout.speed_knob_center, _layout.speed_knob_radius, _layout.speed_knob_text_pos, MIN_PATTERN_SPEED, MAX_PATTERN_SPEED, computer->ram->patterns[_current_pattern].speed, &_speed_knob_state);

	if (gui_button(computer->ram, _layout.sine_wave_button_pos, g_skin_layout.sine_wave_button, _selected_waveform == WAVEFORM_SINE)) {
		_selected_waveform = WAVEFORM_SINE;
	}
	if (gui_button(computer->ram, _layout.square_wave_button_pos, g_skin_layout.square_wave_button, _selected_waveform == WAVEFORM_SQUARE)) {
		_selected_waveform = WAVEFORM_SQUARE;
	}
	if (gui_button(computer->ram, _layout.triangle_wave_button_pos, g_skin_layout.triangle_wave_button, _selected_waveform == WAVEFORM_TRIANGLE)) {
		_selected_waveform = WAVEFORM_TRIANGLE;
	}
	if (gui_button(computer->ram, _layout.sawtooth_wave_button_pos, g_skin_layout.sawtooth_wave_button, _selected_waveform == WAVEFORM_SAWTOOTH)) {
		_selected_waveform = WAVEFORM_SAWTOOTH;
	}
	if (gui_button(computer->ram, _layout.noise_wave_button_pos, g_skin_layout.noise_wave_button, _selected_waveform == WAVEFORM_NOISE)) {
		_selected_waveform = WAVEFORM_NOISE;
	}

	_current_pattern = gui_button_matrix(computer->ram, _layout.pattern_picker_pos, g_skin_layout.sfx_picker_buttons, _current_pattern);
}