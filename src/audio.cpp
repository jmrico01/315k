#include "audio.h"

#include "main.h"

/*internal float32 SquareWave(float32 t)
{
    float32 tMod = fmod(t, 2.0f * PI_F);
    return tMod < PI_F ? 1.0f : -1.0f;
}*/

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

internal void SoundWriteSamples(const Sound* sound,
    int i, float32 amplitude,
    GameAudio* audio)
{
    int sampleInd = sound->sampleIndex + i;
    if (sound->playing
    && sampleInd < sound->buffers[sound->activeVariation].bufferSizeSamples) {
        int16 sample1 = (int16)(amplitude
            * sound->buffers[sound->activeVariation].buffer[
                sampleInd * audio->channels]);
        int16 sample2 = (int16)(amplitude
            * sound->buffers[sound->activeVariation].buffer[
                sampleInd * audio->channels + 1]);

        int ind = (audio->fillStart + i) % audio->bufferSizeSamples;
        audio->buffer[ind * audio->channels]        += sample1;
        audio->buffer[ind * audio->channels + 1]    += sample2;
    }
}

void InitAudioState(const ThreadContext* thread,
    AudioState* audioState, GameAudio* audio,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory)
{
    audioState->runningSampleIndex = 0;

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

#if GAME_INTERNAL
    audioState->debugView = false;
#endif
}

void OutputAudio(GameAudio* audio, GameState* gameState,
    const GameInput* input, GameMemory* memory)
{
    AudioState* audioState = &gameState->audioState;
    audioState->runningSampleIndex += audio->fillStartDelta;

    SoundUpdate(audio, &audioState->soundKick);
    SoundUpdate(audio, &audioState->soundSnare);
    SoundUpdate(audio, &audioState->soundDeath);

    for (int i = 0; i < audio->fillLength; i++) {
        uint32 ind = (audio->fillStart + i) % audio->bufferSizeSamples;
        audio->buffer[ind * audio->channels]        = 0;
        audio->buffer[ind * audio->channels + 1]    = 0;

        SoundWriteSamples(&audioState->soundKick, i, 1.0f, audio);
        SoundWriteSamples(&audioState->soundSnare, i, 0.7f, audio);
        SoundWriteSamples(&audioState->soundDeath, i, 0.5f, audio);

        //audio->buffer[ind * audio->channels]      = 0;
        //audio->buffer[ind * audio->channels + 1]  = 0;
    }

#if GAME_INTERNAL
    if (WasKeyPressed(input, KM_KEY_G)) {
        audioState->debugView = !audioState->debugView;
    }
    if (audioState->debugView) {
        DEBUG_ASSERT(memory->transientStorageSize >= sizeof(LineGLData));
        DEBUG_ASSERT(audio->bufferSizeSamples <= MAX_LINE_POINTS);
        Mat4 proj = Mat4::one;
        Vec3 zoomScale = {
            1.0f + gameState->debugZoom,
            1.0f + gameState->debugZoom,
            1.0f
        };
        Mat4 view = Translate(-gameState->debugCamPos) * Scale(zoomScale);

        LineGLData* lineData = (LineGLData*)memory->transientStorage;
        lineData->count = audio->bufferSizeSamples;
        float32 length = 2.0f;
        float32 height = 1.0f;
        float32 offset = 1.0f;
        for (int i = 0; i < audio->bufferSizeSamples; i++) {
            int16 val = audio->buffer[i * audio->channels];
            float32 normVal = (float32)val / INT16_MAXVAL;
            float32 t = (float32)i / (audio->bufferSizeSamples - 1);
            lineData->pos[i] = {
                t * length - length / 2.0f,
                height * normVal + offset,
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
                height * normVal - offset,
                0.0f
            };
        }
        DrawLine(gameState->lineGL, proj, view,
            lineData, Vec4::one);

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