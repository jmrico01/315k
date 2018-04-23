#include "win32_audio.h"

internal bool Win32InitAudio(Win32Audio* winAudio, GameAudio* gameAudio,
    uint32 channels, uint32 sampleRate, uint32 bufSampleLength)
{
    // Only support stereo for now
    DEBUG_ASSERT(channels == 2);

    // Try to load Windows 10 version
	HMODULE xAudio2Lib = LoadLibrary("xaudio2_9.dll");
	if (!xAudio2Lib) {
        // Fall back to Windows 8 version
		xAudio2Lib = LoadLibrary("xaudio2_8.dll");
    }
	if (!xAudio2Lib) {
        // Fall back to Windows 7 version
		xAudio2Lib = LoadLibrary("xaudio2_7.dll");
	}
    if (!xAudio2Lib) {
        // TODO load earlier versions?
        DEBUG_PRINT("Could not find a valid XAudio2 DLL\n");
        return false;
    }

    XAudio2Create = (XAudio2CreateFunc*)GetProcAddress(
        xAudio2Lib, "XAudio2Create");
    if (!XAudio2Create) {
        DEBUG_PRINT("Failed to load XAudio2Create function\n");
        return false;
    }

    HRESULT hr;
    hr = XAudio2Create(&winAudio->xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to create XAudio2 instance, HRESULT %x\n", hr);
        return false;
    }

    hr = winAudio->xAudio2->CreateMasteringVoice(&winAudio->masterVoice,
        channels,
        sampleRate,
        0,
        NULL
    );
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to create mastering voice, HRESULT %x\n", hr);
        return false;
    }

    winAudio->format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    winAudio->format.Format.nChannels = (WORD)channels;
    winAudio->format.Format.nSamplesPerSec = (DWORD)sampleRate;
    winAudio->format.Format.nAvgBytesPerSec = (DWORD)(
        sampleRate * channels * sizeof(int16));
    winAudio->format.Format.nBlockAlign = (WORD)(channels * sizeof(int16));
    winAudio->format.Format.wBitsPerSample = (WORD)(sizeof(int16) * 8);
    winAudio->format.Format.cbSize = (WORD)(
        sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX));
    winAudio->format.Samples.wValidBitsPerSample =
        winAudio->format.Format.wBitsPerSample;
    winAudio->format.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
    winAudio->format.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

    hr = winAudio->xAudio2->CreateSourceVoice(&winAudio->sourceVoice,
        (const WAVEFORMATEX*)&winAudio->format);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to create source voice, HRESULT %x\n", hr);
        return false;
    }

    gameAudio->channels = channels;
    gameAudio->sampleRate = sampleRate;
    gameAudio->bufferSize = bufSampleLength;
	gameAudio->buffer = (int16*)VirtualAlloc(0,
        bufSampleLength * channels * sizeof(int16),
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    for (uint32 i = 0; i < bufSampleLength; i++) {
        gameAudio->buffer[i * channels] = 0;
        gameAudio->buffer[i * channels + 1] = 0;
    }

    winAudio->buffer.Flags = 0;
    winAudio->buffer.AudioBytes = bufSampleLength * channels * sizeof(int16);
    winAudio->buffer.pAudioData = (const BYTE*)gameAudio->buffer;
    winAudio->buffer.PlayBegin = 0;
    winAudio->buffer.PlayLength = 0;
    winAudio->buffer.LoopBegin = 0;
    winAudio->buffer.LoopLength = 0;
    winAudio->buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    winAudio->buffer.pContext = NULL;

    hr = winAudio->sourceVoice->SubmitSourceBuffer(&winAudio->buffer);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to submit buffer, HRESULT %x\n", hr);
        return false;
    }

    hr = winAudio->sourceVoice->Start(0);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to start source voice, HRESULT %x\n", hr);
        return false;
    }

    return true;
}
