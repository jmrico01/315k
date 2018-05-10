#pragma once

#include <Xaudio2.h>

#include "km_defines.h"
#include "main_platform.h"

#define AUDIO_SAMPLERATE 48000
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFER_SIZE_MILLISECONDS  2000

struct Win32Audio
{
    IXAudio2SourceVoice* sourceVoice;

    int sampleRate;
    int channels;
    int bufferSizeSamples;
    int16* buffer;

    int minLatency;
    int maxLatency;
};

// XAudio2 functions
typedef HRESULT XAudio2CreateFunc(
    _Out_   IXAudio2**          ppXAudio2,
    _In_    UINT32              flags,
    _In_    XAUDIO2_PROCESSOR   xAudio2Processor);
internal XAudio2CreateFunc* xAudio2Create_ = NULL;
#define XAudio2Create xAudio2Create_


internal bool Win32InitAudio(Win32Audio* winAudio,
    int sampleRate, int channels, int bufferSizeSamples);

#if GAME_INTERNAL

#define AUDIO_RECORDING_MAX_SAMPLES (AUDIO_SAMPLERATE * 30)

struct Win32AudioRecording
{
    bool32 recording;
    int bufferSizeSamples;
    int16* buffer;
};

#endif