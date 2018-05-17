#include "load_wav.h"

#define WAV_AUDIO_FORMAT_PCM_16 1

struct ChunkRIFF
{
    char r, i, f1, f2;
	int32 chunkSize;
    char w, a, v, e;
};

struct ChunkFmt
{
    char f, m, t, z;
	int32 chunkSize;
	int16 audioFormat;
	int16 channels;
	int32 sampleRate;
	int32 byteRate;
	int16 blockAlign;
	int16 bitsPerSample;
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
    if (riff->r != 'R' || riff->i != 'I'
    || riff->f1 != 'F' || riff->f2 != 'F') {
        DEBUG_PRINT("Invalid RIFF header on file %s\n", filePath);
        return false;
    }
    if (riff->w != 'W' || riff->a != 'A'
    || riff->v != 'V' || riff->e != 'E') {
        DEBUG_PRINT("Not a WAVE file: %s\n", filePath);
        return false;
    }

    ChunkFmt* fmt = (ChunkFmt*)((char*)wavFile.data + sizeof(ChunkRIFF));
    if (fmt->f != 'f' || fmt->m != 'm' || fmt->t != 't') {
        DEBUG_PRINT("Invalid FMT header on file: %s\n", filePath);
        return false;
    }
    if (fmt->audioFormat != WAV_AUDIO_FORMAT_PCM_16) {
        DEBUG_PRINT("WAV format isn't PCM 16-bit for %s", filePath);
        return false;
    }
    if (fmt->sampleRate != gameAudio->sampleRate) {
        DEBUG_PRINT("WAV file sample rate mismatch: %d vs %d, for %s\n",
            fmt->sampleRate, gameAudio->sampleRate, filePath);
        return false;
    }
    if (fmt->channels != gameAudio->channels) {
        DEBUG_PRINT("WAV file channels mismatch: %d vs %d, for %s\n",
            fmt->channels, gameAudio->channels, filePath);
        return false;
    }

    int32 dataChunkSize = *(int32*)((char*)wavFile.data
    	+ sizeof(ChunkRIFF) + sizeof(ChunkFmt) + sizeof(int32));
    int16* data = (int16*)((char*)wavFile.data
    	+ sizeof(ChunkRIFF) + sizeof(ChunkFmt) + 2 * sizeof(int32));
    int bytesPerSample = fmt->bitsPerSample * fmt->channels / 8;
    int samples = dataChunkSize / bytesPerSample;
    if (samples > AUDIO_BUFFER_MAX_SAMPLES) {
        DEBUG_PRINT("WAV file too long: %s\n", filePath);
        return false;
    }
    for (int i = 0; i < samples; i++) {
        audioBuffer->buffer[i * 2] = data[i * 2];
        audioBuffer->buffer[i * 2 + 1] = data[i * 2 + 1];
    }

    audioBuffer->sampleRate = fmt->sampleRate;
    audioBuffer->channels = fmt->channels;
    audioBuffer->bufferSizeSamples = samples;

    DEBUG_PRINT("Loaded WAV file: %s\n", filePath);
    DEBUG_PRINT("Samples: %d\n", audioBuffer->bufferSizeSamples);

    DEBUGPlatformFreeFileMemory(thread, &wavFile);

    return true;
}