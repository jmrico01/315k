#pragma once

#include <sys/types.h>

#include <alsa/asoundlib.h>

#include "km_defines.h"
#include "main_platform.h"

#define LINUX_STATE_FILE_NAME_COUNT  512
#define BYTES_PER_PIXEL 4

struct LinuxWindowDimension
{
    uint32 Width;
    uint32 Height;
};

struct LinuxAudio
{
    snd_pcm_t* pcmHandle;
    snd_pcm_uframes_t periodSize;
    
    uint32 channels;
    uint32 sampleRate;

    int16* buffer;
    uint32 bufferSize;
    uint32 readIndex;

    pthread_t thread;
    bool32 isPlaying;
};

struct LinuxDebugTimeMarker
{
    uint32 OutputPlayCursor;
    uint32 OutputWriteCursor;
    uint32 OutputLocation;
    uint32 OutputByteCount;
    uint32 ExpectedFlipPlayCursor;

    uint32 FlipPlayCursor;
    uint32 FlipWriteCursor;
};

struct LinuxGameCode
{
    void* gameLibHandle;
    ino_t gameLibId;

    // NOTE: Callbacks can be 0!  You must check before calling
    GameUpdateAndRenderFunc* gameUpdateAndRender;

    bool32 isValid;
};

struct LinuxState
{
	uint64 gameMemorySize;
	void* gameMemoryBlock;

    int32 recordingHandle;
    int32 inputRecordingIndex;

    int32 playbackHandle;
    int32 inputPlayingIndex;

    char exeFilePath[LINUX_STATE_FILE_NAME_COUNT];
    char* exeOnePastLastSlash;
};
