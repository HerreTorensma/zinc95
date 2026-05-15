/*
Audio stuff

The goal is to implement FM-synthesis also with cool visualizations of the waves and whatnot
And I want to seperate the audio API from whatever backend library is used to play it, like SDL2 or Miniaudio

I want to be able to generate a big variety of sounds without having to use an external editor

Some notes for self
phase: where you are on the cycle of a waveform
frequency: how dense the waves are kind of, controls the pitch

*/

#pragma once

#include <stdbool.h>

#include "../computer.h"

typedef enum note {
	NOTE_C,
	NOTE_CSHARP,
	NOTE_D,
	NOTE_DSHARP,
	NOTE_E,
	NOTE_F,
	NOTE_FSHARP,
	NOTE_G,
	NOTE_GSHARP,
	NOTE_A,
	NOTE_ASHARP,
	NOTE_B,
} note_t;

extern const char *note_to_string_map[];

sample_t synth_sample(computer_t *computer, channel_t *channel);

int audio_get_channel_current_step(computer_t *computer, int channel_index);

void audio_cancel_channel(computer_t *computer, int channel_index);

void audio_init(computer_t *computer);

void audio_deinit(computer_t *computer);

void audio_update(float *buffer, int frames);

size_t audio_play_pattern(computer_t *computer, int pattern_index, int channel_index);

float *audio_get_stream();
