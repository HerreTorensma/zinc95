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

	// TODO: put in global layout, like make a knob_t struct
	point_t speed_knob_center; // TODO: rename to delay_knob_center
	point_t volume_knob_center;
	// int speed_knob_radius;
	// point_t speed_knob_text_pos;
	
	rect_t volume_graph_rect;

	point_t pattern_picker1_pos;
	point_t pattern_picker2_pos;
	point_t pattern_picker3_pos;
	point_t pattern_picker4_pos;

	point_t instrument_button_pos;

	point_t attack_knob_center;
	point_t decay_knob_center;
	point_t sustain_knob_center;
	point_t release_knob_center;
	// int attack_knob_radius;
	// point_t attack_knob_text_pos;

	rect_t oscilloscope_rect;
	rect_t instrument_wave_oscilloscope;
}
_layout = {
	.note_list_rect = {{124, 24 , 512, 96}},
	.note_list_text_offset = {2, 2},

	.pitch_graph_rect = {{123, 124, 512, 96}},
	
	.sine_wave_button_pos = {200, 280},
	.square_wave_button_pos = {232, 280},
	.triangle_wave_button_pos = {264, 280},
	.sawtooth_wave_button_pos = {296, 280},
	.noise_wave_button_pos = {328, 280},

	.speed_knob_center = {14, 296},
	.volume_knob_center = {14, 326},
	// .speed_knob_radius = 8,
	// .speed_knob_text_pos = {140, 279},
	
	.volume_graph_rect = {{123, 224, 512, 48}},

	.pattern_picker1_pos = {6, 26},
	.pattern_picker2_pos = {6, 88},
	.pattern_picker3_pos = {6, 150},
	.pattern_picker4_pos = {6, 212},

	.instrument_button_pos = {132, 280},

	// .attack_knob_center = {200, 283},
	// .decay_knob_center = {241, 283},
	// .sustain_knob_center = {282, 283},
	// .release_knob_center = {323, 283},
	.attack_knob_center = {208, 365},
	.decay_knob_center = {208, 394},
	.sustain_knob_center = {208, 424},
	.release_knob_center = {208, 453},
	// .attack_knob_radius = 8,
	// .attack_knob_text_pos = {210, 279},

	.oscilloscope_rect = {{4, 428, 116, 48}},
	.instrument_wave_oscilloscope = {{570, 278+3, 64, 25}},
};

static int _current_pattern = 0;
// static int _selected_waveform = WAVEFORM_SINE;

static int _current_instrument = 0;

static gui_knob_state_t _speed_knob_state = {0};
static gui_knob_state_t _volume_knob_state = {0};
static gui_knob_state_t _attack_knob_state = {0};
static gui_knob_state_t _decay_knob_state = {0};
static gui_knob_state_t _sustain_knob_state = {0};
static gui_knob_state_t _release_knob_state = {0};

static size_t _active_sound_effect_channel_index = 0;

// Only used for wave visualization
static channel_t _fake_channel = {
	.frequency = (int)((1.0f/(float)64) * SAMPLE_RATE)
};

static knob_t _universal_knob = {
	.radius = 8,
	.text_offset = {10, -4},
};

// This is a function because later there will be more knobs and then I need to loop something
static bool _is_any_knob_held() {
	return _speed_knob_state.held || _volume_knob_state.held || _attack_knob_state.held || _decay_knob_state.held || _sustain_knob_state.held || _release_knob_state.held;
}

void sound_set_current_pattern(size_t index) {
	_current_pattern = index;
}

void sound_editor_init(computer_t *computer) {

}

