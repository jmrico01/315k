#pragma once

#include <audioclient.h>
#include <mmdeviceapi.h>

#include "km_defines.h"
#include "main_platform.h"

#define AUDIO_DEFAULT_SAMPLERATE 48000
#define AUDIO_DEFAULT_CHANNELS 2
#define AUDIO_DEFAULT_BUFFER_SIZE_MILLISECONDS 1000

enum AudioFormat
{
    AUDIO_FORMAT_PCM_INT16,
    AUDIO_FORMAT_PCM_FLOAT32
};

struct Win32Audio
{
    IAudioClient* audioClient;
    IAudioRenderClient* renderClient;
    IAudioClock* audioClock;
    AudioFormat format;
    int bitsPerSample;

    int sampleRate;
    int channels;
    int bufferSizeSamples;
    
    uint64 samplesPlayedPrev;
    int frameTimeSamples;
    int latency;
};


bool32 Win32InitAudio(Win32Audio* winAudio,
    int sampleRate, int channels, int bufferSizeSamples);
void Win32StopAudio(Win32Audio* winAudio);

void Win32WriteAudioSamples(const Win32Audio* winAudio,
    const GameAudio* gameAudio, int numSamples);