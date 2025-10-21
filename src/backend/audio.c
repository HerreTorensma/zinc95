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

// TODO: put on computer struct
// Then I read out the current_step and use that to render the progress beam in the sound editor
typedef struct pattern_playback_info {
	uint16_t pattern_index;
	voice_t *voice;
	int time_left_on_current_step;
	int current_step;
} pattern_playback_info_t;

static pattern_playback_info_t pattern_playback_infos[10] = {0};

int get_current_step_of_sound_editor_pattern() {
	return pattern_playback_infos[0].current_step;
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

static oscillator_t _osc1 = {
	.freq = 440.0f,
	.phase = 0.0f,
	.waveform = WAVEFORM_SINE,
};

static oscillator_t _osc2 = {
	.freq = 460.0f,
	.phase = 0.0f,
	.waveform = WAVEFORM_SQUARE,
};

void _update_patterns(computer_t *computer) {
	voice_t *voice = pattern_playback_infos[0].voice;
	if (voice == NULL) {
		return;
	}

	if (pattern_playback_infos[0].current_step == STEPS_IN_PATTERN) {
		// if (voice->oscillator.phase > 0) {

		// }
		// if (voice->oscillator.phase > 0.001 || voice->oscillator.phase < -0.001) {
		// 	return;
		// }

			voice->active = false;
			memset(&pattern_playback_infos[0], 0, sizeof(pattern_playback_info_t));
			// memset(voice, 0, sizeof(voice_t));
			// return;
		// }

		// voice->amplitude *= 0.9;

		// if (voice->amplitude < 0.001) {

			// patterns[0].voice = NULL;
		// }
		// return;

		return;

	}
	
	if (pattern_playback_infos[0].time_left_on_current_step > 0) {
		pattern_playback_infos[0].time_left_on_current_step--;
		return;
	}

	pattern_t *pattern = &get_global_computer()->ram->patterns[pattern_playback_infos[0].pattern_index];

	int octave = pattern->steps[pattern_playback_infos[0].current_step].pitch / 12 + BASE_OCTAVE;
	int freq = note_to_freq_tet12(pattern->steps[pattern_playback_infos[0].current_step].pitch, octave);

	// if (voice->oscillator.phase > 0.001 || voice->oscillator.phase < -0.001) {
	// 	return;
	// }
	// voice->amplitude -= 0.001f;
	// // voice->amplitude *= 0.999f;
	// // // voice->amplitude *= 0.95f;
	// if (voice->amplitude > 0.0001f) {
	// 	return;
	// }


	// TODO: check if the generated sample is close to zero instead (???)
	// I really have to clean up this whole system
	// if (voice->oscillator.phase < -0.01 || voice->oscillator.phase > 0.01) {
	// 	return;
	// }

	// voice->oscillator = (oscillator_t){
	// 	.freq = freq,
	// 	.phase = 0.0f,
	// 	.waveform = pattern->steps[patterns[0].current_step].waveform,
	// };
	voice->oscillator.freq = freq;
	voice->oscillator.waveform = pattern->steps[pattern_playback_infos[0].current_step].waveform;
	// voice->amplitude = 0.05f;
	voice->amplitude = 0.01f * (float)pattern->steps[pattern_playback_infos[0].current_step].volume;

	// patterns[0].time_left_on_current_step = 500;
	pattern_playback_infos[0].time_left_on_current_step = SAMPLES * pattern->speed;
	pattern_playback_infos[0].current_step++;
}

void audio_update(float *buffer, int frames) {
	// printf("audio update\n");

	voice_pool_t *pool = &get_global_computer()->voice_pool;

	
	for (int i = 0; i < frames; i++) {
		_update_patterns(get_global_computer());
		// printf("frame: %d\n", i);

		float left = 0.0f;
		float right = 0.0f;

		for (int j = 0; j < MAX_VOICES; j++) {
			voice_t *voice = &pool->voices[j];
			if (!voice->active) {
				continue;
				// // voice->amplitude *= 0.95f;
				// voice->amplitude -= 0.001f;
				// if (voice->amplitude < 0.0001f) {
				// 	memset(voice, 0, sizeof(voice_t));
				// }
			}

			sample_t sample = osc_next_sample(&voice->oscillator);
			left += sample.left * voice->amplitude;
			right += sample.right * voice->amplitude;

			// sample_t sample1 = osc_next_sample(&_osc1);
			// sample_t sample2 = osc_next_sample(&_osc2);

			// left += (sample1.left * 10.0f + sample2.left) * voice->amplitude;
			// right += (sample1.right * 10.0f + sample2.right) * voice->amplitude;
		}

		buffer[i * 2 + 0] = left;
		buffer[i * 2 + 1] = right;
	}
}

void audio_play_pattern(computer_t *computer, int pattern_index) {
	pattern_t *pattern = &computer->ram->patterns[pattern_index];
	// voice_t *voice = voice_alloc(&computer->voice_pool);
	voice_t *voice = &computer->voice_pool.voices[0];
	voice->active = true;

	pattern_playback_infos[0].current_step = 0;
	pattern_playback_infos[0].pattern_index = pattern_index;
	pattern_playback_infos[0].time_left_on_current_step = 0;
	pattern_playback_infos[0].voice = voice;
}