void sound_editor_update(computer_t *computer) {
	if (input_key_pressed(KEY_SPACE)) {
		if (audio_get_channel_current_step(computer, 0) > 0) { // TODO: make the default value -1 or something bc now it's annoying
			audio_cancel_channel(computer, 0);
		} else {
			_active_sound_effect_channel_index = audio_play_pattern(computer, _current_pattern, 0);
		}
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

	// TODO: scroll wheel control on volume and notes

	if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
		if (point_in_rect(mouse_pos, _layout.pitch_graph_rect)) {
			int step = adjusted_mouse_pos.x / (_layout.pitch_graph_rect.w / STEPS_IN_PATTERN);
			// int pitch = 47 - (mouse_pos.y - _layout.pitch_graph_rect.y) / 2;
			int pitch = MAX_PITCH-1 - adjusted_mouse_pos.y / (_layout.pitch_graph_rect.h / MAX_PITCH);
			// printf("step: %d, pitch: %d\n", step, pitch);
			computer->ram->patterns[_current_pattern].steps[step].pitch = pitch;
			computer->ram->patterns[_current_pattern].steps[step].instrument_index = _current_instrument;
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
	// TODO: highlight cell under mouse
	for (size_t i = 0; i < STEPS_IN_PATTERN; i++) {
		point_t pos = {
			.x = _layout.note_list_rect.x + (i / 8) * 64,
			.y = _layout.note_list_rect.y + (i % 8) * 12,
		};

		int progress = audio_get_channel_current_step(computer, _active_sound_effect_channel_index);
		if (progress > 0 && i == progress) {
			gfx_draw_filled_rect(FB_SURF(computer->ram->framebuffer.data), RECT(pos.x, pos.y, 64, 12), COLOR_LIGHTGRAY);
		}

		pos.x += _layout.note_list_text_offset.x;
		pos.y += _layout.note_list_text_offset.y;

		pattern_step_t *step = &computer->ram->patterns[_current_pattern].steps[i];

		gui_draw_text(computer->ram, 1, note_to_string_map[step->pitch % 12], pos, COLOR_WHITE);

		// Instrument
		{
			uint8_t waveform = computer->ram->patterns[_current_pattern].steps[i].instrument_index;
			pos.x += 2 * (computer->ram->fonts[1].widths[0] + computer->ram->fonts[1].horizontal_space) + computer->ram->fonts[1].horizontal_space;
			string_t string = int_to_string_formatted(get_temp_allocator(), step->instrument_index, 2, ' ');
			gui_draw_string(computer->ram, 1, string, pos, 1 + step->instrument_index); // TODO: use new colors
		}

		// Volume
		{
			pos.x += 2 * (computer->ram->fonts[1].widths[0] + computer->ram->fonts[1].horizontal_space) + computer->ram->fonts[1].horizontal_space;
			string_t string = int_to_string_formatted(get_temp_allocator(), step->volume, 2, ' ');
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
			// 9 + computer->ram->patterns[_current_pattern].steps[i].instrument_index
			1 + computer->ram->patterns[_current_pattern].steps[i].instrument_index
		);

		// The thing underneath
		{
			int y = _layout.pitch_graph_rect.y + _layout.pitch_graph_rect.h - (computer->ram->patterns[_current_pattern].steps[i].pitch * (_layout.pitch_graph_rect.h / MAX_PITCH)) + 1;
	
			color_t color = COLOR_DARKGRAY;
			if (i == audio_get_channel_current_step(computer, _active_sound_effect_channel_index)) {
				color = COLOR_LIGHTGRAY;
			}

			gfx_draw_filled_rect(
				FB_SURF(computer->ram->framebuffer.data),
				RECT(
					_layout.pitch_graph_rect.x + i * (_layout.pitch_graph_rect.w / STEPS_IN_PATTERN) + 1,
					y,
					(_layout.pitch_graph_rect.w / STEPS_IN_PATTERN) - 1,
					_layout.pitch_graph_rect.y + _layout.pitch_graph_rect.h - y
				),
				color
			);
		}
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

	// Delay
	computer->ram->patterns[_current_pattern].speed = gui_knob(
		computer->ram,
		0,
		_layout.speed_knob_center,
		_universal_knob,
		MIN_PATTERN_SPEED, MAX_PATTERN_SPEED,
		computer->ram->patterns[_current_pattern].speed,
		&_speed_knob_state
	);

	// Volume
	computer->ram->patterns[_current_pattern].volume = gui_knob(
		computer->ram,
		0,
		_layout.volume_knob_center,
		_universal_knob,
		0, 255,
		computer->ram->patterns[_current_pattern].volume,
		&_volume_knob_state
	);

	waveform_t *waveform = &computer->ram->instruments[_current_instrument].waveform;
	if (gui_button(computer->ram, _layout.sine_wave_button_pos, g_skin_layout.sine_wave_button, *waveform == WAVEFORM_SINE)) {
		*waveform = WAVEFORM_SINE;
	}
	if (gui_button(computer->ram, _layout.square_wave_button_pos, g_skin_layout.square_wave_button, *waveform == WAVEFORM_SQUARE)) {
		*waveform = WAVEFORM_SQUARE;
	}
	if (gui_button(computer->ram, _layout.triangle_wave_button_pos, g_skin_layout.triangle_wave_button, *waveform == WAVEFORM_TRIANGLE)) {
		*waveform = WAVEFORM_TRIANGLE;
	}
	if (gui_button(computer->ram, _layout.sawtooth_wave_button_pos, g_skin_layout.sawtooth_wave_button, *waveform == WAVEFORM_SAWTOOTH)) {
		*waveform = WAVEFORM_SAWTOOTH;
	}
	if (gui_button(computer->ram, _layout.noise_wave_button_pos, g_skin_layout.noise_wave_button, *waveform == WAVEFORM_NOISE)) {
		*waveform = WAVEFORM_NOISE;
	}

	// Instrument oscilloscope
	// TODO: maybe make a seperate one for left and right (stereo)
	{
		_fake_channel.phase = 0.0f;
		_fake_channel.instrument_index = _current_instrument;
		for (size_t i = 0; i < _layout.instrument_wave_oscilloscope.w; i++) {
			sample_t sample = synth_sample(computer, &_fake_channel);

			int x = _layout.instrument_wave_oscilloscope.x + i;
			int y = _layout.instrument_wave_oscilloscope.y + (_layout.instrument_wave_oscilloscope.h / 2) + (int)(sample.left / 2 * _layout.instrument_wave_oscilloscope.h);

			// TODO: make color part of skin
			gfx_set_pixel(&computer->ram->framebuffer, x, y, 53);
		}
	}
	
	// Instruments
	{
		for (size_t i = 0; i < MAX_INSTRUMENTS; i++) {
			point_t pos = _layout.instrument_button_pos;
			pos.y += i * g_skin_layout.instrument_button.unpressed_rect.h;
			if (gui_button(computer->ram, pos, g_skin_layout.instrument_button, i == _current_instrument)) {
				_current_instrument = i;
			}

			gfx_draw_filled_rect(FB_SURF(computer->ram->framebuffer.data), RECT(pos.x + 4, pos.y + 4, 4, 4), 1 + i);
			// gfx_set_pixel(&computer->ram->framebuffer, pos.x + 6, pos.y + 5, COLOR_WHITE);
		}
	}
	
	// ADSR
	computer->ram->instruments[_current_instrument].attack = gui_knob(
		computer->ram,
		0,
		_layout.attack_knob_center,
		_universal_knob,
		0, 255,
		computer->ram->instruments[_current_instrument].attack,
		&_attack_knob_state
	);

	computer->ram->instruments[_current_instrument].decay = gui_knob(
		computer->ram,
		0,
		_layout.decay_knob_center,
		_universal_knob,
		0, 255,
		computer->ram->instruments[_current_instrument].decay,
		&_decay_knob_state
	);

	computer->ram->instruments[_current_instrument].sustain = gui_knob(
		computer->ram,
		0,
		_layout.sustain_knob_center,
		_universal_knob,
		0, 255,
		computer->ram->instruments[_current_instrument].sustain,
		&_sustain_knob_state
	);

	computer->ram->instruments[_current_instrument].release = gui_knob(
		computer->ram,
		0,
		_layout.release_knob_center,
		_universal_knob,              
		0, 255,
		computer->ram->instruments[_current_instrument].release,
		&_release_knob_state
	);

	// size_t new_current_pattern = gui_button_matrix(computer->ram, _layout.pattern_picker1_pos, g_skin_layout.sfx_picker_buttons, _current_pattern);
	// new_current_pattern = 98 + gui_button_matrix(computer->ram, _layout.pattern_picker2_pos, g_skin_layout.sfx_picker_buttons, 98 + _current_pattern);
	// new_current_pattern = 98 * 2 + gui_button_matrix(computer->ram, _layout.pattern_picker3_pos, g_skin_layout.sfx_picker_buttons, 98 * 2 + _current_pattern);
	// new_current_pattern = 98 * 3 + gui_button_matrix(computer->ram, _layout.pattern_picker4_pos, g_skin_layout.sfx_picker_buttons, 98 * 3 + _current_pattern);
	
	// size_t new_current_pattern = gui_button_matrix(computer->ram, _layout.pattern_picker1_pos, g_skin_layout.sfx_picker_buttons, _current_pattern, 0);
	size_t new_current_pattern = gui_button_matrix(computer->ram, _layout.pattern_picker1_pos, g_skin_layout.sfx_picker_buttons, _current_pattern);
	
	// new_current_pattern = gui_button_matrix(computer->ram, _layout.pattern_picker2_pos, g_skin_layout.sfx_picker_buttons, _current_pattern % 98, 98);
	// new_current_pattern = 98 * 2 + gui_button_matrix(computer->ram, _layout.pattern_picker3_pos, g_skin_layout.sfx_picker_buttons, _current_pattern % 98);
	// new_current_pattern = 98 * 3 + gui_button_matrix(computer->ram, _layout.pattern_picker4_pos, g_skin_layout.sfx_picker_buttons, _current_pattern % 98);
	if (_current_pattern != new_current_pattern) {
		audio_cancel_channel(computer, 0);
	}
	_current_pattern = new_current_pattern;

	// Pattern
	char buffer[32];
	sprintf(buffer, "#%03d\n", _current_pattern);
	gui_draw_text(computer->ram, 1, buffer, (point_t){6, 272}, COLOR_BLACK);

	// Oscilloscope
	{
		float *stream = audio_get_stream();

		float inc = 1.0f / (float)_layout.oscilloscope_rect.w;

		for (size_t i = 0; i < _layout.oscilloscope_rect.w; i++) {
			int x = _layout.oscilloscope_rect.x + i;
			int y = _layout.oscilloscope_rect.y + (_layout.oscilloscope_rect.h / 2) + (int)(stream[i] * _layout.oscilloscope_rect.h);

			// TODO: make color part of skin
			gfx_set_pixel(&computer->ram->framebuffer, x, y, 53);
		}
	}
}
