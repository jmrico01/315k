#pragma once

#include <km_common/km_debug.h>

#include "main_platform.h"

#define AUDIO_MAX_SAMPLERATE 48000
#define AUDIO_MAX_CHANNELS 2
#define AUDIO_MAX_LENGTH_SECS 8
#define AUDIO_MAX_SAMPLES (AUDIO_MAX_SAMPLERATE * AUDIO_MAX_LENGTH_SECS)

struct AudioBuffer
{
    uint32 sampleRate;
    uint8 channels;
    uint64 bufferSizeSamples;
    float32 buffer[AUDIO_MAX_SAMPLES * AUDIO_MAX_CHANNELS];
};

bool32 LoadWAV(const ThreadContext* thread, const char* filePath,
    const GameAudio* gameAudio, AudioBuffer* audioBuffer,
    const MemoryBlock& transient,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory);

float32 LinearSample(const GameAudio* audio,
    const float32* buffer, uint64 bufferLengthSamples,
    int channel, float32 t);