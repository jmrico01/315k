#pragma once

#include <AudioUnit/AudioUnit.h>

struct MacOSAudio
{
	uint32 bufferSize;
	int16* buffer;
	int16* readCursor;
	int16* writeCursor;

	AudioStreamBasicDescription audioDescriptor;
	AudioUnit audioUnit;

#if 0
	AudioQueueRef AudioQueue;
	AudioQueueBufferRef AudioBuffers[2];
	bool32 RanAFrame;
#endif
};