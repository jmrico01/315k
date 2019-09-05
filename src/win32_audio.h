#pragma once

#include <audioclient.h>
#include <km_common/km_defines.h>
#include <mmdeviceapi.h>

#include "main_platform.h"

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
	uint16 bitsPerSample;

	uint32 sampleRate;
	uint8 channels;
	uint64 bufferSizeSamples;
	
	uint64 latency;

	bool32 midiInBusy;
	MidiInput midiIn;
};

bool32 Win32InitAudio(Win32Audio* winAudio, uint64 bufferSizeMilliseconds);
void Win32StopAudio(Win32Audio* winAudio);

void Win32WriteAudioSamples(const Win32Audio* winAudio,
	const GameAudio* gameAudio, uint64 numSamples);
