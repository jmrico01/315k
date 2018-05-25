#pragma once

#include "km_defines.h"
#include "load_wav.h"

#define WAVE_BUFFER_LENGTH_SECONDS 1
#define WAVE_BUFFER_MAX_SAMPLES (AUDIO_MAX_SAMPLERATE \
    * WAVE_BUFFER_LENGTH_SECONDS)

#define WAVETABLE_MAX_WAVES 16
#define WAVETABLE_MAX_VOICES 16

struct Sound
{
    bool32 play;
    bool32 playing;
    int sampleIndex;

    AudioBuffer buffer;
};

struct Voice
{
    float32 t;
    float32 freq;
    float32 amp;
    bool32 active;
};

struct Wave
{
    float32 buffer[WAVE_BUFFER_MAX_SAMPLES * AUDIO_MAX_CHANNELS];
};

struct WaveTable
{
    /*float32 tWave;
    float32 waveFreq;
    float32 waveAmp;*/

    float32 tWaveTable;
    int bufferLengthSamples;
    int numWaves;
    Wave waves[WAVETABLE_MAX_WAVES];

    int numVoices;
    Voice voices[WAVETABLE_MAX_VOICES];

    float32 tOsc1;
    float32 osc1Freq;
    float32 osc1Amp;
    float32 tOsc2;
    float32 osc2Freq;
    float32 osc2Amp;

    Voice osc1;
    Voice osc2;
};

struct AudioState
{
    Sound soundKick;
    Sound soundSnare;
    Sound soundDeath;

    Sound soundNotes[12];

    WaveTable waveTable;

    bool32 globalMute;

#if GAME_INTERNAL
    bool32 debugView;
#endif
};

struct GameState;
void InitAudioState(const ThreadContext* thread,
    AudioState* audioState, GameAudio* audio,
    MemoryBlock* transient,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory);
void OutputAudio(GameAudio* audio, GameState* gameState,
    const GameInput* input, MemoryBlock transient);
