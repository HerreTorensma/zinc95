#include "audio.h"

#include <SDL2/SDL.h>

void audio_update(float *buffer, int frames);

static void _sdl2_audio_callback(void *userdata, uint8_t *stream, int len) {
	float *out = (float *)stream;
	int frames = len / (sizeof(float) * CHANNELS);
	audio_update(out, frames);
}

void audio_init(computer_t *computer) {
	SDL_AudioSpec wav_spec;

	SDL_AudioSpec spec = {
		.freq = SAMPLE_RATE,
		// .format = AUDIO_S16LSB,
		.format = AUDIO_F32SYS, // A sample is a 32 bit float
		// .channels = 1, // 1 channel for stereo audio
		.channels = 2, // 2 channels for stereo audio
		.samples = SAMPLES, // 1024 size of the audio buffer to be filled
		.callback = _sdl2_audio_callback,
	};

	if (SDL_OpenAudio(&spec, NULL) < 0) {
		printf("SDL_OpenAudio failed! SDL_Error: %s\n", SDL_GetError());
	}

	SDL_PauseAudio(0);
}

void audio_deinit(computer_t *computer) {
	SDL_CloseAudio();
}