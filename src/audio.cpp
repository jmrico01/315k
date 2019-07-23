#include "audio.h"

#include "main.h"
#include "km_debug.h"

const uint8 MIDI_EVENT_NOTEON  = 0x9;
const uint8 MIDI_EVENT_NOTEOFF = 0x8;

internal inline float32 MidiNoteToFreq(int midiNote)
{
	return 261.625565f * powf(2.0f, (midiNote - 60.0f) / 12.0f);
}

internal float32 SquareWave(float32 t)
{
	float32 tMod = fmod(t, 2.0f * PI_F);
	return tMod < PI_F ? 1.0f : -1.0f;
}
internal float32 SawtoothWave(float32 t)
{
	float32 tMod = fmod(t, 2.0f * PI_F);
	if (tMod < PI_F) {
		return tMod / PI_F;
	}
	else {
		return (tMod - 2.0f * PI_F) / PI_F;
	}
}
internal float32 TriangleWave(float32 t)
{
	float32 tMod = fmod(t, 2.0f * PI_F);
	if (tMod < PI_F / 2.0f) {
		return tMod / (PI_F / 2.0f);
	}
	else if (tMod < PI_F * 3.0f / 2.0f) {
		return (tMod - PI_F / 2.0f) / PI_F * -2.0f + 1.0f;
	}
	else {
		return (tMod - PI_F * 3.0f / 2.0f) / (PI_F / 2.0f) - 1.0f;
	}
}

