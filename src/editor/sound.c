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
} layout_t;

static const layout_t _layout = {
	.piano_pos = {100, 100},
};

#define PIANO_KEY_WIDTH 20
#define PIANO_KEY_HEIGHT 80

static voice_t *_voice_map[12] = {0};

static const zinc_key_t _note_key_map[] = {
	[NOTE_C] = KEY_S,
	[NOTE_CSHARP] = KEY_E,
	[NOTE_D] = KEY_D,
	[NOTE_DSHARP] = KEY_R,
	[NOTE_E] = KEY_F,
	[NOTE_F] = KEY_G,
	[NOTE_FSHARP] = KEY_Y,
	[NOTE_G] = KEY_H,
	[NOTE_GSHARP] = KEY_U,
	[NOTE_A] = KEY_J,
	[NOTE_ASHARP] = KEY_I,
	[NOTE_B] = KEY_K,
};

static int _current_octave = 4;

void sound_editor_init(computer_t *computer) {

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
}

void sound_editor_draw(computer_t *computer) {
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