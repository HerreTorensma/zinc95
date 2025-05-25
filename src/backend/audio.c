#include "audio.h"

#include "sdl2.h"

#include <math.h>

float note_to_freq(note_t note, int octave) {

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

static oscillator_t osc = {
	.freq = 500.0f,
	.phase = 0.0f,
	.waveform = WAVEFORM_SINE,
};

void audio_update(float *buffer, int frames) {
	// osc.freq += 1.0f;
	for (int i = 0; i < frames; i++) {
		sample_t sample = osc_next_sample(&osc);
		
		buffer[i * 2 + 0] = sample.left;
		buffer[i * 2 + 1] = sample.right;
	}
}