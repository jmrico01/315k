#pragma once

#include "km_defines.h"
#include "load_wav.h"

#define SOUND_MAX_VARIATIONS 3

struct Sound
{
    bool32 play;
    bool32 playing;
    int sampleIndex;

    int variations;
    AudioBuffer buffers[SOUND_MAX_VARIATIONS];
    int activeVariation;
};

struct AudioState
{
    int runningSampleIndex;

    Sound soundKick;
    Sound soundSnare;
    Sound soundDeath;

    Sound soundNotes[12];

    float32 tWave;

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
    const GameInput* input, GameMemory* memory);
