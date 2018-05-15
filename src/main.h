#pragma once

#include "km_math.h"
#include "opengl.h"
#include "opengl_base.h"
#include "text.h"

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

    int circlePos;
    Vec3 debugCamPos;

    GLuint rectShader;
    GLuint rectVAO;

    RectGL rectGL;
    LineGL lineGL;
    TextGL textGL;

    MarkerGL markerGL;
    CircleGL circleGL;

    FT_Library ftLibrary;
    FontFace fontFace;

    GLuint framebuffer;
    GLuint colorBuffer;
};