#include <string.h>
#include <libgen.h>

#include <SDL2/SDL_mixer.h>

#include "TUM_Sound.h"
#include "TUM_Utils.h"

#define SAMPLE_FOLDER "/../resources/waveforms/"

#define NUM_WAVEFORMS 20

#define AUDIO_CHANNELS 2
#define MIXING_CHANNELS 4

#define GEN_FULL_SAMPLE_PATH(SAMPLE) SAMPLE_FOLDER #SAMPLE ".wav",

Mix_Chunk *samples[NUM_WAVEFORMS] = { 0 };

static char TUMSound_online = 0;

typedef struct loaded_sample {
    char *name;
    Mix_Chunk *sample;
    struct loaded_sample *next;
} loaded_sample_t;

loaded_sample_t user_samples = { .name = "HEAD" };

void tumSoundExit(void)
{
#ifndef DOCKER
    unsigned int i;

    for (i = 0; i < NUM_WAVEFORMS; i++) {
        Mix_FreeChunk(samples[i]);
    }
#endif /* DOCKER */
}

int tumSoundInit(char *bin_dir_str)
{
#ifndef DOCKER
    char *waveFileNames[] = { FOR_EACH_SAMPLE(GEN_FULL_SAMPLE_PATH) };

    char *fullWaveFileNames[NUM_WAVEFORMS] = { 0 };

    int ret;
    size_t bin_dir_len = strlen(bin_dir_str);
    unsigned int i, j;

    for (i = 0; i < NUM_WAVEFORMS; i++) {
        fullWaveFileNames[i] =
            calloc(1, sizeof(char) * (strlen(waveFileNames[i]) +
                                      bin_dir_len + 1));
        if (!fullWaveFileNames[i]) {
            PRINT_ERROR("Failed to allocate file name #%d '%s'", i,
                        waveFileNames[i]);
            goto err_file_names;
        }
        strcpy(fullWaveFileNames[i], bin_dir_str);
        strcat(fullWaveFileNames[i], waveFileNames[i]);
    }

    if (Mix_OpenAudio(22050, AUDIO_S16SYS, AUDIO_CHANNELS, 512)) {
        PRINT_ERROR("Failed to open audio with #%d channels",
                    AUDIO_CHANNELS);
        goto err_open_audio;
    }

    if ((ret = Mix_AllocateChannels(MIXING_CHANNELS)) != MIXING_CHANNELS) {
        PRINT_ERROR("Failed to allocate %d channels, only %d allocated",
                    MIXING_CHANNELS, ret);
        goto err_allocate_channels;
    }

    for (i = 0; i < NUM_WAVEFORMS; i++) {
        samples[i] = Mix_LoadWAV(tumUtilFindResourcePath(fullWaveFileNames[i]));
        if (!samples[i]) {
            PRINT_ERROR("Failed to load WAV: %s",
                        fullWaveFileNames[i]);
            goto err_loadWAV;
        }
    }

    for (j = 0; j < i; j++) {
        free(fullWaveFileNames[j]);
    }

    TUMSound_online = 1;

    atexit(tumSoundExit);

    return 0;

err_loadWAV:
    for (j = 0; j < i; j++) {
        Mix_FreeChunk(samples[j]);
    }
err_allocate_channels:
    Mix_CloseAudio();
err_open_audio:
    i--;
err_file_names:
    for (j = 0; j < i; j++) {
        free(fullWaveFileNames[j]);
    }
    return -1;
#else
#warning "Sound API is not available in Container!"
    return 0;
#endif /* DOCKER */
}

void tumSoundPlaySample(unsigned char index)
{
#ifndef DOCKER
    if (TUMSound_online) {
        Mix_PlayChannel(-1, samples[index], 0);
    }
#endif /* DOCKER */
}

int tumSoundLoadUserSample(const char *filepath)
{
    if (!TUMSound_online) {
        PRINT_ERROR(
            "Trying to load sample without calling 'tumSoundInit'");
        return -1;
    }

    if (filepath == NULL) {
        PRINT_ERROR("Sample filepath is not valid");
        return -1;
    }

    loaded_sample_t *new_sample = calloc(1, sizeof(loaded_sample_t));
    if (new_sample == NULL) {
        PRINT_ERROR("Could not allocate new sample");
        return -1;
    }

    new_sample->name = strdup(basename((char *)filepath));
    if (new_sample->name == NULL) {
        PRINT_ERROR("Could not allocate name of sample '%s'", filepath);
        goto err_name;
    }

    new_sample->sample = Mix_LoadWAV(filepath);
    if (new_sample->sample == NULL) {
        PRINT_ERROR("Count not load sample '%s'", filepath);
        goto err_sample;
    }

    loaded_sample_t *iterator = &user_samples;

    for (; iterator->next; iterator = iterator->next)
        ;

    iterator->next = new_sample;

    return 0;
err_sample:
    free(new_sample->name);
err_name:
    free(new_sample);
    return -1;
}

int tumSoundPlayUserSample(const char *filename)
{
    if (filename == NULL) {
        PRINT_ERROR("Invalid sample name provided");
        return -1;
    }

    loaded_sample_t *iterator = &user_samples;

    for (; iterator; iterator = iterator->next)
        if (!strcmp(basename((char *)filename), iterator->name)) {
            goto found_sample;
        }

    return -1;

found_sample:
    if (!iterator->sample) {
        PRINT_ERROR("Sample '%s' does not have a loaded sample",
                    iterator->name ? iterator->name : "NULL");
        return -1;
    }
    Mix_PlayChannel(-1, iterator->sample, 0);
    return 0;
}
