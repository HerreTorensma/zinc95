#include "sound.h"
#include "menu.h"
#include "../backend/input.h"
#include "../backend/audio.h"

#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#include "../common/math2d.h"
#include "../backend/gfx.h"
#include "../backend/input.h"
#include "../backend/gui.h"

typedef struct layout {
	point_t piano_pos;
	rect_t pitch_graph_rect;
	point_t sine_wave_button_pos;
	point_t square_wave_button_pos;
	point_t triangle_wave_button_pos;
	point_t sawtooth_wave_button_pos;
} layout_t;

static const layout_t _layout = {
	.piano_pos = {300, 100},
	.pitch_graph_rect = {{4, 24, 256, 96}},
	.sine_wave_button_pos = {300, 24},
	.square_wave_button_pos = {332, 24},
	.triangle_wave_button_pos = {364, 24},
	.sawtooth_wave_button_pos = {396, 24},
};

#define PIANO_KEY_WIDTH 20
#define PIANO_KEY_HEIGHT 80

static voice_t *_voice_map[12] = {0};

static const zinc_key_t _note_key_map[] = {
	[NOTE_C] = KEY_Z,
	[NOTE_CSHARP] = KEY_S,
	[NOTE_D] = KEY_X,
	[NOTE_DSHARP] = KEY_D,
	[NOTE_E] = KEY_C,
	[NOTE_F] = KEY_V,
	[NOTE_FSHARP] = KEY_G,
	[NOTE_G] = KEY_B,
	[NOTE_GSHARP] = KEY_H,
	[NOTE_A] = KEY_N,
	[NOTE_ASHARP] = KEY_J,
	[NOTE_B] = KEY_M,
};

static int _current_octave = 4;

static int _current_pattern = 0;
static int _selected_waveform = WAVEFORM_SINE;

void sound_editor_init(computer_t *computer) {
	// Initialize the first pattern
	for (size_t i = 0; i < STEPS_IN_PATTERN; i++) {
		computer->ram->patterns[_current_pattern].speed = 1;
		computer->ram->patterns[_current_pattern].steps[i].pitch = 0;
		computer->ram->patterns[_current_pattern].steps[i].volume = 0;
		computer->ram->patterns[_current_pattern].steps[i].waveform = WAVEFORM_SINE;
	}
}

void sound_editor_update(computer_t *computer) {
	if (input_key_pressed(KEY_UP)) {
		_current_octave++;
	}
	if (input_key_pressed(KEY_DOWN)) {
		_current_octave--;
	}

	for (int i = 0; i < 12; i++) {
		if (input_key_pressed(_note_key_map[i])) {
			voice_t *voice = voice_alloc(&computer->voice_pool);
			voice->oscillator = (oscillator_t){
				.freq = note_to_freq_tet12(i, _current_octave),
				.phase = 0.0f,
				.waveform = WAVEFORM_SINE,
			};
			voice->amplitude = 0.5f;

			_voice_map[i] = voice;
		}
	}

	for (int i = 0; i < 12; i++) {
		if (input_key_released(_note_key_map[i])) {
			_voice_map[i]->active = false;
			_voice_map[i] = NULL;
		}
	}

	point_t mouse_pos = input_get_mouse_pos();

	if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
		if (point_in_rect(mouse_pos, _layout.pitch_graph_rect)) {
			int step = (mouse_pos.x - _layout.pitch_graph_rect.x) / 8;
			int pitch = 47 - (mouse_pos.y - _layout.pitch_graph_rect.y) / 2;
			// printf("step: %d, pitch: %d\n", step, pitch);
			computer->ram->patterns[_current_pattern].steps[step].pitch = pitch;
			computer->ram->patterns[_current_pattern].steps[step].waveform = _selected_waveform;
		}
	}

	if (input_key_pressed(KEY_SPACE)) {
		audio_play_pattern(computer, _current_pattern);
	}
}

