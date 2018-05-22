#include "win32_audio.h"

#include "km_debug.h"

// REFERENCE_TIME as defined by Windows API
#define REFERENCE_TIME_NANOSECONDS 100
// Careful when using this value: close to int overflow
#define REFERENCE_TIMES_PER_SECOND (1000000000 / REFERENCE_TIME_NANOSECONDS)

bool32 Win32InitAudio(Win32Audio* winAudio,
    int sampleRate, int channels, int bufferSizeSamples)
{
    // TODO release/CoTaskMemFree on failure
    HRESULT hr;

    IMMDeviceEnumerator* deviceEnumerator;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
        CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
        (void**)&deviceEnumerator);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to create device enumerator\n");
        return false;
    }

    IMMDevice* device;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to get default audio endpoint\n");
        return false;
    }

    IAudioClient* audioClient;
    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL,
        (void**)&audioClient);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to activate audio device\n");
        return false;
    }

    WAVEFORMATEX* format;
    hr = audioClient->GetMixFormat(&format);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to get audio device format\n");
        return false;
    }

    // TODO do this differently: query for several formats with some priority
    //  e.g. 1st PCM float, 2nd PCM int16 ext, 3rd PCM int16
    DEBUG_PRINT("---------- Audio device format ----------\n");
    DEBUG_PRINT("Sample rate: %d\n", format->nSamplesPerSec);
    DEBUG_PRINT("Channels: %d\n", format->nChannels);
    DEBUG_PRINT("Bits per sample: %d\n", format->wBitsPerSample);
    DEBUG_PRINT("Block align: %d\n", format->nBlockAlign);
    switch (format->wFormatTag) {
        case WAVE_FORMAT_PCM: {
            DEBUG_PRINT("Format: PCM\n");
            winAudio->format = AUDIO_FORMAT_PCM_INT16;
            winAudio->bitsPerSample = (int)format->wBitsPerSample;
        } break;
        case WAVE_FORMAT_EXTENSIBLE: {
            if (sizeof(WAVEFORMATEX) + format->cbSize
            < sizeof(WAVEFORMATEXTENSIBLE)) {
                DEBUG_PRINT("Extended format, invalid structure size\n");
                return false;
            }

            WAVEFORMATEXTENSIBLE* formatExt = (WAVEFORMATEXTENSIBLE*)format;
            DEBUG_PRINT("Valid bits per sample: %d\n",
                formatExt->Samples.wValidBitsPerSample);
            DEBUG_PRINT("Channel mask: %d\n", formatExt->dwChannelMask);
            if (formatExt->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
                DEBUG_PRINT("Format: PCM ext\n");
                winAudio->format = AUDIO_FORMAT_PCM_INT16;
                winAudio->bitsPerSample =
                    (int)formatExt->Samples.wValidBitsPerSample; // TODO wrong
            }
            else if (formatExt->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
                DEBUG_PRINT("Format: IEEE float\n");
                winAudio->format = AUDIO_FORMAT_PCM_FLOAT32;
                winAudio->bitsPerSample =
                    (int)formatExt->Samples.wValidBitsPerSample; // TODO wrong
            }
            else {
                DEBUG_PRINT("Unrecognized audio device ext format: %d\n",
                    formatExt->SubFormat);
                return false;
            }
        } break;
        default: {
            DEBUG_PRINT("Unrecognized audio device format: %d\n",
                format->wFormatTag);
            return false;
        } break;
    }
    DEBUG_PRINT("-----------------------------------------\n");

    REFERENCE_TIME bufferSizeRefTimes = REFERENCE_TIMES_PER_SECOND
        / sampleRate * bufferSizeSamples;
    hr = audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED, // TODO should I use exclusive mode?
        0,
        bufferSizeRefTimes,
        0,
        format,
        NULL
    );
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to initialize audio client\n");
        return false;
    }

    UINT32 bufferSizeFrames;
    hr = audioClient->GetBufferSize(&bufferSizeFrames);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to get audio buffer size\n");
        return false;
    }

    IAudioRenderClient* renderClient;
    hr = audioClient->GetService(__uuidof(IAudioRenderClient),
        (void**)&renderClient);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to get audio render client\n");
        return false;
    }

    IAudioClock* audioClock;
    hr = audioClient->GetService(__uuidof(IAudioClock),
        (void**)&audioClock);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to get audio clock\n");
        return false;
    }

    // Dummy get/release calls for setup purposes
    BYTE* buffer;
    hr = renderClient->GetBuffer(0, &buffer);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to get audio render client buffer\n");
        return false;
    }
    hr = renderClient->ReleaseBuffer(0, 0);
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to release audio render client buffer\n");
        return false;
    }

    winAudio->sampleRate = (int)format->nSamplesPerSec;
    winAudio->channels = (int)format->nChannels;
    winAudio->bufferSizeSamples = (int)bufferSizeFrames;

    winAudio->audioClient = audioClient;
    winAudio->renderClient = renderClient;
    winAudio->audioClock = audioClock;

    hr = audioClient->Start();
    if (FAILED(hr)) {
        DEBUG_PRINT("Failed to start audio client\n");
        return false;
    }

    return true;
}

void Win32StopAudio(Win32Audio* winAudio)
{
    winAudio->audioClient->Stop();
}

void Win32WriteAudioSamples(const Win32Audio* winAudio,
    const GameAudio* gameAudio, int numSamples)
{
    DEBUG_ASSERT(numSamples <= winAudio->bufferSizeSamples);

    BYTE* audioBuffer;
    HRESULT hr = winAudio->renderClient->GetBuffer((UINT32)numSamples,
        &audioBuffer);
    if (SUCCEEDED(hr)) {
        if (winAudio->format == AUDIO_FORMAT_PCM_FLOAT32) {
            float32* buffer = (float32*)audioBuffer;
            for (int i = 0; i < numSamples; i++) {
                buffer[i * winAudio->channels] =
                    gameAudio->buffer[i * gameAudio->channels];
                buffer[i * winAudio->channels + 1] =
                    gameAudio->buffer[i * gameAudio->channels + 1];
            }
        }
        winAudio->renderClient->ReleaseBuffer((UINT32)numSamples, 0);
    }
}