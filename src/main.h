#pragma once

#include "km_math.h"
#include "opengl.h"
#include "opengl_base.h"
#include "text.h"

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

struct AudioState
{
    int runningSampleIndex;
    float32 amplitude;
    float32 tSine1;
    float32 tSine2;

#if GAME_INTERNAL
    bool32 debugView;
#endif
};

struct GameState
{
    AudioState audioState;

    Tiles tiles;

    Vec3 pos;
    float32 angle;

    GLuint rectShader;
    GLuint rectVAO;

    LineGL lineGL;
    TextGL textGL;
    FT_Library ftLibrary;
    FontFace fontFace;
};