#pragma once

#include "km_math.h"
#include "load_png.h"
#include "opengl.h"
#include "opengl_base.h"
#include "text.h"
#include "particles.h"
#include "audio.h"

#define NUM_FRAMEBUFFERS 3
#define MAX_LEVEL_HALFBEATS 16
#define DEATH_DURATION_HALFBEATS 4

struct MarkerGL
{
    GLuint vertexArray;
    GLuint vertexBuffer;
    GLuint programID;
};
struct CircleGL
{
    GLuint vertexArray;
    GLuint vertexBuffer;
    GLuint programID;
};

struct Tiles
{
#define TILES_X     100
#define TILES_Y     100
#define TILE_SIZE   0.05f
    float32 height[TILES_X][TILES_Y];
    Vec4 color[TILES_X][TILES_Y];

    GLuint vao;
    GLuint quadIndBuffer;
    GLuint lineIndBuffer;

    GLuint shader;
};

struct GameState
{
    AudioState audioState;

    // Game data --------------------------
    int bpm;
    float32 lastHalfBeat;
    float32 lastBeat;
    int halfBeatCount;

    int levelLength;
    int respawn;
    bool snareHits[MAX_LEVEL_HALFBEATS][12];
    bool notes[MAX_LEVEL_HALFBEATS][12];

    int circlePos;

    bool32 dead;
    float32 deadTime;
    int deadHalfBeats;
    int killerHalfBeat;
    // ------------------------------------

    Vec3 debugCamPos;
    float32 debugZoom;

    RectGL rectGL;
    TexturedRectGL texturedRectGL;
    LineGL lineGL;
    TextGL textGL;
    ParticleSystemGL psGL;

    MarkerGL markerGL;
    CircleGL circleGL;

    FT_Library ftLibrary;
    FontFace fontFaceSmall;
    FontFace fontFaceMedium;

    GLuint framebuffers[NUM_FRAMEBUFFERS];
    GLuint colorBuffers[NUM_FRAMEBUFFERS];
    GLuint screenQuadVertexArray;
    GLuint screenQuadVertexBuffer;
    GLuint screenQuadUVBuffer;

    GLuint screenShader;
    GLuint bloomExtractShader;
    GLuint bloomBlendShader;
    GLuint blurShader;
    GLuint grainShader;
    GLuint grainTexture;
    GLuint fxaaShader;

    TextureGL pTexBase;
    ParticleSystem ps;
};

inline float32 RandFloat32();
inline float32 RandFloat32(float32 min, float32 max);