void Sound::Init(const ThreadContext* thread, const GameAudio* audio,
	const char* filePath, const MemoryBlock& transient,
	DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
	DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory)
{
	play = false;
	playing = false;
	sampleIndex = 0;

	LoadWAV(thread, filePath, audio, &buffer, transient,
		DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
}

void Sound::Update(const GameAudio* audio)
{
	if (playing) {
		sampleIndex += audio->sampleDelta;
		if (sampleIndex >= buffer.bufferSizeSamples) {
			playing = false;
		}
	}
	if (play) {
		play = false;
		playing = true;
		sampleIndex = 0;
	}
}

void Sound::WriteSamples(float32 amplitude, GameAudio* audio) const
{
	// TODO eh... last minute decisions
	if (!playing) {
		return;
	}

	uint64 samplesToWrite = audio->fillLength;
	if (sampleIndex + samplesToWrite > buffer.bufferSizeSamples) {
		samplesToWrite = buffer.bufferSizeSamples - sampleIndex;
	}
	for (uint64 i = 0; i < samplesToWrite; i++) {
		uint64 sampleInd = sampleIndex + i;
		float32 sample1 = amplitude
			* buffer.buffer[sampleInd * audio->channels];
		float32 sample2 = amplitude
			* buffer.buffer[sampleInd * audio->channels + 1];

		audio->buffer[i * audio->channels] += sample1;
		audio->buffer[i * audio->channels + 1] += sample2;
	}
}

void WaveTable::Init(const GameAudio* audio)
{
	tWaveTable = 0.0f;

	int waveBufferLength = WAVE_BUFFER_LENGTH_SECONDS * audio->sampleRate;
	bufferLengthSamples = waveBufferLength;
	// Instance-specific wave table parameters
	// Ideally there would be some way to feed these in
	numWaves = 4;
	for (int i = 0; i < waveBufferLength; i++) {
		float32 t = 2.0f * PI_F * (float32)i / audio->sampleRate;
		int ind1 = i * audio->channels;
		int ind2 = i * audio->channels + 1;
		waves[0].buffer[ind1] = sinf(t);
		waves[0].buffer[ind2] = sinf(t);
		waves[1].buffer[ind1] = 0.9f * TriangleWave(t);
		waves[1].buffer[ind2] = 0.9f * TriangleWave(t);
		waves[2].buffer[ind1] = 0.3f * SawtoothWave(t);
		waves[2].buffer[ind2] = 0.3f * SawtoothWave(t);
		waves[3].buffer[ind1] = 0.3f * SquareWave(t);
		waves[3].buffer[ind2] = 0.3f * SquareWave(t);
	}

	activeVoices = 0;

	for (int i = 0; i < WAVETABLE_OSCILLATORS; i++) {
		oscillators[i].tWave = 0.0f;
		oscillators[i].freq = 0.0f;
		oscillators[i].amp = 0.0f;
	}
	for (int i = 0; i < WAVETABLE_OSCILLATORS; i++) {
		oscillators[i].freq = RandFloat32(0.2f, 2.0f);
		oscillators[i].amp = 0.0f;
	}

	envelopes[0].attack = 0.05f;
	envelopes[0].decay = 0.05f;
	envelopes[0].sustain = 0.8f;
	envelopes[0].release = 0.1f;
}

void WaveTable::Update(const GameAudio* audio, const GameInput* input)
{
	for (int i = 0; i < activeVoices; i++) {
		voices[i].time += (float32)audio->sampleDelta / audio->sampleRate;
		voices[i].tWave += voices[i].freq * audio->sampleDelta / audio->sampleRate;
		voices[i].tWave = fmod(voices[i].tWave, 1.0f);
	}
	for (int i = 0; i < WAVETABLE_OSCILLATORS; i++) {
		oscillators[i].tWave += oscillators[i].freq * audio->sampleDelta / audio->sampleRate;
		oscillators[i].tWave = fmod(oscillators[i].tWave, 1.0f);
	}

	if (input->arduinoIn.connected) {
		const ArduinoInput& arduinoInput = input->arduinoIn;
		tWaveTable = arduinoInput.analogValues[0][0];
		oscillators[WAVETABLE_OSCILLATORS - 1].amp = arduinoInput.analogValues[0][2];
		oscillators[WAVETABLE_OSCILLATORS - 1].freq = arduinoInput.analogValues[0][3] * 20.0f;

		envelopes[0].attack = arduinoInput.analogValues[0][4] * 5.0f;
		envelopes[0].release = arduinoInput.analogValues[0][5] * 5.0f;
	}
	else {
		float32 tWaveTablePixRange = 600.0f;
		float32 tWaveTablePixOffset = 200.0f;
		float32 tWaveTableT = (input->mousePos.y - tWaveTablePixOffset) / tWaveTablePixRange;
		tWaveTable = Lerp(0.0f, 1.0f, tWaveTableT);
		tWaveTable = ClampFloat32(tWaveTable, 0.0f, 1.0f);
	}

	// Drive WaveTable voices with MIDI input
	for (int i = 0; i < input->midiIn.numMessages; i++) {
		uint8 status = input->midiIn.messages[i].status;
		uint8 dataByte1 = input->midiIn.messages[i].dataByte1;
		//uint8 dataByte2 = input->midiIn.messages[i].dataByte2;
		uint8 event = status >> 4;
		//uint8 channel = status & 0xf;
		switch (event) {
			case MIDI_EVENT_NOTEON: {
				int midiNote = (int)dataByte1;
				int vInd = -1;

				// Try to find an existing voice with matching MIDI note
				// TODO need to rethink this a bit maybe. don't wanna hard-reset voice
				bool32 overwroteVoice = false;
				for (int v = 0; v < activeVoices; v++) {
					if (voices[v].midiNote == midiNote) {
						vInd = v;
						overwroteVoice = true;
						/*voices[v].time = 0.0f;
						voices[v].baseFreq = MidiNoteToFreq(midiNote);
						voices[v].maxAmp = 0.2f;
						voices[v].sustained = true;
						overwroteVoice = true;*/
						break;
					}
				}
				if (!overwroteVoice) {
					// No existing voice, must add a new one
					if (activeVoices >= WAVETABLE_MAX_VOICES) {
						continue;
					}
					vInd = activeVoices++;
				}

				voices[vInd].time = 0.0f;
				voices[vInd].baseFreq = MidiNoteToFreq(midiNote);
				voices[vInd].maxAmp = 0.2f;
				voices[vInd].midiNote = midiNote;
				voices[vInd].sustained = true;
				voices[vInd].envelope = 0;
			} break;
			case MIDI_EVENT_NOTEOFF: {
				int midiNote = (int)dataByte1;
				if (activeVoices <= 0) {
					continue;
				}
				for (int v = 0; v < activeVoices; v++) {
					if (voices[v].midiNote == midiNote) {
						voices[v].sustained = false;
						voices[v].releaseTime = voices[v].time;
						voices[v].releaseAmp = voices[v].amp;
					}
				}
			} break;
			default: {
				LOG_INFO("Unknown MIDI event: %d\n", event);
			} break;
		}
	}

	// Apply oscillators
	if (IsKeyPressed(input, KM_KEY_Y)) {
		for (int i = 0; i < WAVETABLE_OSCILLATORS; i++) {
			oscillators[i].amp += 0.001f;
			oscillators[i].amp = ClampFloat32(oscillators[i].amp, 0.0f, 1.0f);
		}
	}
	if (IsKeyPressed(input, KM_KEY_H)) {
		for (int i = 0; i < WAVETABLE_OSCILLATORS; i++) {
			oscillators[i].amp -= 0.001f;
			oscillators[i].amp = ClampFloat32(oscillators[i].amp, 0.0f, 1.0f);
		}
	}
	float32 sampleOsc0 = oscillators[WAVETABLE_OSCILLATORS - 1].amp * LinearSample(audio,
		waves[0].buffer, bufferLengthSamples, 0, oscillators[WAVETABLE_OSCILLATORS - 1].tWave);
	tWaveTable = ClampFloat32(tWaveTable + sampleOsc0, 0.0f, 1.0f);

	for (int i = 0; i < activeVoices; i++) {
		float32 sampleOsc = oscillators[0].amp * LinearSample(audio,
			waves[0].buffer, bufferLengthSamples, 0, oscillators[0].tWave);
		voices[i].freq = voices[i].baseFreq * (1.0f + sampleOsc);
	}

	// Remove voices that were released and have faded out
	for (int i = 0; i < activeVoices; i++) {
		const EnvelopeADSR env = envelopes[voices[i].envelope];
		float32 t = voices[i].time;
		if (!voices[i].sustained) {
			// R envelope
			float32 elapsed = t - voices[i].releaseTime;
			// [0, +inf]
			float32 elapsedNorm = elapsed / env.release;
			if (elapsedNorm >= 1.0f) {
				// Remove voice
				for (int v = i; v < activeVoices - 1; v++) {
					voices[v] = voices[v + 1];
				}
				activeVoices--;
				i--;
				continue;
			}
		}
	}
}

void WaveTable::WriteSamples(GameAudio* audio)
{
	float32 indexWaveTable = tWaveTable * (numWaves - 1);
	float32 tMix = indexWaveTable - floorf(indexWaveTable);
	int wave1 = ClampInt((int)floorf(indexWaveTable), 0, numWaves - 1);
	int wave2 = ClampInt((int)ceilf(indexWaveTable), 0, numWaves - 1);

	const float32* wave1Buffer = waves[wave1].buffer;
	const float32* wave2Buffer = waves[wave2].buffer;
	for (int v = 0; v < activeVoices; v++) {
		const EnvelopeADSR env = envelopes[voices[v].envelope];
		for (int i = 0; i < audio->fillLength; i++) {
			float32 t = voices[v].time + (float32)i
				/ audio->sampleRate;
			if (voices[v].sustained) {
				// ADS envelopes
				float32 envAmp;
				if (t < env.attack) {
					envAmp = t / env.attack;
				}
				else if (t < env.attack + env.decay) {
					// [0, 1]
					envAmp = (t - env.attack) / env.decay;
					// [1, 0]
					envAmp = -envAmp + 1.0f;
					// [1, env.sustain]
					envAmp = envAmp * (1.0f - env.sustain) + env.sustain;
				}
				else {
					envAmp = env.sustain;
				}
				voices[v].amp = envAmp * voices[v].maxAmp;
			}
			else {
				// R envelope
				float32 elapsed = t - voices[v].releaseTime;
				// [0, +inf]
				float32 elapsedNorm = elapsed / env.release;
				if (elapsedNorm >= 1.0f) {
					continue;
				}
				else {
					// [releaseAmp, 0]
					voices[v].amp = (-elapsedNorm + 1.0f) * voices[v].releaseAmp;
				}
			}
			float32 tWave = fmod(voices[v].tWave + voices[v].freq * i / audio->sampleRate, 1.0f);
			float32 sample1Wave1 = LinearSample(audio, wave1Buffer, bufferLengthSamples, 0, tWave);
			float32 sample1Wave2 = LinearSample(audio, wave2Buffer, bufferLengthSamples, 0, tWave);
			float32 sample2Wave1 = LinearSample(audio, wave1Buffer, bufferLengthSamples, 1, tWave);
			float32 sample2Wave2 = LinearSample(audio, wave2Buffer, bufferLengthSamples, 1, tWave);

			audio->buffer[i * audio->channels] += voices[v].amp
				* Lerp(sample1Wave1, sample1Wave2, tMix);
			audio->buffer[i * audio->channels + 1] += voices[v].amp
				* Lerp(sample2Wave1, sample2Wave2, tMix);
		}
	}
}

void AudioState::Init(const ThreadContext* thread,
	GameAudio* audio, MemoryBlock* transient,
	DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
	DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory)
{
	globalMute = false;

	for (int c = 0; c < AUDIO_MAX_CHANNELS; c++) {
		lastSamplesRaw[c] = 0.0;
		lastSamplesFiltered[c] = 0.0;
	}

	const int KICK_VARIATIONS = 1;
	const char* kickSoundFiles[KICK_VARIATIONS] = {
		"data/audio/kick.wav"
	};
	const int SNARE_VARIATIONS = 1;
	const char* snareSoundFiles[SNARE_VARIATIONS] = {
		"data/audio/snare.wav"
	};
	const int DEATH_VARIATIONS = 1;
	const char* deathSoundFiles[DEATH_VARIATIONS] = {
		"data/audio/death.wav"
	};

	soundKick.Init(thread, audio,
		kickSoundFiles[0], *transient,
		DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
	soundSnare.Init(thread, audio,
		snareSoundFiles[0], *transient,
		DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
	soundDeath.Init(thread, audio,
		deathSoundFiles[0], *transient,
		DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);

	for (int i = 0; i < 12; i++) {
		char buf[128];
		sprintf(buf, "data/audio/note%d.wav", i);
		soundNotes[i].Init(thread, audio,
			buf, *transient,
			DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
	}

	waveTable.Init(audio);

#if GAME_INTERNAL
	debugView = false;
#endif
}

void OutputAudio(GameAudio* audio, GameState* gameState,
	const GameInput* input, MemoryBlock transient)
{
	DEBUG_ASSERT(audio->channels == 2); // Stereo support only
	AudioState& audioState = gameState->audioState;

	static float32 lastWrittenSample0, lastWrittenSample1;
	if (audio->sampleDelta > 0) {
		lastWrittenSample0 = audio->buffer[(audio->sampleDelta - 1) * audio->channels];
		lastWrittenSample1 = audio->buffer[(audio->sampleDelta - 1) * audio->channels + 1];
	}

	audioState.soundKick.Update(audio);
	audioState.soundSnare.Update(audio);
	audioState.soundDeath.Update(audio);
	for (int i = 0; i < 12; i++) {
		audioState.soundNotes[i].Update(audio);
	}

	audioState.waveTable.Update(audio, input);

	for (uint64 i = 0; i < audio->fillLength; i++) {
		audio->buffer[i * audio->channels] = 0.0f;
		audio->buffer[i * audio->channels + 1] = 0.0f;
	}

	if (gameState->audioState.globalMute) {
		return;
	}

	audioState.waveTable.WriteSamples(audio);

	// TODO wow, so many last minute decisions here
	audioState.soundKick.WriteSamples(1.0f, audio);
	audioState.soundSnare.WriteSamples(0.7f, audio);
	audioState.soundDeath.WriteSamples(0.5f, audio);
	for (int i = 0; i < 12; i++) {
		audioState.soundNotes[i].WriteSamples(0.2f, audio);
	}

	const uint64 lastSampleInd = (audio->fillLength - 1) * audio->channels;

	float32 lastSamplesRaw[AUDIO_MAX_CHANNELS];
	for (uint8 c = 0; c < audio->channels; c++) {
		lastSamplesRaw[c] = audio->buffer[lastSampleInd + c];
	}

	// float32 lowPassFrequency = MaxFloat32((float32)input->mousePos.x, 0.0f);
	// float32 a = 2.0f * PI_F * lowPassFrequency / (float32)audio->sampleRate;
	// float32 alpha = a / (a + 1.0f);
	// float32 prevSampleFiltered0 = audioState.lastSamplesFiltered[0];
	// float32 prevSampleFiltered1 = audioState.lastSamplesFiltered[1];
	// for (uint64 i = 0; i < audio->fillLength; i++) {
	// 	uint64 sampleInd = i * audio->channels;
	// 	audio->buffer[sampleInd]     *= alpha;
	// 	audio->buffer[sampleInd]     += (1.0f - alpha) * prevSampleFiltered0;
	// 	audio->buffer[sampleInd + 1] *= alpha;
	// 	audio->buffer[sampleInd + 1] += (1.0f - alpha) * prevSampleFiltered1;
	// 	prevSampleFiltered0 = audio->buffer[sampleInd];
	// 	prevSampleFiltered1 = audio->buffer[sampleInd + 1];
	// }

	// float32 highPassFrequency = MaxFloat32((float32)input->mousePos.x * 2.0f + 100.0f, 0.0f);
	// float32 a = 2.0f * PI_F * highPassFrequency / (float32)audio->sampleRate;
	// float32 alpha = ClampFloat32(1.0f / (a + 1.0f), 0.0f, 1.0f);
	// float32 prevSampleRaw0 = audioState.lastSamplesRaw[0];
	// float32 prevSampleRaw1 = audioState.lastSamplesRaw[1];
	// float32 prevSampleFiltered0 = audioState.lastSamplesFiltered[0];
	// float32 prevSampleFiltered1 = audioState.lastSamplesFiltered[1];
	// for (uint64 i = 0; i < audio->fillLength; i++) {
	// 	uint64 sampleInd = i * audio->channels;
	// 	float32 sampleRaw0 = audio->buffer[sampleInd];
	// 	float32 sampleRaw1 = audio->buffer[sampleInd + 1];
	// 	audio->buffer[sampleInd]     = alpha * (prevSampleFiltered0 + sampleRaw0 - prevSampleRaw0);
	// 	audio->buffer[sampleInd + 1] = alpha * (prevSampleFiltered1 + sampleRaw1 - prevSampleRaw1);
	// 	prevSampleRaw0 = sampleRaw0;
	// 	prevSampleRaw1 = sampleRaw1;
	// 	prevSampleFiltered0 = audio->buffer[sampleInd];
	// 	prevSampleFiltered1 = audio->buffer[sampleInd + 1];
	// }

	for (uint8 c = 0; c < audio->channels; c++) {
		audioState.lastSamplesRaw[c] = lastSamplesRaw[c];
		audioState.lastSamplesFiltered[c] = audio->buffer[lastSampleInd + c];
	}

	if (input->arduinoIn.connected) {
		float32 volume = input->arduinoIn.analogValues[0][1];
		for (uint64 i = 0; i < audio->fillLength; i++) {
			audio->buffer[i * audio->channels] *= volume;
			audio->buffer[i * audio->channels + 1] *= volume;
		}
	}

#if GAME_INTERNAL
	if (WasKeyPressed(input, KM_KEY_G)) {
		audioState.debugView = !audioState.debugView;
	}
#endif
}