#pragma once

#include "main_platform.h"
#include "km_debug.h"

#define AUDIO_SAMPLERATE_MAX 48000
#define AUDIO_CHANNELS_MAX 2
#define AUDIO_BUFFER_MAX_SAMPLES (AUDIO_SAMPLERATE_MAX * 2)

struct AudioBuffer
{
    int sampleRate;
    int channels;
    int bufferSizeSamples;
    float32 buffer[AUDIO_BUFFER_MAX_SAMPLES * AUDIO_CHANNELS_MAX];
};

bool32 LoadWAV(const ThreadContext* thread, const char* filePath,
    const GameAudio* gameAudio, AudioBuffer* audioBuffer,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory);