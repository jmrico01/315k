#include "load_wav.h"

#include "km_lib.h"

#define WAVE_FORMAT_PCM         0x0001
#define WAVE_FORMAT_IEEE_FLOAT  0x0003

struct ChunkHeader
{
    char c1, c2, c3, c4;
    int32 dataSize;
};

struct ChunkRIFF
{
    ChunkHeader header;
    char w, a, v, e;
};

struct WaveFormat
{
    int16 audioFormat;
    int16 channels;
    int32 sampleRate;
    int32 byteRate;
    int16 blockAlign;
    int16 bitsPerSample;
    // there might be additional data here
};

bool32 LoadWAV(const ThreadContext* thread, const char* filePath,
    const GameAudio* gameAudio, AudioBuffer* audioBuffer,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory)
{
    DEBUGReadFileResult wavFile = DEBUGPlatformReadFile(thread, filePath);
    if (!wavFile.data) {
        DEBUG_PRINT("Failed to open WAV file at: %s\n", filePath);
        return false;
    }

    ChunkRIFF* riff = (ChunkRIFF*)wavFile.data;
    if (riff->header.c1 != 'R' || riff->header.c2 != 'I'
    || riff->header.c3 != 'F' || riff->header.c4 != 'F') {
        DEBUG_PRINT("Invalid RIFF header on file %s\n", filePath);
        return false;
    }
    if (riff->w != 'W' || riff->a != 'A'
    || riff->v != 'V' || riff->e != 'E') {
        DEBUG_PRINT("Not a WAVE file: %s\n", filePath);
        return false;
    }

    ChunkHeader* fmtHeader = (ChunkHeader*)(riff + 1);
    if (fmtHeader->c1 != 'f' || fmtHeader->c2 != 'm' || fmtHeader->c3 != 't') {
        DEBUG_PRINT("Invalid fmt header on file: %s\n", filePath);
        return false;
    }
    WaveFormat* format = (WaveFormat*)(fmtHeader + 1);
    if (format->audioFormat != WAVE_FORMAT_IEEE_FLOAT) {
        DEBUG_PRINT("WAV format isn't IEEE float (%d) for %s\n",
            format->audioFormat, filePath);
        return false;
    }
    if (format->sampleRate != gameAudio->sampleRate) {
        DEBUG_PRINT("WAV file sample rate mismatch: %d vs %d, for %s\n",
            format->sampleRate, gameAudio->sampleRate, filePath);
        return false;
    }
    if (format->channels != gameAudio->channels) {
        DEBUG_PRINT("WAV file channels mismatch: %d vs %d, for %s\n",
            format->channels, gameAudio->channels, filePath);
        return false;
    }

    int bytesRead = sizeof(ChunkRIFF) + sizeof(ChunkHeader)
        + fmtHeader->dataSize;
    ChunkHeader* header = (ChunkHeader*)((char*)format + fmtHeader->dataSize);
    while (header->c1 != 'd' || header->c2 != 'a'
    || header->c3 != 't' || header->c4 != 'a') {
        /*DEBUG_PRINT("skipped chunk: %c%c%c%c\n",
            header->c1, header->c2, header->c3, header->c4);*/
        int bytesToSkip = sizeof(ChunkHeader) + header->dataSize;
        if (bytesRead + bytesToSkip >= wavFile.size) {
            DEBUG_PRINT("WAV file has no data chunk: %s\n", filePath);
            return false;
        }
        header = (ChunkHeader*)((char*)header + bytesToSkip);
        bytesRead += bytesToSkip;
    }

    void* data = (void*)(header + 1);
    int bytesPerSample = format->bitsPerSample / 8;
    int lengthSamples = header->dataSize / bytesPerSample / format->channels;
    if (lengthSamples > AUDIO_MAX_SAMPLES) {
        DEBUG_PRINT("WAV file too long: %s\n", filePath);
        return false;
    }
    MemCopy(audioBuffer->buffer, data, header->dataSize);

    audioBuffer->sampleRate = format->sampleRate;
    audioBuffer->channels = format->channels;
    audioBuffer->bufferSizeSamples = lengthSamples;

    //DEBUG_PRINT("Loaded WAV file: %s\n", filePath);
    //DEBUG_PRINT("Samples: %d\n", audioBuffer->bufferSizeSamples);

    DEBUGPlatformFreeFileMemory(thread, &wavFile);

    return true;
}

float32 LinearSample(const GameAudio* audio,
    const float32* buffer, int bufferLengthSamples,
    int channel, float32 t)
{
    float32 iFloat = t * bufferLengthSamples;
    int i1 = (int)floorf(iFloat);
    int i2 = (int)ceilf(iFloat);
    float32 val1 = buffer[i1 * audio->channels + channel];
    float32 val2 = buffer[i2 * audio->channels + channel];
    return Lerp(val1, val2, iFloat - i1);
}