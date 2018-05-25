#include "audio.h"

#include "main.h"
#include "km_debug.h"

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

internal void DrawAudioBuffer(
    const GameState* gameState, const GameAudio* audio,
    const float32* buffer, int bufferSizeSamples, int channel,
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
    for (int i = 0; i < bufferSizeSamples; i++) {
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

internal void SoundWriteSamples(const Sound* sound, float32 amplitude,
    GameAudio* audio)
{
    if (!sound->playing) {
        return;
    }

    const AudioBuffer* buffer = &sound->buffer;
    int samplesToWrite = audio->fillLength;
    if (sound->sampleIndex + samplesToWrite > buffer->bufferSizeSamples) {
        samplesToWrite = buffer->bufferSizeSamples - sound->sampleIndex;
    }
    for (int i = 0; i < samplesToWrite; i++) {
        int sampleInd = sound->sampleIndex + i;
        float32 sample1 = amplitude
            * buffer->buffer[sampleInd * audio->channels];
        float32 sample2 = amplitude
            * buffer->buffer[sampleInd * audio->channels + 1];

        audio->buffer[i * audio->channels] += sample1;
        audio->buffer[i * audio->channels + 1] += sample2;
    }
}

internal void WaveTableInit(const GameAudio* audio, WaveTable* waveTable)
{
    //waveTable->tWave = 0.0f;
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

    waveTable->numVoices = 12;
    for (int i = 0; i < waveTable->numVoices; i++) {
        waveTable->voices[i].t = 0.0f;
        waveTable->voices[i].active = false;
    }
    //waveTable->voices[1].active = true;

    waveTable->tOsc1 = 0.0f;
    waveTable->osc1Freq = 1.5f;
    waveTable->osc1Amp = 0.4f;

    waveTable->tOsc2 = 0.0f;
    waveTable->osc2Freq = 2.0f;
    waveTable->osc2Amp = 0.05f;

    // Very rough normalization
    // Meh, nevermind. Need a good loudness function for this.
    /*float32 loudness[WAVETABLE_MAX_WAVES];
    float32 minLoudness = 1e9;
    DEBUG_PRINT("loudness values:\n");
    for (int i = 0; i < waveTable->numWaves; i++) {
        loudness[i] = CalcWaveLoudness(audio,
            waveTable->waves[i].buffer, waveBufferLength);
        if (loudness[i] < minLoudness) {
            minLoudness = loudness[i];
        }
        DEBUG_PRINT("%d - %f\n", i, loudness[i]);
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
    /*waveTable->tWave += waveTable->waveFreq
        * audio->sampleDelta / audio->sampleRate;
    waveTable->tWave = fmod(waveTable->tWave, 1.0f);*/
    for (int i = 0; i < waveTable->numVoices; i++) {
        waveTable->voices[i].t += waveTable->voices[i].freq
            * audio->sampleDelta / audio->sampleRate;
        waveTable->voices[i].t = fmod(waveTable->voices[i].t, 1.0f);
    }
    waveTable->tOsc1 += waveTable->osc1Freq
        * audio->sampleDelta / audio->sampleRate;
    waveTable->tOsc1 = fmod(waveTable->tOsc1, 1.0f);
    waveTable->tOsc2 += waveTable->osc2Freq
        * audio->sampleDelta / audio->sampleRate;
    waveTable->tOsc2 = fmod(waveTable->tOsc2, 1.0f);

    /*float32 freqBase = 261.0f;
    float32 freqMin = freqBase * 1.0f;
    float32 freqMax = freqBase * 2.0f;
    float32 freqPixelRange = 1200.0f;
    float32 freqPixelOffset = 600.0f;
    float32 freqT = (input->mousePos.x - freqPixelOffset) / freqPixelRange;
    waveTable->waveFreq = Lerp(freqMin, freqMax, freqT);
    waveTable->waveAmp = 0.2f;*/

    float32 tWaveTablePixRange = 600.0f;
    float32 tWaveTablePixOffset = 200.0f;
    float32 tWaveTableT = (input->mousePos.y - tWaveTablePixOffset)
        / tWaveTablePixRange;
    waveTable->tWaveTable = Lerp(0.0f, 1.0f, tWaveTableT);
    waveTable->tWaveTable = ClampFloat32(waveTable->tWaveTable, 0.0f, 1.0f);

    if (IsKeyPressed(input, KM_KEY_Y)) {
        waveTable->osc1Freq += 0.01f;
    }
    if (IsKeyPressed(input, KM_KEY_H)) {
        waveTable->osc1Freq -= 0.01f;
    }
    if (IsKeyPressed(input, KM_KEY_T)) {
        waveTable->osc2Freq += 0.01f;
    }
    if (IsKeyPressed(input, KM_KEY_G)) {
        waveTable->osc2Freq -= 0.01f;
    }
    float32 sampleOsc1 = waveTable->osc1Amp * LinearSample(audio,
        waveTable->waves[0].buffer, waveTable->bufferLengthSamples,
        0, waveTable->tOsc1);
    float32 sampleOsc2 = waveTable->osc2Amp * LinearSample(audio,
        waveTable->waves[0].buffer, waveTable->bufferLengthSamples,
        0, waveTable->tOsc2);
    waveTable->tWaveTable = ClampFloat32(
        waveTable->tWaveTable + sampleOsc1,
        0.0f, 1.0f);

    float32 tonicFreq = 261.0f;
    float32 chromaticFreqs[12];
    float32 root12Two = powf(2.0f, 1.0f / 12.0f);
    for (int i = 0; i < 12; i++) {
        chromaticFreqs[i] = tonicFreq * powf(root12Two, (float32)i);
    }
    //chromaticFreqs[0] = tonicFreq * 1.0f;
    //chromaticFreqs[7] = tonicFreq * 3.0f / 2.0f;
    for (int i = 0; i < waveTable->numVoices; i++) {
        waveTable->voices[i].freq = chromaticFreqs[i];
        waveTable->voices[i].amp = 0.1f;
    }
    waveTable->voices[1].freq += waveTable->voices[1].freq * sampleOsc2;
    waveTable->voices[3].freq += waveTable->voices[3].freq * sampleOsc1;

    waveTable->voices[0].active = IsKeyPressed(input, KM_KEY_Z);
    waveTable->voices[2].active = IsKeyPressed(input, KM_KEY_X);
    waveTable->voices[4].active = IsKeyPressed(input, KM_KEY_C);
    waveTable->voices[5].active = IsKeyPressed(input, KM_KEY_V);
    waveTable->voices[7].active = IsKeyPressed(input, KM_KEY_B);
    waveTable->voices[9].active = IsKeyPressed(input, KM_KEY_N);
    waveTable->voices[11].active = IsKeyPressed(input, KM_KEY_M);
}

internal void WaveTableWriteSamples(WaveTable* waveTable,
    GameAudio* audio)
{
    float32 tWaveTable = waveTable->tWaveTable * (waveTable->numWaves - 1);
    float32 tMix = tWaveTable - floorf(tWaveTable);
    int wave1 = ClampInt((int)floorf(tWaveTable), 0, waveTable->numWaves - 1);
    int wave2 = ClampInt((int)ceilf(tWaveTable), 0, waveTable->numWaves - 1);

    int waveBufferLength = waveTable->bufferLengthSamples;
    const float32* wave1Buffer = waveTable->waves[wave1].buffer;
    const float32* wave2Buffer = waveTable->waves[wave2].buffer;
    for (int v = 0; v < waveTable->numVoices; v++) {
        if (!waveTable->voices[v].active) {
            continue;
        }
        for (int i = 0; i < audio->fillLength; i++) {
            // TODO oh boy, doing this here would be very hard...
            /*float32 tOsc1 = fmod(waveTable->tOsc1
                + waveTable->osc1Freq * i / audio->sampleRate, 1.0f);
            float32 sampleOsc1 = LinearSample(audio,
                waveTable->waves[0].buffer, waveBufferLength, 0, tOsc1);
            waveTable->waveFreq *= sampleOsc1;*/

            float32 tWave = fmod(waveTable->voices[v].t
                + waveTable->voices[v].freq * i / audio->sampleRate, 1.0f);
            float32 sample1Wave1 = LinearSample(audio,
                wave1Buffer, waveBufferLength, 0, tWave);
            float32 sample1Wave2 = LinearSample(audio,
                wave2Buffer, waveBufferLength, 0, tWave);
            float32 sample2Wave1 = LinearSample(audio,
                wave1Buffer, waveBufferLength, 1, tWave);
            float32 sample2Wave2 = LinearSample(audio,
                wave2Buffer, waveBufferLength, 1, tWave);

            audio->buffer[i * audio->channels] += waveTable->voices[v].amp
                * Lerp(sample1Wave1, sample1Wave2, tMix);
            audio->buffer[i * audio->channels + 1] += waveTable->voices[v].amp
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
    DEBUG_ASSERT(audio->sampleDelta >= 0);
    DEBUG_ASSERT(audio->channels == 2); // Stereo support only
    AudioState* audioState = &gameState->audioState;

    SoundUpdate(audio, &audioState->soundKick);
    SoundUpdate(audio, &audioState->soundSnare);
    SoundUpdate(audio, &audioState->soundDeath);
    for (int i = 0; i < 12; i++) {
        SoundUpdate(audio, &audioState->soundNotes[i]);
    }

    WaveTableUpdate(audio, input, &audioState->waveTable);

    for (int i = 0; i < audio->fillLength; i++) {
        audio->buffer[i * audio->channels] = 0.0f;
        audio->buffer[i * audio->channels + 1] = 0.0f;
    }

    if (gameState->audioState.globalMute) {
        return;
    }

    SoundWriteSamples(&audioState->soundKick, 1.0f, audio);
    SoundWriteSamples(&audioState->soundSnare, 0.7f, audio);
    SoundWriteSamples(&audioState->soundDeath, 0.5f, audio);
    for (int i = 0; i < 12; i++) {
        SoundWriteSamples(&audioState->soundNotes[i], 0.2f, audio);
    }

    WaveTableWriteSamples(&audioState->waveTable, audio);

#if GAME_INTERNAL
    if (WasKeyPressed(input, KM_KEY_G)) {
        audioState->debugView = !audioState->debugView;
    }
    if (audioState->debugView) {
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

        /*Vec4 waveColors[WAVETABLE_MAX_WAVES] = {
            Vec4 { 1.0f, 0.5f, 0.5f, 1.0f },
            Vec4 { 0.5f, 1.0f, 0.5f, 1.0f },
            Vec4 { 0.5f, 0.5f, 1.0f, 1.0f },
            Vec4 { 0.8f, 0.8f, 0.5f, 1.0f },
            Vec4 { 0.8f, 0.5f, 0.8f, 1.0f },
            Vec4 { 0.5f, 0.8f, 0.8f, 1.0f },
        };
        for (int i = 0; i < audioState->waveTable.numWaves; i++) {
            DrawAudioBuffer(gameState, audio,
                audioState->waveTable.waves[i].buffer,
                audioState->waveTable.bufferLengthSamples,
                0,
                nullptr, nullptr, 0,
                Vec3 { -1.0f, 0.0f, 0.0f }, Vec2 { 2.0f, 1.0f },
                waveColors[i],
                transient
            );
        }*/
        DrawAudioBuffer(gameState, audio,
            audioState->soundKick.buffer.buffer,
            audioState->soundKick.buffer.bufferSizeSamples,
            0,
            nullptr, nullptr, 0,
            Vec3 { -1.0f, 0.0f, 0.0f }, Vec2 { 2.0f, 1.0f },
            Vec4 { 0.5f, 0.7f, 0.8f, 1.0f },
            transient
        );
        DEBUG_PRINT("%d\n", audioState->soundKick.buffer.bufferSizeSamples);
    }
#endif
}