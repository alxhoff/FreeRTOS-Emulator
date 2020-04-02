#include <string.h>

#include <SDL2/SDL_mixer.h>

#include "TUM_Sound.h"
#include "TUM_Utils.h"

#define SAMPLE_FOLDER "/../resources/waveforms/"

#define NUM_WAVEFORMS 20

#define AUDIO_CHANNELS 2
#define MIXING_CHANNELS 4

#define GEN_FULL_SAMPLE_PATH(SAMPLE) SAMPLE_FOLDER #SAMPLE ".wav",

char *waveFileNames[] = { FOR_EACH_SAMPLE(GEN_FULL_SAMPLE_PATH) };

char *fullWaveFileNames[NUM_WAVEFORMS] = { 0 };

Mix_Chunk *samples[NUM_WAVEFORMS] = { 0 };

void vExitAudio(void)
{
#ifndef DOCKER
	unsigned int i;

	for (i = 0; i < NUM_WAVEFORMS; i++)
		Mix_FreeChunk(samples[i]);
#endif /* DOCKER */
}

int vInitAudio(char *bin_dir_str)
{
#ifndef DOCKER
    int ret;
	size_t bin_dir_len = strlen(bin_dir_str);
	unsigned int i, j;

	for (i = 0; i < NUM_WAVEFORMS; i++) {
		fullWaveFileNames[i] =
			calloc(1, sizeof(char) * (strlen(waveFileNames[i]) +
						  bin_dir_len + 1));
        if(!fullWaveFileNames[i]){
            PRINT_ERROR("Failed to allocate file name #%d '%s'", i, waveFileNames[i]);
            goto err_file_names;
        }
		strcpy(fullWaveFileNames[i], bin_dir_str);
		strcat(fullWaveFileNames[i], waveFileNames[i]);
	}

	if(Mix_OpenAudio(22050, AUDIO_S16SYS, AUDIO_CHANNELS, 512)){
        PRINT_ERROR("Failed to open audio with #%d channels", AUDIO_CHANNELS);
        goto err_open_audio;
    }

	if((ret = Mix_AllocateChannels(MIXING_CHANNELS)) != MIXING_CHANNELS){
        PRINT_ERROR("Failed to allocate %d channels, only %d allocated", MIXING_CHANNELS, ret);
        goto err_allocate_channels;
    }

	for (i = 0; i < NUM_WAVEFORMS; i++) {
		samples[i] = Mix_LoadWAV(fullWaveFileNames[i]);
		if (!samples[i]) {
            PRINT_ERROR("Failed to load WAV: %s", fullWaveFileNames[i]);
            goto err_loadWAV;
		}
	}

	atexit(vExitAudio);

    return 0;

err_loadWAV:
    for(j=0; j < i; j++)
        Mix_FreeChunk(samples[j]);
err_allocate_channels:
    Mix_CloseAudio();
err_open_audio:
    i--;
err_file_names:
    for(j=0 ; j < i; j++)
        free(fullWaveFileNames[j]);
    return -1;
#else
#warning "Sound API is not available in Container!"
    return 0;
#endif /* DOCKER */
}

void vPlaySample(unsigned char index)
{
#ifndef DOCKER
	Mix_PlayChannel(-1, samples[index], 0);
#endif /* DOCKER */
}
