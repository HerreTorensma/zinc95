#include "audio.h"

#include "sdl2.h"
#include "../computer.h"

#include <math.h>

const char *note_to_string_map[] = {
	[NOTE_C] = "C",
	[NOTE_CSHARP] = "C#",
	[NOTE_D] = "D",
	[NOTE_DSHARP] = "D#",
	[NOTE_E] = "E",
	[NOTE_F] = "F",
	[NOTE_FSHARP] = "F#",
	[NOTE_G] = "G",
	[NOTE_GSHARP] = "G#",
	[NOTE_A] = "A",
	[NOTE_ASHARP] = "A#",
	[NOTE_B] = "B",
};

voice_t *voice_alloc(voice_pool_t *pool) {
	for (size_t i = 0; i < MAX_VOICES; i++) {
		if (!pool->voices[i].active) {
			pool->voices[i].active = true;
			return &pool->voices[i];
		}
	}

	// Voices are full
	return NULL;
}

// Tet 12 music scale
float note_to_freq_tet12(note_t note, int octave) {
	int n = (octave - 4) * 12 + note;
	return 440.0f * powf(2.0f, n / 12.0f);
}

sample_t osc_next_sample(oscillator_t *osc) {
	float value = 0.0f;

	switch (osc->waveform) {
		case (WAVEFORM_SINE): {
			value = sinf(2.0f * M_PI * osc->phase);
			break;
		}

		case (WAVEFORM_SQUARE): {
			value = (osc->phase < 0.5f) ? 1.0f : -1.0f;
			break;
		}
	}

	osc->phase += osc->freq / (float)SAMPLE_RATE;
	if (osc->phase >= 1.0f) {
		osc->phase -= 1.0f;
	}

	return (sample_t){.left = value, .right = value};
}

void audio_init(computer_t *computer) {
	sdl2_audio_init(computer);
}

void audio_update(float *buffer, int frames) {
	voice_pool_t *pool = &get_global_computer()->voice_pool;

	for (int i = 0; i < frames; i++) {
		float left = 0.0f;
		float right = 0.0f;

		for (int j = 0; j < MAX_VOICES; j++) {
			voice_t *voice = &pool->voices[j];
			if (!voice->active) {
				continue;
			}

			sample_t sample = osc_next_sample(&voice->oscillator);

			left += sample.left * voice->amplitude;
			right += sample.right * voice->amplitude;
		}

		buffer[i * 2 + 0] = left;
		buffer[i * 2 + 1] = right;
	}
}