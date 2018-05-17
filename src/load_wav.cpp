#include "load_wav.h"

struct ChunkRIFF
{
	int32 chunkID; // big endian, boo
	int32 chunkSize;
	int32 format; // big endian, boo
};

struct ChunkFmt
{
	int32 chunkID; // big endian, boo
	int32 chunkSize;
	int16 audioFormat;
	int16 channels;
	int32 sampleRate;
	int32 byteRate;
	int16 blockAlign;
	int16 bitsPerSample;
};

AudioBuffer LoadWAV(const ThreadContext* thread,
    const char* filePath,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory)
{
	AudioBuffer result;
	result.bufferSizeSamples = 0;

    DEBUGReadFileResult wavFile = DEBUGPlatformReadFile(thread, filePath);
    if (!wavFile.data) {
        DEBUG_PRINT("Failed to open WAV file at: %s\n", filePath);
        return result;
    }

    ChunkRIFF* riff = (ChunkRIFF*)wavFile.data;
    DEBUG_PRINT("chunk ID: %d, size: %d, format: %d\n",
    	riff->chunkID, riff->chunkSize, riff->format);

    ChunkFmt* fmt = (ChunkFmt*)((char*)wavFile.data + sizeof(ChunkRIFF));
    DEBUG_PRINT("channels: %d, sampleRate: %d, byteRate: %d, align: %d, bitsPerSample: %d\n",
    	fmt->channels, fmt->sampleRate, fmt->byteRate, fmt->blockAlign, fmt->bitsPerSample);

    int32 dataChunkSize = *(int32*)((char*)wavFile.data
    	+ sizeof(ChunkRIFF) + sizeof(ChunkFmt) + sizeof(int32));
    /*void* data = (void*)((char*)wavFile.data
    	+ sizeof(ChunkRIFF) + sizeof(ChunkFmt) + 2 * sizeof(int32));*/
    DEBUG_PRINT("data chunk size: %d\n", dataChunkSize);

    DEBUGPlatformFreeFileMemory(thread, &wavFile);

    return result;
}