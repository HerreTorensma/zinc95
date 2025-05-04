#include "sound.h"
#include "menu.h"
#include "../backend/input.h"

#include <math.h>
#include <SDL2/SDL.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// #define SAMPLE_RATE 44100
// #define SAMPLE_RATE 1000
#define SAMPLE_RATE 22050
#define AMPLITUDE 1000
// #define FREQUENCY 440  // Hz (A4 note)
#define FREQUENCY 220  // Hz (A4 note)

#define BASE_FREQUENCY 440.0 // Base frequency (A4)
#define LFO_FREQUENCY 5.0    // Vibrato speed (Hz)
#define VIBRATO_DEPTH 10.0   // Vibrato depth (max frequency variation in Hz)

void audio_callback(void *userdata, uint8_t *stream, int len) {
	static double phase = 0;
	int16_t *buffer = (int16_t *)stream;
	int length = len / 2;  // 16-bit audio (2 bytes per sample)

	for (int i = 0; i < length; i++) {
		double t = phase / SAMPLE_RATE;
		
		// Square wave: 1 or -1 depending on phase
		// buffer[i] = (sin(2.0 * M_PI * FREQUENCY * t) > 0) ? AMPLITUDE : -AMPLITUDE;

		// Sine wave
		buffer[i] = (int16_t)(AMPLITUDE * sin(2.0 * M_PI * FREQUENCY * t));

		// Sawtooth
		// buffer[i] = (Sint16)(AMPLITUDE * (2.0 * fmod(FREQUENCY * t, 1.0) - 1.0));

		// Triangle
		// buffer[i] = (Sint16)(AMPLITUDE * (2.0 * fabs(fmod(FREQUENCY * t, 1.0) - 0.5) - 1.0));
		
		phase += 1;
	}
}

// void audio_callback(void* userdata, Uint8* stream, int len) {
// 	static double phase = 0;  // Phase for square wave
// 	static double lfo_phase = 0;  // Phase for vibrato LFO
// 	Sint16* buffer = (Sint16*)stream; // Convert stream to 16-bit buffer
// 	int samples = len / 2;  // Each sample is 2 bytes (16-bit audio)

// 	for (int i = 0; i < samples; i++) {
// 		// Generate LFO (sine wave for vibrato)
// 		double vibrato = sin(lfo_phase) * VIBRATO_DEPTH;
		
// 		// Compute the actual frequency with vibrato applied
// 		double current_frequency = BASE_FREQUENCY + vibrato;

// 		// Generate square wave with vibrato effect
// 		buffer[i] = (sin(phase) > 0) ? AMPLITUDE : -AMPLITUDE;

// 		// Increment phase based on vibrato-modulated frequency
// 		phase += 2.0 * M_PI * current_frequency / SAMPLE_RATE;

// 		// Increment LFO phase
// 		lfo_phase += 2.0 * M_PI * LFO_FREQUENCY / SAMPLE_RATE;

// 		// Keep phase values within range
// 		if (phase > 2.0 * M_PI) phase -= 2.0 * M_PI;
// 		if (lfo_phase > 2.0 * M_PI) lfo_phase -= 2.0 * M_PI;
// 	}
// }

void sound_editor_init(computer_t *computer) {
	SDL_AudioSpec wav_spec;

	wav_spec.freq = SAMPLE_RATE;
	// wav_spec.format = AUDIO_S16SYS;
	wav_spec.format = AUDIO_S16LSB;
	wav_spec.channels = 1;
	// wav_spec.samples = 4096;
	wav_spec.samples = 1024;
	wav_spec.callback = audio_callback;

	if (SDL_OpenAudio(&wav_spec, NULL) < 0) {
		printf("SDL_OpenAudio failed! SDL_Error: %s\n", SDL_GetError());
		// return 1;
	}

	// SDL_PauseAudio(0);
}

void sound_editor_update(computer_t *computer) {
	if (input_key_pressed(KEY_F6)) {
		SDL_PauseAudio(0);
	}
	if (input_key_pressed(KEY_F6)) {
		SDL_PauseAudio(1);
	}
}

void sound_editor_draw(computer_t *computer) {
	
}