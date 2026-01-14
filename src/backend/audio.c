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

int audio_get_channel_current_step(computer_t *computer, int channel_index) {
	return computer->channels[channel_index].current_step_index;
}

void audio_cancel_channel(computer_t *computer, int channel_index) {
	memset(&computer->channels[channel_index], 0, sizeof(channel_t));
}

size_t channel_alloc(computer_t *computer) {
	for (size_t i = 0; i < MAX_CHANNELS; i++) {
		if (computer->channels[i].active) {
			continue;
		}
		return i;
	}
	return 0; // Take channel 0 if all are taken
}

static float _note_to_freq(note_t note, int octave) {
	int midi_note = (octave + 1) * 12 + note;
	return 440.0f * powf(2.0f, (midi_note - 69) / 12.0f);
}

sample_t synth_sample(computer_t *computer, channel_t *channel) {
	channel->phase += channel->frequency / (float)SAMPLE_RATE;
	if (channel->phase >= 1.0f) {
		channel->phase -= 1.0f;
	}

	float value = 0.0f;

	instrument_t *instrument = &computer->ram->instruments[channel->instrument_index];

	// TODO: make proper
	// switch (instrument->waveform) {
	switch (channel->instrument_index) {
		case (WAVEFORM_SINE): {
			value = sinf(2.0f * M_PI * channel->phase);
			break;
		}

		case (WAVEFORM_SQUARE): {
			value = (channel->phase < 0.5f) ? 1.0f : -1.0f;
			break;
		}

		case WAVEFORM_TRIANGLE: {
			value = 2.0f * fabsf(2.0f * channel->phase - 1.0f) - 1.0f;
			break;
		}

		case WAVEFORM_SAWTOOTH: {
			value = channel->phase;
			break;
		}

		case WAVEFORM_NOISE: {
			static uint16_t lfsr = 0xACE1;
			uint16_t bit = ((lfsr >> 0) ^ (lfsr >> 1)) & 1;
			lfsr = (lfsr >> 1) | (bit << 14);
			float thing = lfsr & 1;

			value = thing ? 1.0f : -1.0f;

			break;
		}
	}

	return (sample_t){.left = value, .right = value};
}

void audio_init(computer_t *computer) {
	sdl2_audio_init(computer);
}

void _update_channels(computer_t *computer) {
	for (size_t i = 0; i < MAX_CHANNELS; i++) {
		channel_t *channel = &computer->channels[i];
		instrument_t *instrument = &computer->ram->instruments[channel->instrument_index];
		
		if (!channel->active) {
			continue;
		}

		if (channel->current_step_index == STEPS_IN_PATTERN) {
			channel->active = false;
			memset(channel, 0, sizeof(channel_t));
			
			continue;
		}
		
		if (channel->time_left_on_current_step > 0) {
			channel->time_left_on_current_step--;
			continue;
		}
	
		pattern_t *pattern = &computer->ram->patterns[channel->pattern_index];
	
		int octave = pattern->steps[channel->current_step_index].pitch / 12 + BASE_OCTAVE;
	
		channel->instrument_index = pattern->steps[channel->current_step_index].instrument_index;
		channel->frequency = _note_to_freq(pattern->steps[channel->current_step_index].pitch, octave);
		channel->amplitude = 0.01f * (float)pattern->steps[channel->current_step_index].volume;
	
		channel->time_left_on_current_step = SAMPLES * pattern->speed;
		channel->current_step_index++;
	}
}

void audio_update(float *buffer, int frames) {
	computer_t *computer = get_global_computer();

	for (int i = 0; i < frames; i++) {
		_update_channels(computer);

		float left = 0.0f;
		float right = 0.0f;

		for (int j = 0; j < MAX_CHANNELS; j++) {
			channel_t *channel = &computer->channels[j];
			if (!channel->active) {
				continue;
			}

			sample_t sample = synth_sample(computer, channel);
			left += sample.left * channel->amplitude;
			right += sample.right * channel->amplitude;
		}

		buffer[i * 2 + 0] = left;
		buffer[i * 2 + 1] = right;
	}
}

size_t audio_play_pattern(computer_t *computer, int pattern_index, int channel_index) {
	if (channel_index == -1) {
		channel_index = channel_alloc(computer);
	}
	channel_t *channel = &computer->channels[channel_index];

	pattern_t *pattern = &computer->ram->patterns[pattern_index];
	channel->active = true;

	channel->current_step_index = 0;
	channel->pattern_index = pattern_index;
	channel->time_left_on_current_step = 0;

	return channel_index;
}
