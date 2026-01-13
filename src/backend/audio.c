#include "audio.h"

#include "sdl2.h"

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

static channel_t channels[12] = {0};

int get_current_step_of_sound_editor_pattern() {
	return channels[0].current_step;
}

voice_t *voice_alloc(voice_pool_t *pool) {
	for (size_t i = 0; i < MAX_VOICES; i++) {
		if (!pool->voices[i].active) {
			pool->voices[i].active = true;
			return &pool->voices[i];
		}
	}

	// Voices are full, so no sound will be played
	return NULL;
}

// Tet 12 music scale
float note_to_freq_tet12(note_t note, int octave) {
	int n = (octave - 4) * 12 + note;
	return 440.0f * powf(2.0f, n / 12.0f);
}

sample_t osc_next_sample(oscillator_t *osc) {
	osc->phase += osc->freq / (float)SAMPLE_RATE;
	if (osc->phase >= 1.0f) {
		osc->phase -= 1.0f;
	}

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

		case WAVEFORM_TRIANGLE: {
			value = 2.0f * fabsf(2.0f * osc->phase - 1.0f) - 1.0f;
			break;
		}

		case WAVEFORM_SAWTOOTH: {
			value = osc->phase;
			break;
		}

		case WAVEFORM_NOISE: {
			static uint16_t lfsr = 0xACE1;
			uint16_t bit = ((lfsr >> 0) ^ (lfsr >> 1)) & 1;
			lfsr = (lfsr >> 1) | (bit << 14);
			float thing = lfsr & 1;

			value = thing ? 1.0f : -1.0f;

			printf("value: %f\n", value);
			break;
		}
	}

	return (sample_t){.left = value, .right = value};
}

void audio_init(computer_t *computer) {
	sdl2_audio_init(computer);
}

void _update_channels(computer_t *computer) {
	voice_t *voice = channels[0].voice;
	if (voice == NULL) {
		return;
	}

	if (channels[0].current_step == STEPS_IN_PATTERN) {
		voice->active = false;
		memset(&channels[0], 0, sizeof(channel_t));
		
		return;
	}
	
	if (channels[0].time_left_on_current_step > 0) {
		channels[0].time_left_on_current_step--;
		return;
	}

	pattern_t *pattern = &get_global_computer()->ram->patterns[channels[0].pattern_index];

	int octave = pattern->steps[channels[0].current_step].pitch / 12 + BASE_OCTAVE;
	int freq = note_to_freq_tet12(pattern->steps[channels[0].current_step].pitch, octave);

	voice->oscillator.freq = freq;
	voice->oscillator.waveform = pattern->steps[channels[0].current_step].waveform;
	voice->amplitude = 0.01f * (float)pattern->steps[channels[0].current_step].volume;

	channels[0].time_left_on_current_step = SAMPLES * pattern->speed;
	channels[0].current_step++;
}

void audio_update(float *buffer, int frames) {
	voice_pool_t *pool = &get_global_computer()->voice_pool;

	for (int i = 0; i < frames; i++) {
		_update_channels(get_global_computer());

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

void audio_play_pattern(computer_t *computer, int pattern_index) {
	pattern_t *pattern = &computer->ram->patterns[pattern_index];
	voice_t *voice = &computer->voice_pool.voices[0];
	voice->active = true;

	channels[0].current_step = 0;
	channels[0].pattern_index = pattern_index;
	channels[0].time_left_on_current_step = 0;
	channels[0].voice = voice;
}
