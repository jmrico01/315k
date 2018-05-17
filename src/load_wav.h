#pragma once

#include "main_platform.h"
#include "km_debug.h"

struct AudioBuffer
{
    int sampleRate;
    int channels;
    int bufferSizeSamples;
    int16* buffer;
};

AudioBuffer LoadWAV(const ThreadContext* thread,
    const char* filePath,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory);