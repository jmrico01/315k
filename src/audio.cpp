#include "audio.h"

#include "main.h"
#include "km_debug.h"

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

internal float32 LinearSample(const GameAudio* audio,
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
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory)
{
    sound->play = false;
    sound->playing = false;
    sound->sampleIndex = 0;

    LoadWAV(thread, filePath,
        audio, &sound->buffer,
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
    waveTable->tWave = 0.0f;
    waveTable->tWaveTable = 0.0f;

    int waveBufferLength = WAVE_BUFFER_LENGTH_SECONDS * audio->sampleRate;
    waveTable->bufferLengthSamples = waveBufferLength;
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
    waveTable->tWave += waveTable->freq
        * audio->sampleDelta / audio->sampleRate;
    waveTable->tWave = fmod(waveTable->tWave, 1.0f);

    float32 freqBase = 261.0f;
    float32 freqMin = freqBase * 1.0f;
    float32 freqMax = freqBase * 2.0f;
    float32 freqPixelRange = 1200.0f;
    float32 freqPixelOffset = 600.0f;
    float32 freqT = (input->mousePos.x - freqPixelOffset) / freqPixelRange;
    waveTable->freq = Lerp(freqMin, freqMax, freqT);
    waveTable->amp = 0.2f;

    float32 tWaveTablePixRange = 600.0f;
    float32 tWaveTablePixOffset = 200.0f;
    float32 tWaveTableT = (input->mousePos.y - tWaveTablePixOffset)
        / tWaveTablePixRange;
    waveTable->tWaveTable = Lerp(0.0f, 1.0f, tWaveTableT);
    waveTable->tWaveTable = ClampFloat32(waveTable->tWaveTable, 0.0f, 1.0f);
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
    for (int i = 0; i < audio->fillLength; i++) {
        float32 t = fmod(waveTable->tWave
            + waveTable->freq * i / audio->sampleRate, 1.0f);
        float32 sample1Wave1 = waveTable->amp * LinearSample(audio,
            wave1Buffer, waveBufferLength, 0, t);
        float32 sample1Wave2 = waveTable->amp * LinearSample(audio,
            wave2Buffer, waveBufferLength, 0, t);
        float32 sample2Wave1 = waveTable->amp * LinearSample(audio,
            wave1Buffer, waveBufferLength, 1, t);
        float32 sample2Wave2 = waveTable->amp * LinearSample(audio,
            wave2Buffer, waveBufferLength, 1, t);

        audio->buffer[i * audio->channels] += Lerp(
            sample1Wave1, sample1Wave2, tMix);
        audio->buffer[i * audio->channels + 1] += Lerp(
            sample2Wave1, sample2Wave2, tMix);
    }
}

void InitAudioState(const ThreadContext* thread,
    AudioState* audioState, GameAudio* audio,
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
        DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
    SoundInit(thread, audio,
        &audioState->soundSnare,
        snareSoundFiles[0],
        DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
    SoundInit(thread, audio,
        &audioState->soundDeath,
        deathSoundFiles[0],
        DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);

    for (int i = 0; i < 12; i++) {
        char buf[128];
        sprintf(buf, "data/audio/note%d.wav", i);
        SoundInit(thread, audio,
            &audioState->soundNotes[i],
            buf,
            DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
    }

    audioState->tWave = 0.0f;

    WaveTableInit(audio, &audioState->waveTable);

#if GAME_INTERNAL
    audioState->debugView = false;
#endif
}

void OutputAudio(GameAudio* audio, GameState* gameState,
    const GameInput* input, MemoryBlock transient)
{
    DEBUG_ASSERT(audio->sampleDelta >= 0);
    AudioState* audioState = &gameState->audioState;
    // Advance tWave based on previous toneWave frequency
    audioState->tWave += 2.0f * PI_F * audioState->toneWave
        * audio->sampleDelta / audio->sampleRate;

    float32 ampWave = 0.15f;
    float32 toneWaveBase = 261.0f;
    float32 toneMin = toneWaveBase * 1.0f;
    float32 toneMax = toneWaveBase * 2.0f;
    float32 tonePixelRange = 600.0f;
    float32 tonePixelOffset = 600.0f;
    float32 toneT = (input->mousePos.x - tonePixelOffset) / tonePixelRange;
    audioState->toneWave = Lerp(toneMin, toneMax, toneT);

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

    /*for (int i = 0; i < audio->fillLength; i++) {
        float32 tWaveOff = 2.0f * PI_F * audioState->toneWave
            * i / audio->sampleRate;

        float32 waveSample = 0.0f;
        if (input->keyboard[KM_KEY_Z].isDown) {
            waveSample += ampWave * sinf(audioState->tWave + tWaveOff);
        }
        if (input->keyboard[KM_KEY_X].isDown) {
            float32 amp = ampWave * 0.9f;
            waveSample += amp * TriangleWave(audioState->tWave + tWaveOff);
        }
        if (input->keyboard[KM_KEY_C].isDown) {
            float32 amp = ampWave * 0.3f;
            waveSample += amp * SawtoothWave(audioState->tWave + tWaveOff);
        }
        if (input->keyboard[KM_KEY_V].isDown) {
            float32 amp = ampWave * 0.3f;
            waveSample += amp * SquareWave(audioState->tWave + tWaveOff);
        }
        audio->buffer[i * audio->channels]      += waveSample;
        audio->buffer[i * audio->channels + 1]  += waveSample;
    }*/

    SoundWriteSamples(&audioState->soundKick, 1.0f, audio);
    SoundWriteSamples(&audioState->soundSnare, 0.7f, audio);
    SoundWriteSamples(&audioState->soundDeath, 0.5f, audio);
    for (int i = 0; i < 12; i++) {
        SoundWriteSamples(&audioState->soundNotes[i], 0.2f, audio);
    }

    if (input->keyboard[KM_KEY_Z].isDown) {
        WaveTableWriteSamples(&audioState->waveTable, audio);
    }

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

        DrawAudioBuffer(gameState, audio,
            audioState->waveTable.waves[0].buffer,
            audioState->waveTable.bufferLengthSamples,
            0,
            nullptr, nullptr, 0,
            Vec3 { -1.0f, 0.0f, 0.0f }, Vec2 { 2.0f, 1.0f },
            Vec4 { 1.0f, 0.5f, 0.5f, 1.0f },
            transient
        );

        DrawAudioBuffer(gameState, audio,
            audioState->waveTable.waves[1].buffer,
            audioState->waveTable.bufferLengthSamples,
            0,
            nullptr, nullptr, 0,
            Vec3 { -1.0f, 0.0f, 0.0f }, Vec2 { 2.0f, 1.0f },
            Vec4 { 0.5f, 1.0f, 0.5f, 1.0f },
            transient
        );

        DrawAudioBuffer(gameState, audio,
            audioState->waveTable.waves[2].buffer,
            audioState->waveTable.bufferLengthSamples,
            0,
            nullptr, nullptr, 0,
            Vec3 { -1.0f, 0.0f, 0.0f }, Vec2 { 2.0f, 1.0f },
            Vec4 { 0.5f, 0.5f, 1.0f, 1.0f },
            transient
        );

        DrawAudioBuffer(gameState, audio,
            audioState->waveTable.waves[3].buffer,
            audioState->waveTable.bufferLengthSamples,
            0,
            nullptr, nullptr, 0,
            Vec3 { -1.0f, 0.0f, 0.0f }, Vec2 { 2.0f, 1.0f },
            Vec4 { 0.5f, 0.8f, 0.8f, 1.0f },
            transient
        );
    }
#endif
}