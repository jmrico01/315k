#pragma once

#include "main_platform.h"
#include "km_debug.h"

#define AUDIO_SAMPLERATE_MAX 48000
#define AUDIO_CHANNELS_MAX 2
#define AUDIO_BUFFER_MAX_SAMPLES (2 * \
    AUDIO_SAMPLERATE_MAX * AUDIO_CHANNELS_MAX)

struct AudioBuffer
{
    int sampleRate;
    int channels;
    int bufferSizeSamples;
    int16 buffer[AUDIO_BUFFER_MAX_SAMPLES];
};

bool32 LoadWAV(const ThreadContext* thread, const char* filePath,
    const GameAudio* gameAudio, AudioBuffer* audioBuffer,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory);