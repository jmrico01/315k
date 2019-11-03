#pragma once

#include <km_common/km_defines.h>

const uint64 BUFFERVIEW_MAX_MARKS = 1024;

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

	FixedArray<uint64, BUFFERVIEW_MAX_MARKS> marks;

	BufferViewDrawMode drawMode;
	float32 tOffset;
	Vec2 tSize;

	uint64 selectStart;
	uint64 selectEnd;

	void SetPosition(Vec2Int newPos, Vec2Int newSize, Vec2 newAnchor);
	void ResetControls();
	void UpdateAndDraw(const GameInput& input, const ScreenInfo& screenInfo,
		const RectGL& rectGL, const RectPixelGL& rectPixelGL, const LineGL& lineGL,
		const TextGL& textGL, const FontFace& fontFace,
		uint64 sampleRate, const MemoryBlock& transient);
};

void FillNormalizedLineGLDataFromBuffer(const float32* buffer, uint64 numSamples,
	uint8 channels, uint8 channel, LineGLData* lineData);