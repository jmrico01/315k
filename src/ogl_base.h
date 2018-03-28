#pragma once

#include <GL/glew.h>

#include "km_math.h"

struct RectGL
{
    GLuint vertexArray;
    GLuint vertexBuffer;
    GLuint programID;
};

struct TexturedRectGL
{
    GLuint vertexArray;
    GLuint vertexBuffer;
    GLuint uvBuffer;
    GLuint programID;
};

struct LineGL
{
    GLuint vertexArray;
    GLuint vertexBuffer;
    GLuint programID;
};

void InitOpenGL();
void ResizeGL(int width, int height);

GLuint LoadShaders(
    const char* vertFilePath, const char* fragFilePath);

RectGL CreateRectGL();
TexturedRectGL CreateTexturedRectGL();
LineGL CreateLineGL();

void DrawRect(
    RectGL rectGL,
    Vec3 pos, Vec2 anchor, Vec2 size, Vec4 color);
void DrawTexturedRect(
    TexturedRectGL texturedRectGL,
    Vec3 pos, Vec2 anchor, Vec2 size, GLuint texture);
void DrawLine(
    LineGL lineGL, Mat4 proj, Mat4 view,
    Vec3 v1, Vec3 v2, Vec4 color);