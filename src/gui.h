#pragma once

#include "km_defines.h"

enum BufferViewDrawMode
{
	BUFFERVIEW_DRAW_BOTH,
	BUFFERVIEW_DRAW_0,
	BUFFERVIEW_DRAW_1,

	BUFFERVIEW_LAST
};

struct BufferView
{
	Vec2Int origin;
	Vec2Int size;

	uint8 channels;
	uint64 numSamples;
	float32* buffer;

	BufferViewDrawMode drawMode;
	float32 tOffset;
	Vec2 tSize;

	uint64 selectStart;
	uint64 selectEnd;

	void SetPosition(Vec2Int pos, Vec2Int size, Vec2 anchor);
	void UpdateAndDraw(const GameInput& input, const ScreenInfo& screenInfo,
		const RectGL& rectGL, const RectPixelGL& rectPixelGL, const LineGL& lineGL,
		const TextGL& textGL, const FontFace& fontFace,
		uint64 sampleRate, const MemoryBlock& transient);
};