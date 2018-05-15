#pragma once

#include <AudioUnit/AudioUnit.h>

#define AUDIO_SAMPLERATE 48000
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFER_SIZE_MILLISECONDS 1000

struct MacOSAudio
{
	AudioStreamBasicDescription audioDescriptor;
	AudioUnit audioUnit;

    int sampleRate;
    int channels;
    int bufferSizeSamples;
	int16* buffer;

	int readCursor;
	int writeCursor;

	int minLatency;
	int maxLatency;
};

void MacOSInitCoreAudio(MacOSAudio* macOSAudio,
    int sampleRate, int channels, int bufferSizeSamples);
void MacOSStopCoreAudio(MacOSAudio* macOSAudio);