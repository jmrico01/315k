#include "audio.h"

#include "main.h"
#include "km_debug.h"

internal float32 SquareWave(float32 t)
{
    float32 tMod = fmod(t, 2.0f * PI_F);
    return tMod < PI_F ? 1.0f : -1.0f;
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
    int variations, const char* filePaths[],
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory)
{
    DEBUG_ASSERT(variations < SOUND_MAX_VARIATIONS);
    sound->play = false;
    sound->playing = false;
    sound->sampleIndex = 0;
    sound->variations = variations;

    for (int i = 0; i < variations; i++) {
        LoadWAV(thread, filePaths[i],
            audio, &sound->buffers[i],
            DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
    }
    sound->activeVariation = 0;
}

internal void SoundUpdate(const GameAudio* audio, Sound* sound)
{
    if (sound->playing) {
        sound->sampleIndex += audio->fillStartDelta;
        if (sound->sampleIndex
        >= sound->buffers[sound->activeVariation].bufferSizeSamples) {
            sound->playing = false;
        }
    }
    if (sound->play) {
        sound->play = false;
        sound->playing = true;
        sound->sampleIndex = 0;
        sound->activeVariation = rand() % sound->variations;
    }
}

internal void SoundWriteSamples(const Sound* sound, float32 amplitude,
    GameAudio* audio)
{
    if (!sound->playing) {
        return;
    }

    const AudioBuffer* activeBuffer = &sound->buffers[sound->activeVariation];
    for (int i = 0; i < audio->fillLength; i++) {
        int sampleInd = sound->sampleIndex + i;
        if (sampleInd >= activeBuffer->bufferSizeSamples) {
            break;
        }
        int16 sample1 = (int16)(amplitude
            * activeBuffer->buffer[sampleInd * audio->channels]);
        int16 sample2 = (int16)(amplitude
            * activeBuffer->buffer[sampleInd * audio->channels + 1]);

        int ind = (audio->fillStart + i) % audio->bufferSizeSamples;
        audio->buffer[ind * audio->channels] += sample1;
        audio->buffer[ind * audio->channels + 1] += sample2;
    }
}

void InitAudioState(const ThreadContext* thread,
    AudioState* audioState, GameAudio* audio,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory)
{
    audioState->runningSampleIndex = 0;
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
        KICK_VARIATIONS, kickSoundFiles,
        DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
    SoundInit(thread, audio,
        &audioState->soundSnare,
        SNARE_VARIATIONS, snareSoundFiles,
        DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
    SoundInit(thread, audio,
        &audioState->soundDeath,
        DEATH_VARIATIONS, deathSoundFiles,
        DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);

    for (int i = 0; i < 12; i++) {
        char buf[128];
        sprintf(buf, "data/audio/note%d.wav", i);
        const char* noteSoundFiles[1] = {
            buf
        };
        SoundInit(thread, audio,
            &audioState->soundNotes[i],
            1, noteSoundFiles,
            DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
    }

    audioState->tWave = 0.0f;

#if GAME_INTERNAL
    audioState->debugView = false;
#endif
}

void OutputAudio(GameAudio* audio, GameState* gameState,
    const GameInput* input, MemoryBlock transient)
{
    float32 ampWave = 0.15f;
    float32 toneWave = 261.0f;

    // Modify wave tone based on mouse position
    float32 toneMin = toneWave * 1.0f;
    float32 toneMax = toneWave * 2.0f;
    float32 tonePixelRange = 600.0f;
    float32 tonePixelOffset = 600.0f;
    float32 toneT = (input->mousePos.x - tonePixelOffset) / tonePixelRange;
    toneWave = Lerp(toneMin, toneMax, toneT);

    DEBUG_ASSERT(audio->fillStartDelta >= 0);
    AudioState* audioState = &gameState->audioState;
    audioState->runningSampleIndex += audio->fillStartDelta;
    audioState->tWave += 2.0f * PI_F * toneWave
        * audio->fillStartDelta / audio->sampleRate;

    SoundUpdate(audio, &audioState->soundKick);
    SoundUpdate(audio, &audioState->soundSnare);
    SoundUpdate(audio, &audioState->soundDeath);
    for (int i = 0; i < 12; i++) {
        SoundUpdate(audio, &audioState->soundNotes[i]);
    }

    for (int i = 0; i < audio->fillLength; i++) {
        int ind = (audio->fillStart + i) % audio->bufferSizeSamples;
        audio->buffer[ind * audio->channels] = 0;
        audio->buffer[ind * audio->channels + 1] = 0;
    }

    if (gameState->audioState.globalMute) {
        return;
    }

    for (int i = 0; i < audio->fillLength; i++) {
        int ind = (audio->fillStart + i) % audio->bufferSizeSamples;
        float32 tWaveOff = 2.0f * PI_F * toneWave
            * i / audio->sampleRate;
        /*float32 ampBass = Lerp(ampBassStart, ampBassEnd,
            t / soundRequestLength);*/
        int16 waveSample = 0;
        if (input->keyboard[KM_KEY_Z].isDown) {
            waveSample = (int16)(INT16_MAXVAL * ampWave * sinf(
                audioState->tWave + tWaveOff));
        }
        else if (input->keyboard[KM_KEY_X].isDown) {
            float32 ampDamp = ampWave * 0.9f;
            waveSample = (int16)(INT16_MAXVAL * ampDamp * TriangleWave(
                audioState->tWave + tWaveOff));
        }
        else if (input->keyboard[KM_KEY_C].isDown) {
            float32 ampDamp = ampWave * 0.3f;
            waveSample = (int16)(INT16_MAXVAL * ampDamp * SquareWave(
                audioState->tWave + tWaveOff));
        }
        audio->buffer[ind * audio->channels]      += waveSample;
        audio->buffer[ind * audio->channels + 1]  += waveSample;
    }

    SoundWriteSamples(&audioState->soundKick, 1.0f, audio);
    SoundWriteSamples(&audioState->soundSnare, 0.7f, audio);
    SoundWriteSamples(&audioState->soundDeath, 0.5f, audio);
    for (int i = 0; i < 12; i++) {
        SoundWriteSamples(&audioState->soundNotes[i], 0.2f, audio);
    }

#if GAME_INTERNAL
    if (WasKeyPressed(input, KM_KEY_G)) {
        audioState->debugView = !audioState->debugView;
    }
    if (audioState->debugView) {
        // TODO pass only transient block to this function in the 1st place
        DEBUG_ASSERT(transient.size >= sizeof(LineGLData));
        DEBUG_ASSERT(audio->bufferSizeSamples <= MAX_LINE_POINTS);
        Mat4 proj = Mat4::one;
        Vec3 zoomScale = {
            1.0f + gameState->debugZoom,
            1.0f + gameState->debugZoom,
            1.0f
        };
        Mat4 view = Scale(zoomScale) * Translate(gameState->debugCamPos);

        LineGLData* lineData = (LineGLData*)transient.memory;
        float32 length = 2.0f;
        float32 amplitude = 1.0f;
        float32 offset = 1.0f;
        
        lineData->count = audio->bufferSizeSamples;
        for (int i = 0; i < audio->bufferSizeSamples; i++) {
            int16 val = audio->buffer[i * audio->channels];
            float32 normVal = (float32)val / INT16_MAXVAL;
            float32 t = (float32)i / (audio->bufferSizeSamples - 1);
            lineData->pos[i] = {
                t * length - length / 2.0f,
                amplitude * normVal + offset,
                0.0f
            };
        }
        DrawLine(gameState->lineGL, proj, view,
            lineData, Vec4::one);
        for (int i = 0; i < audio->bufferSizeSamples; i++) {
            int16 val = audio->buffer[i * audio->channels + 1];
            float32 normVal = (float32)val / INT16_MAXVAL;
            float32 t = (float32)i / (audio->bufferSizeSamples - 1);
            lineData->pos[i] = {
                t * length - length / 2.0f,
                amplitude * normVal - offset,
                0.0f
            };
        }
        DrawLine(gameState->lineGL, proj, view,
            lineData, Vec4::one);

        lineData->count = 2;
        int prevPlayMark = (audio->playMarker - audio->fillStartDelta)
            % audio->bufferSizeSamples;
        float32 tPrevPlayMark = (float32)prevPlayMark
            / (audio->bufferSizeSamples - 1);
        lineData->pos[0] = Vec3 {
            tPrevPlayMark * length - length / 2.0f,
            amplitude + offset,
            0.0f
        };
        lineData->pos[1] = Vec3 {
            tPrevPlayMark * length - length / 2.0f,
            -(amplitude + offset),
            0.0f
        };
        DrawLine(gameState->lineGL, proj, view,
            lineData, Vec4 { 1.0f, 0.5f, 0.5f, 1.0f });
        float32 tPlayMark = (float32)audio->playMarker
            / (audio->bufferSizeSamples - 1);
        lineData->pos[0] = Vec3 {
            tPlayMark * length - length / 2.0f,
            amplitude + offset,
            0.0f
        };
        lineData->pos[1] = Vec3 {
            tPlayMark * length - length / 2.0f,
            -(amplitude + offset),
            0.0f
        };
        DrawLine(gameState->lineGL, proj, view,
            lineData, Vec4::red);
        float32 tFillStart = (float32)audio->fillStart
            / (audio->bufferSizeSamples - 1);
        lineData->pos[0] = Vec3 {
            tFillStart * length - length / 2.0f,
            amplitude + offset,
            0.0f
        };
        lineData->pos[1] = Vec3 {
            tFillStart * length - length / 2.0f,
            -(amplitude + offset),
            0.0f
        };
        DrawLine(gameState->lineGL, proj, view,
            lineData, Vec4::green);
        int fillEnd = (audio->fillStart + audio->fillLength)
            % audio->bufferSizeSamples;
        float32 tFillEnd = (float32)fillEnd / (audio->bufferSizeSamples - 1);
        lineData->pos[0] = Vec3 {
            tFillEnd * length - length / 2.0f,
            amplitude + offset,
            0.0f
        };
        lineData->pos[1] = Vec3 {
            tFillEnd * length - length / 2.0f,
            -(amplitude + offset),
            0.0f
        };
        DrawLine(gameState->lineGL, proj, view,
            lineData, Vec4::blue);

        /*length = 1.0f;
        lineData->count = audioState->soundDeath.buffer.bufferSizeSamples;
        for (int i = 0; i < lineData->count; i++) {
            int16 val = audioState->soundDeath.buffer.buffer[
                i * audio->channels];
            float32 normVal = (float32)val / INT16_MAXVAL;
            float32 t = (float32)i / (lineData->count - 1);
            lineData->pos[i] = {
                t * length - length / 2.0f,
                height * normVal,
                0.0f
            };
        }
        DrawLine(gameState->lineGL, proj, view,
            lineData, Vec4::one);*/
    }
#endif
}