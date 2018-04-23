#pragma once

#include <Xaudio2.h>

#include "km_defines.h"

struct Win32Audio
{
    IXAudio2* xAudio2;
    IXAudio2MasteringVoice* masterVoice;
    IXAudio2SourceVoice* sourceVoice;
    WAVEFORMATEXTENSIBLE format;
    XAUDIO2_BUFFER buffer;

    int sampleLatency;
};

// XAudio2 functions
typedef HRESULT XAudio2CreateFunc(
    _Out_   IXAudio2**          ppXAudio2,
    _In_    UINT32              flags,
    _In_    XAUDIO2_PROCESSOR   xAudio2Processor);
internal XAudio2CreateFunc* xAudio2Create_ = NULL;
#define XAudio2Create xAudio2Create_