void sound_editor_draw(computer_t *computer) {
	// memset(computer->ram->patterns, 0, PATTERN_AMOUNT * sizeof(pattern_t));

	// Pitch graph
	gfx_draw_filled_rect(FB_SURF(computer->ram->framebuffer.data), _layout.pitch_graph_rect, COLOR_BLACK);

	for (size_t i = 0; i < STEPS_IN_PATTERN; i++) {
		gfx_draw_line(FB_SURF(computer->ram->framebuffer.data), POINT(_layout.pitch_graph_rect.x + i * 8, _layout.pitch_graph_rect.y), POINT(_layout.pitch_graph_rect.x + i * 8, _layout.pitch_graph_rect.y + _layout.pitch_graph_rect.h - 1), COLOR_DARKGRAY);
	}
	

	for (size_t i = 0; i < STEPS_IN_PATTERN; i++) {
		// gfx_draw_line(FB_SURF(computer->ram->framebuffer.data), POINT(i * 8, ), POINT_T, color_t color)
		// gfx_draw_filled_rect(FB_SURF(computer->ram->framebuffer.data), RECT(_layout.pitch_graph_rect.x + i * 8 + 1, _layout.pitch_graph_rect.y + _layout.pitch_graph_rect.h - (computer->ram->patterns[_current_pattern].steps[i].pitch * 2) - 3, 7, 3), COLOR_DARKGREEN);
		gfx_draw_filled_rect(FB_SURF(computer->ram->framebuffer.data), RECT(_layout.pitch_graph_rect.x + i * 8 + 1, _layout.pitch_graph_rect.y + _layout.pitch_graph_rect.h - (computer->ram->patterns[_current_pattern].steps[i].pitch * 2) - 3, 7, 3), COLOR_DARKRED + computer->ram->patterns[_current_pattern].steps[i].waveform);
	}

	computer->ram->patterns[_current_pattern].speed = gui_slider(computer->ram, 0, RECT(100, 200, 40, 40), 1, 16, computer->ram->patterns[_current_pattern].speed);

	if (gui_button(computer->ram, _layout.sine_wave_button_pos, skin_layout.sine_wave_button, _selected_waveform == WAVEFORM_SINE)) {
		_selected_waveform = WAVEFORM_SINE;
	}
	if (gui_button(computer->ram, _layout.square_wave_button_pos, skin_layout.square_wave_button, _selected_waveform == WAVEFORM_SQUARE)) {
		_selected_waveform = WAVEFORM_SQUARE;
	}
	if (gui_button(computer->ram, _layout.triangle_wave_button_pos, skin_layout.triangle_wave_button, _selected_waveform == WAVEFORM_TRIANGLE)) {
		_selected_waveform = WAVEFORM_TRIANGLE;
	}
	if (gui_button(computer->ram, _layout.sawtooth_wave_button_pos, skin_layout.sawtooth_wave_button, _selected_waveform == WAVEFORM_SAWTOOTH)) {
		_selected_waveform = WAVEFORM_SAWTOOTH;
	}

	// Draw a piano
	for (int i = 0; i < 12; i++) {
		color_t color = COLOR_WHITE;
		if (_voice_map[i] != NULL && _voice_map[i]->active) {
			color = 2;
		}

		gfx_draw_filled_rect(FB_SURF(computer->ram->framebuffer.data), RECT(_layout.piano_pos.x + i * PIANO_KEY_WIDTH, _layout.piano_pos.y, PIANO_KEY_WIDTH, PIANO_KEY_HEIGHT), color);
		gfx_draw_rect(FB_SURF(computer->ram->framebuffer.data), RECT(_layout.piano_pos.x + i * PIANO_KEY_WIDTH, _layout.piano_pos.y, PIANO_KEY_WIDTH, PIANO_KEY_HEIGHT), COLOR_BLACK);
		gui_draw_text(computer->ram, 0, note_to_string_map[i], POINT(2 + _layout.piano_pos.x + i * PIANO_KEY_WIDTH, 2 + _layout.piano_pos.y), COLOR_BLACK);
	}
}