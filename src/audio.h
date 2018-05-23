#pragma once

#include "km_defines.h"
#include "load_wav.h"

#define WAVE_BUFFER_LENGTH_SECONDS 1
#define WAVE_BUFFER_MAX_SAMPLES (AUDIO_MAX_SAMPLERATE \
    * WAVE_BUFFER_LENGTH_SECONDS)
#define WAVETABLE_MAX_WAVES 4

struct Sound
{
    bool32 play;
    bool32 playing;
    int sampleIndex;

    AudioBuffer buffer;
};

struct Wave
{
    float32 buffer[WAVE_BUFFER_MAX_SAMPLES * AUDIO_MAX_CHANNELS];
};

struct WaveTable
{
    float32 freq;
    float32 amp;

    float32 tWave;
    float32 tWaveTable;
    int bufferLengthSamples;
    int numWaves;
    Wave waves[WAVETABLE_MAX_WAVES];
};

struct AudioState
{
    Sound soundKick;
    Sound soundSnare;
    Sound soundDeath;

    Sound soundNotes[12];

    float32 tWave;
    float32 toneWave;

    WaveTable waveTable;

    bool32 globalMute;

#if GAME_INTERNAL
    bool32 debugView;
#endif
};

struct GameState;
void InitAudioState(const ThreadContext* thread,
    AudioState* audioState, GameAudio* audio,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory);
void OutputAudio(GameAudio* audio, GameState* gameState,
    const GameInput* input, MemoryBlock transient);
