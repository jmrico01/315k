#pragma once

#include <km_common/km_defines.h>

#include "gui.h"
#include "load_wav.h"

#define REPLAY_MIDI_MESSAGES_MAX 1024
#define REPLAY_INSTANCES_MAX 8

#define WAVE_BUFFER_LENGTH_SECONDS 1
#define WAVE_BUFFER_MAX_SAMPLES (AUDIO_MAX_SAMPLERATE * WAVE_BUFFER_LENGTH_SECONDS)

#define WAVETABLE_MAX_WAVES 16
#define WAVETABLE_MAX_VOICES 128
#define WAVETABLE_OSCILLATORS 16
#define WAVETABLE_ENVELOPES 4

const uint64 DEBUG_BUFFER_SAMPLES = MEGABYTES(32);

struct Sound
{
	bool32 play;
	bool32 playing;
	uint64 sampleIndex;

	AudioBuffer buffer;

	void Init(const ThreadContext* thread, const GameAudio* audio,
		const char* filePath, const MemoryBlock& transient,
		DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
		DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory);
	void Update(const GameAudio* audio);
	void WriteSamples(float32 amplitude, GameAudio* audio) const;
};

struct Voice
{
	float32 baseFreq;
	float32 maxAmp;
	int midiNote;

	float32 time;
	float32 tWave;
	float32 freq;
	float32 amp;

	bool32 sustained;
	float32 releaseTime;
	float32 releaseAmp;

	int envelope;
};

struct Oscillator
{
	float32 tWave;
	float32 freq;
	float32 amp;
};

struct Wave
{
	float32 buffer[WAVE_BUFFER_MAX_SAMPLES * AUDIO_MAX_CHANNELS];
};

struct EnvelopeADSR
{
	float32 attack;
	float32 decay;
	float32 sustain;
	float32 release;
};

struct WaveTable
{
	float32 tWaveTable;
	int bufferLengthSamples;
	int numWaves;
	Wave waves[WAVETABLE_MAX_WAVES];

	int activeVoices;
	Voice voices[WAVETABLE_MAX_VOICES];

	Oscillator oscillators[WAVETABLE_OSCILLATORS];

	EnvelopeADSR envelopes[WAVETABLE_ENVELOPES];

	void Init(const GameAudio* audio);
	void Update(const GameAudio* audio, const GameInput* input);
	void WriteSamples(GameAudio* audio);
};

struct ReplayFrameInfo
{
	uint64 frame;
	uint64 midiMessageStart;
	uint64 numMidiMessages;
};

struct MidiInputReplay
{
	uint64 currentFrame;
	uint64 totalFrames;

	uint64 currentFrameInfo;
	uint64 numFrameInfos;
	ReplayFrameInfo frameInfo[REPLAY_MIDI_MESSAGES_MAX];

	uint64 totalMidiMessages;
	MidiMessage midiMessages[REPLAY_MIDI_MESSAGES_MAX];
};

struct AudioState
{
	Sound soundKick;
	Sound soundSnare;
	Sound soundDeath;

	Sound soundNotes[12];

	WaveTable waveTable;

	uint64 activeReplays;
	MidiInputReplay midiInputReplays[REPLAY_INSTANCES_MAX];

	float32 lastSamplesRaw[AUDIO_MAX_CHANNELS];
	float32 lastSamplesFiltered[AUDIO_MAX_CHANNELS];

	bool32 globalMute;

#if GAME_INTERNAL
	bool32 debugView;

	bool32 debugRecording;
	bool32 debugViewRecording;
	uint64 debugBufferSamples;
	float32 debugBuffer[DEBUG_BUFFER_SAMPLES];
	BufferView debugBufferView;

	uint8 arduinoChannel;
#endif

	void Init(const ThreadContext* thread,
		GameAudio* audio, MemoryBlock* transient,
		DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
		DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory);
};

struct GameState;
void OutputAudio(GameAudio* audio, GameState* gameState,
	const GameInput* input, MemoryBlock transient);
