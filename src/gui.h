#pragma once

#include "km_defines.h"

struct BufferView
{
	Vec2Int origin;
	Vec2Int size;

	uint8 channels;
	uint64 numSamples;
	float32* buffer;

	float32 tCenter;
	Vec2 tZoom;

	void SetPosition(Vec2Int pos, Vec2Int size, Vec2 anchor);
	void UpdateAndDraw(const GameInput& input, const ScreenInfo& screenInfo,
		const RectGL& rectGL, const LineGL& lineGL, const MemoryBlock& transient);
};