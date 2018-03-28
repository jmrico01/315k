#pragma once

#ifdef _WIN32
#define GLEW_STATIC     // required by Windows
#include <Windows.h>    // ensure this is included before GLFW
#endif

#include "km_types.h"
#include "halfedge.h"

// TODO get rid of this
#define PATH_MAX_LENGTH 512

#define KEY_INPUT_BUFFER_MAX 256

struct KeyEvent {
    char ascii;
    bool pressed;
};

struct ReadFileResult
{
    uint64 size;
    void* data;
};

struct SharedState
{
    Vec3 cameraPos;
    Quat modelRot;

    HalfEdgeMesh mesh;
    HalfEdgeMeshGL meshGL;

    DynamicArray<uint32> selectedVerts;
    DynamicArray<uint32> allVerts;
};

char* GetAppPath();
void CatStrings(const char* str1, const char* str2, char* dest, uint64 destLen);
// Reads entire contents of file into memory.
// Returns a ReadFileResult containing the file data and size.
// free(..) must be called on the file data when done.
ReadFileResult ReadFile(const char* path);

// Transform from homogeneous coordinates to screen (pixel) coordinates
Mat4 HomogeneousToScreen();