#include "audio.h"

#include "main.h"
#include "km_debug.h"

#define MIDI_EVENT_NOTEON 0x9
#define MIDI_EVENT_NOTEOFF 0x8

#if 0
internal float32 CalcWaveLoudness(const GameAudio* audio,
    const float32* buffer, int bufferLengthSamples)
{
    float32 loudness = 0.0f;
    float32 prevVal1 = buffer[0];
    float32 prevVal2 = buffer[1];
    for (int i = 1; i < bufferLengthSamples; i++) {
        float32 val1 = buffer[i * audio->channels];
        float32 val2 = buffer[i * audio->channels + 1];
        loudness += (val1 - prevVal1) * (val1 - prevVal1)
            + (val2 - prevVal2) * (val2 - prevVal2);
        prevVal1 = val1;
        prevVal2 = val2;
    }

    return loudness;
}
#endif

internal inline float32 MidiNoteToFreq(int midiNote)
{
    return 261.625565f * powf(2.0f, (midiNote - 60.0f) / 12.0f);
}

internal void DrawAudioBuffer(
    const GameState* gameState, const GameAudio* audio,
    const float32* buffer, uint64 bufferSizeSamples, uint8 channel,
    const int marks[], const Vec4 markColors[], int numMarks,
    Vec3 origin, Vec2 size, Vec4 color,
    MemoryBlock transient)
{
    DEBUG_ASSERT(transient.size >= sizeof(LineGLData));
    DEBUG_ASSERT(bufferSizeSamples <= MAX_LINE_POINTS);
    LineGLData* lineData = (LineGLData*)transient.memory;
    Mat4 proj = Mat4::one;
    Vec3 zoomScale = {
        1.0f + gameState->debugZoom,
        1.0f + gameState->debugZoom,
        1.0f
    };
    Mat4 view = Scale(zoomScale) * Translate(gameState->debugCamPos);
    
    lineData->count = bufferSizeSamples;
    for (uint64 i = 0; i < bufferSizeSamples; i++) {
        float32 val = buffer[i * audio->channels + channel];
        float32 t = (float32)i / (bufferSizeSamples - 1);
        lineData->pos[i] = {
            origin.x + t * size.x,
            origin.y + size.y * val,
            origin.z
        };
    }
    DrawLine(gameState->lineGL, proj, view,
        lineData, color);

    lineData->count = 2;
    for (int i = 0; i < numMarks; i++) {
        float32 tMark = (float32)marks[i] / (bufferSizeSamples - 1);
        lineData->pos[0] = Vec3 {
            origin.x + tMark * size.x,
            origin.y,
            origin.z
        };
        lineData->pos[1] = Vec3 {
            origin.x + tMark * size.x,
            origin.y + size.y,
            origin.z
        };
        DrawLine(gameState->lineGL, proj, view,
            lineData, markColors[i]);
    }
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

internal void SoundInit(const ThreadContext* thread,
    const GameAudio* audio, Sound* sound,
    const char* filePath,
    MemoryBlock* transient,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory)
{
    sound->play = false;
    sound->playing = false;
    sound->sampleIndex = 0;

    LoadWAV(thread, filePath,
        audio, &sound->buffer,
        transient,
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

internal void SoundUpdate(const GameAudio* audio, Sound* sound)
{
    if (sound->playing) {
        sound->sampleIndex += audio->sampleDelta;
        if (sound->sampleIndex >= sound->buffer.bufferSizeSamples) {
            sound->playing = false;
        }
    }
    if (sound->play) {
        sound->play = false;
        sound->playing = true;
        sound->sampleIndex = 0;
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

internal void WaveTableInit(const GameAudio* audio, WaveTable* waveTable)
{
    waveTable->tWaveTable = 0.0f;

    int waveBufferLength = WAVE_BUFFER_LENGTH_SECONDS * audio->sampleRate;
    waveTable->bufferLengthSamples = waveBufferLength;
    // Instance-specific wave table parameters
    // Ideally there would be some way to feed these in
    waveTable->numWaves = 4;
    for (int i = 0; i < waveBufferLength; i++) {
        float32 t = 2.0f * PI_F * (float32)i / audio->sampleRate;
        int ind1 = i * audio->channels;
        int ind2 = i * audio->channels + 1;
        waveTable->waves[0].buffer[ind1] = sinf(t);
        waveTable->waves[0].buffer[ind2] = sinf(t);
        waveTable->waves[1].buffer[ind1] = 0.9f * TriangleWave(t);
        waveTable->waves[1].buffer[ind2] = 0.9f * TriangleWave(t);
        waveTable->waves[2].buffer[ind1] = 0.3f * SawtoothWave(t);
        waveTable->waves[2].buffer[ind2] = 0.3f * SawtoothWave(t);
        waveTable->waves[3].buffer[ind1] = 0.3f * SquareWave(t);
        waveTable->waves[3].buffer[ind2] = 0.3f * SquareWave(t);
    }

    waveTable->activeVoices = 0;

    for (int i = 0; i < WAVETABLE_OSCILLATORS; i++) {
        waveTable->oscillators[i].tWave = 0.0f;
        waveTable->oscillators[i].freq = 0.0f;
        waveTable->oscillators[i].amp = 0.0f;
    }
    for (int i = 0; i < WAVETABLE_OSCILLATORS; i++) {
        waveTable->oscillators[i].freq = RandFloat32(0.2f, 2.0f);
        waveTable->oscillators[i].amp = 0.0f;
    }

    waveTable->envelopes[0].attack = 0.05f;
    waveTable->envelopes[0].decay = 0.05f;
    waveTable->envelopes[0].sustain = 0.8f;
    waveTable->envelopes[0].release = 0.1f;

    // Very rough normalization
    // Meh, nevermind. Need a good loudness function for this.
    /*float32 loudness[WAVETABLE_MAX_WAVES];
    float32 minLoudness = 1e9;
    LOG_INFO("loudness values:\n");
    for (int i = 0; i < waveTable->numWaves; i++) {
        loudness[i] = CalcWaveLoudness(audio,
            waveTable->waves[i].buffer, waveBufferLength);
        if (loudness[i] < minLoudness) {
            minLoudness = loudness[i];
        }
        LOG_INFO("%d - %f\n", i, loudness[i]);
    }
    for (int w = 0; w < waveTable->numWaves; w++) {
        for (int i = 0; i < waveBufferLength; i++) {
            int ind1 = i * audio->channels;
            int ind2 = i * audio->channels + 1;
            waveTable->waves[w].buffer[ind1] *= minLoudness / loudness[w];
            waveTable->waves[w].buffer[ind2] *= minLoudness / loudness[w];
        }
    }*/
}

internal void WaveTableUpdate(const GameAudio* audio, const GameInput* input,
    WaveTable* waveTable)
{
    for (int i = 0; i < waveTable->activeVoices; i++) {
        waveTable->voices[i].time += (float32)audio->sampleDelta /
            audio->sampleRate;
        waveTable->voices[i].tWave += waveTable->voices[i].freq
            * audio->sampleDelta / audio->sampleRate;
        waveTable->voices[i].tWave = fmod(waveTable->voices[i].tWave, 1.0f);
    }
    for (int i = 0; i < WAVETABLE_OSCILLATORS; i++) {
        waveTable->oscillators[i].tWave += waveTable->oscillators[i].freq
            * audio->sampleDelta / audio->sampleRate;
        waveTable->oscillators[i].tWave = 
            fmod(waveTable->oscillators[i].tWave, 1.0f);
    }

    float32 tWaveTablePixRange = 600.0f;
    float32 tWaveTablePixOffset = 200.0f;
    float32 tWaveTableT = (input->mousePos.y - tWaveTablePixOffset)
        / tWaveTablePixRange;
    waveTable->tWaveTable = Lerp(0.0f, 1.0f, tWaveTableT);
    waveTable->tWaveTable = ClampFloat32(waveTable->tWaveTable, 0.0f, 1.0f);

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
                bool32 overwroteVoice = false;
                for (int v = 0; v < waveTable->activeVoices; v++) {
                    if (waveTable->voices[v].midiNote == midiNote) {
                        vInd = v;
                        overwroteVoice = true;
                        /*waveTable->voices[v].time = 0.0f;
                        waveTable->voices[v].baseFreq = MidiNoteToFreq(midiNote);
                        waveTable->voices[v].maxAmp = 0.2f;
                        waveTable->voices[v].sustained = true;
                        overwroteVoice = true;*/
                        break;
                    }
                }
                if (!overwroteVoice) {
                    // No existing voice, must add a new one
                    if (waveTable->activeVoices >= WAVETABLE_MAX_VOICES) {
                        continue;
                    }
                    vInd = waveTable->activeVoices;
                }

                waveTable->voices[vInd].time = 0.0f;
                waveTable->voices[vInd].baseFreq = MidiNoteToFreq(midiNote);
                waveTable->voices[vInd].maxAmp = 0.2f;
                waveTable->voices[vInd].midiNote = midiNote;
                waveTable->voices[vInd].sustained = true;
                waveTable->voices[vInd].envelope = 0;
                waveTable->activeVoices++;
            } break;
            case MIDI_EVENT_NOTEOFF: {
                int midiNote = (int)dataByte1;
                if (waveTable->activeVoices <= 0) {
                    continue;
                }
                for (int v = 0; v < waveTable->activeVoices; v++) {
                    if (waveTable->voices[v].midiNote == midiNote) {
                        waveTable->voices[v].sustained = false;
                        waveTable->voices[v].releaseTime =
                            waveTable->voices[v].time;
                        waveTable->voices[v].releaseAmp =
                            waveTable->voices[v].amp;
                    }
                }
            } break;
        }
    }

    // Apply oscillators
    if (IsKeyPressed(input, KM_KEY_Y)) {
        for (int i = 0; i < WAVETABLE_OSCILLATORS; i++) {
            waveTable->oscillators[i].amp += 0.001f;
            waveTable->oscillators[i].amp =
                ClampFloat32(waveTable->oscillators[i].amp, 0.0f, 1.0f);
        }
    }
    if (IsKeyPressed(input, KM_KEY_H)) {
        for (int i = 0; i < WAVETABLE_OSCILLATORS; i++) {
            waveTable->oscillators[i].amp -= 0.001f;
            waveTable->oscillators[i].amp =
                ClampFloat32(waveTable->oscillators[i].amp, 0.0f, 1.0f);
        }
    }
    float32 sampleOsc0 = waveTable->oscillators[0].amp * LinearSample(audio,
        waveTable->waves[0].buffer, waveTable->bufferLengthSamples,
        0, waveTable->oscillators[0].tWave);
    waveTable->tWaveTable = ClampFloat32(
        waveTable->tWaveTable + sampleOsc0,
        0.0f, 1.0f);

    for (int i = 0; i < waveTable->activeVoices; i++) {
        float32 sampleOsc = waveTable->oscillators[i].amp * LinearSample(
            audio,
            waveTable->waves[0].buffer, waveTable->bufferLengthSamples,
            0, waveTable->oscillators[i].tWave);
        waveTable->voices[i].freq = waveTable->voices[i].baseFreq *
            (1.0f + sampleOsc);
    }

    // Remove voices that were released and have faded out
    for (int i = 0; i < waveTable->activeVoices; i++) {
        const EnvelopeADSR env = waveTable->envelopes[
            waveTable->voices[i].envelope];
        float32 t = waveTable->voices[i].time;
        if (!waveTable->voices[i].sustained) {
            // R envelope
            float32 elapsed = t - waveTable->voices[i].releaseTime;
            // [0, +inf]
            float32 elapsedNorm = elapsed / env.release;
            if (elapsedNorm >= 1.0f) {
                // Remove voice
                for (int v = i; v < waveTable->activeVoices - 1; v++) {
                    waveTable->voices[v] = waveTable->voices[v + 1];
                }
                waveTable->activeVoices--;
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

    int waveBufferLength = bufferLengthSamples;
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
            float32 sample1Wave1 = LinearSample(audio, wave1Buffer, waveBufferLength, 0, tWave);
            float32 sample1Wave2 = LinearSample(audio, wave2Buffer, waveBufferLength, 0, tWave);
            float32 sample2Wave1 = LinearSample(audio, wave1Buffer, waveBufferLength, 1, tWave);
            float32 sample2Wave2 = LinearSample(audio, wave2Buffer, waveBufferLength, 1, tWave);

            audio->buffer[i * audio->channels] += voices[v].amp
                * Lerp(sample1Wave1, sample1Wave2, tMix);
            audio->buffer[i * audio->channels + 1] += voices[v].amp
                * Lerp(sample2Wave1, sample2Wave2, tMix);
        }
    }
}

void InitAudioState(const ThreadContext* thread,
    AudioState* audioState, GameAudio* audio,
    MemoryBlock* transient,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory)
{
    audioState->globalMute = false;

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

    SoundInit(thread, audio,
        &audioState->soundKick,
        kickSoundFiles[0],
        transient,
        DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
    SoundInit(thread, audio,
        &audioState->soundSnare,
        snareSoundFiles[0],
        transient,
        DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
    SoundInit(thread, audio,
        &audioState->soundDeath,
        deathSoundFiles[0],
        transient,
        DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);

    for (int i = 0; i < 12; i++) {
        char buf[128];
        sprintf(buf, "data/audio/note%d.wav", i);
        SoundInit(thread, audio,
            &audioState->soundNotes[i],
            buf,
            transient,
            DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
    }

    WaveTableInit(audio, &audioState->waveTable);

#if GAME_INTERNAL
    audioState->debugView = false;
#endif
}

void OutputAudio(GameAudio* audio, GameState* gameState,
    const GameInput* input, MemoryBlock transient)
{
    DEBUG_ASSERT(audio->channels == 2); // Stereo support only
    AudioState& audioState = gameState->audioState;

#if GAME_INTERNAL
    // DEBUG recording logic
    if (WasKeyPressed(input, KM_KEY_R)) {
        audioState.debugRecording = !audioState.debugRecording;
        if (audioState.debugRecording) {
            audioState.debugBufferSamples = 0;
        }
    }
    if (WasKeyPressed(input, KM_KEY_V)) {
        audioState.debugViewRecording = !audioState.debugViewRecording;
    }
    if (audioState.debugRecording) {
        if (audio->sampleDelta > 0
        && (audioState.debugBufferSamples + audio->sampleDelta) * audio->channels <= DEBUG_BUFFER_SAMPLES) {
            MemCopy(audioState.debugBuffer + audioState.debugBufferSamples * audio->channels,
                audio->buffer, audio->sampleDelta * audio->channels * sizeof(float32));
            audioState.debugBufferSamples += audio->sampleDelta;
        }
    }
    if (audioState.debugViewRecording) {
        uint64 samples = MinUInt64(audioState.debugBufferSamples, MAX_LINE_POINTS);
        DrawAudioBuffer(gameState, audio,
            audioState.debugBuffer, samples, 0,
            nullptr, nullptr, 0,
            Vec3 { -1.0f, 0.5f, 0.0f }, Vec2 { 2.0f, 1.0f },
            Vec4::one,
            transient
        );
        DrawAudioBuffer(gameState, audio,
            audioState.debugBuffer, samples, 1,
            nullptr, nullptr, 0,
            Vec3 { -1.0f, -0.5f, 0.0f }, Vec2 { 2.0f, 1.0f },
            Vec4::one,
            transient
        );
    }
#endif

    audioState.soundKick.Update(audio);
    audioState.soundSnare.Update(audio);
    audioState.soundDeath.Update(audio);
    for (int i = 0; i < 12; i++) {
        audioState.soundNotes[i].Update(audio);
    }

    WaveTableUpdate(audio, input, &audioState.waveTable);

    for (uint64 i = 0; i < audio->fillLength; i++) {
        audio->buffer[i * audio->channels] = 0.0f;
        audio->buffer[i * audio->channels + 1] = 0.0f;
    }

    if (gameState->audioState.globalMute) {
        return;
    }

    // TODO wow, so many last minute decisions here
    audioState.soundKick.WriteSamples(1.0f, audio);
    audioState.soundSnare.WriteSamples(0.7f, audio);
    audioState.soundDeath.WriteSamples(0.5f, audio);
    for (int i = 0; i < 12; i++) {
        audioState.soundNotes[i].WriteSamples(0.2f, audio);
    }

    audioState.waveTable.WriteSamples(audio);

    float32 lowPassFrequency = MaxFloat32((float32)input->mousePos.x, 0.0f);
    float32 a = 2.0f * PI_F * lowPassFrequency / (float32)audio->sampleRate;
    float32 alpha = a / (a + 1.0f);
    for (uint64 i = 1; i < audio->fillLength; i++) {
        audio->buffer[i * audio->channels] *= alpha;
        audio->buffer[i * audio->channels] += (1.0f - alpha)
            * audio->buffer[(i - 1) * audio->channels];
        audio->buffer[i * audio->channels + 1] *= alpha;
        audio->buffer[i * audio->channels + 1] += (1.0f - alpha)
            * audio->buffer[(i - 1) * audio->channels + 1];
    }

    // float32 highPassFrequency = MaxFloat32((float32)input->mousePos.x * 2.0f + 100.0f, 0.0f);
    // float32 a = 2.0f * PI_F * highPassFrequency / (float32)audio->sampleRate;
    // float32 alpha = 1.0f / (a + 1.0f);
    // float32 prevChannel0 = audio->buffer[0];
    // float32 prevChannel1 = audio->buffer[1];
    // for (uint64 i = 1; i < audio->fillLength; i++) {
    //     float32 channel0 = audio->buffer[i * audio->channels];
    //     audio->buffer[i * audio->channels] = alpha * (audio->buffer[(i - 1) * audio->channels] + channel0 - prevChannel0);
    //     prevChannel0 = channel0;
    //     float32 channel1 = audio->buffer[i * audio->channels + 1];
    //     audio->buffer[i * audio->channels + 1] = alpha * (audio->buffer[(i - 1) * audio->channels + 1] + channel1 - prevChannel1);
    //     prevChannel1 = channel1;
    // }

#if GAME_INTERNAL
    if (WasKeyPressed(input, KM_KEY_G)) {
        audioState.debugView = !audioState.debugView;
    }
    if (audioState.debugView) {
        DrawAudioBuffer(gameState, audio,
            audio->buffer, audio->fillLength, 0,
            nullptr, nullptr, 0,
            Vec3 { -1.0f, 0.5f, 0.0f }, Vec2 { 2.0f, 1.0f },
            Vec4::one,
            transient
        );
        DrawAudioBuffer(gameState, audio,
            audio->buffer, audio->fillLength, 1,
            nullptr, nullptr, 0,
            Vec3 { -1.0f, -0.5f, 0.0f }, Vec2 { 2.0f, 1.0f },
            Vec4::one,
            transient
        );
    }
#endif
}