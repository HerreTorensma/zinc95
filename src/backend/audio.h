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

// typedef struct audio_frame {
// 	float left;
// 	float right;
// } audio_frame_t;

// #define AUDIO_BUFFER_SIZE 1024

// typedef struct audio_buffer {
// 	audio_frame_t frames[AUDIO_BUFFER_SIZE];
// 	int frames_amount;
// } audio_buffer_t;

typedef struct sample {
	float left;
	float right;
} sample_t;

typedef enum waveform {
	WAVEFORM_SINE,
	WAVEFORM_SQUARE,
	WAVEFORM_TRIANGLE,
} waveform_t;

typedef struct oscillator {
	waveform_t waveform;
	float freq;
	float phase;
} oscillator_t;

typedef struct voice {
	oscillator_t oscillator;
	float amplitude;

	float duration;
	float time;
	
	bool active;
} voice_t;

#define MAX_VOICES 32

typedef struct voice_pool {
	voice_t voices[MAX_VOICES];
} voice_pool_t;

float note_to_freq(note_t note, int octave);

void audio_update(float *buffer, int frames);