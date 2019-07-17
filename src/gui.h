#pragma once

#include "km_defines.h"

struct BufferView
{
    Vec2Int origin;
    Vec2Int size;

    uint8 channels;
    uint64 numSamples;
    float32* buffer;

    uint64 sampleStart;
    uint64 sampleEnd;

    void Init(Vec2Int pos, Vec2Int size, Vec2 anchor, uint64 numSamples, const float32* buffer);
    void Update(const GameInput& input, float32 deltaTime);
    void Draw();
};