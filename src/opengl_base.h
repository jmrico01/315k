#pragma once

#include <km_common/km_math.h>
#include <km_platform/main_platform.h>

#include "opengl.h"

#define MAX_LINE_POINTS 1048576

struct RectGL
{
	GLuint vertexArray;
	GLuint vertexBuffer;
	GLuint programID;
    
	template <typename Allocator>
        void Init(Allocator* allocator);
	void Draw(Mat4 transform, Vec4 color) const;
};

struct RectPixelGL
{
	GLuint vertexArray;
	GLuint vertexBuffer;
	GLuint programID;
};

struct TexturedRectPixelGL
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

struct PlaneGL
{
	GLuint vertexArray;
	GLuint vertexBuffer;
	GLuint programID;
};

struct BoxGL
{
	GLuint vertexArray;
	GLuint vertexBuffer;
	GLuint programID;
};

struct RectCoordsNDC
{
	Vec3 pos;
	Vec2 size;
};

struct LineGLData
{
	uint64 count;
	Vec3 pos[MAX_LINE_POINTS];
};

RectCoordsNDC ToRectCoordsNDC(Vec2Int pos, Vec2Int size,
                              ScreenInfo screenInfo);
RectCoordsNDC ToRectCoordsNDC(Vec2Int pos, Vec2Int size, Vec2 anchor,
                              ScreenInfo screenInfo);

template <typename Allocator>
GLuint LoadShaders(Allocator* allocator,
                   const char* vertFilePath, const char* fragFilePath);

template <typename Allocator>
RectPixelGL InitRectPixelGL(Allocator* allocator);
template <typename Allocator>
TexturedRectPixelGL InitTexturedRectPixelGL(Allocator* allocator);
template <typename Allocator>
LineGL InitLineGL(Allocator* allocator);
template <typename Allocator>
PlaneGL InitPlaneGL(Allocator* allocator);
template <typename Allocator>
BoxGL InitBoxGL(Allocator* allocator);

void DrawRectPixel(const RectPixelGL& rectPixelGL,
                   const ScreenInfo& screenInfo,
                   Vec2Int pos, Vec2 anchor, Vec2Int size, Vec4 color);
void DrawTexturedRectPixel(const TexturedRectPixelGL& texturedRectPixelGL,
                           const ScreenInfo& screenInfo,
                           Vec2Int pos, Vec2 anchor, Vec2Int size, bool flipHorizontal, GLuint texture);
void DrawPlane(PlaneGL,
               Mat4 vp, Vec3 point, Vec3 normal, Vec4 color);
void DrawBox(BoxGL boxGL,
             Mat4 vp, Vec3 min, Vec3 max, Vec4 color);

// Batch functions
void DrawLine(LineGL lineGL, Mat4 transform, const LineGLData* lineData, Vec4 color);
