#include "gui.h"

#include <km_common/km_input.h>

#include "opengl_base.h"

void BufferView::SetPosition(Vec2Int pos, Vec2Int size, Vec2 anchor)
{
	DEBUG_ASSERT(numSamples <= MAX_LINE_POINTS);

	this->origin = { pos.x - (int)(size.x * anchor.x), pos.y - (int)(size.y * anchor.y) };
	this->size = size;
}

void BufferView::ResetControls()
{
	tOffset = 0.0f;
	tSize = Vec2::one;
	drawMode = BUFFERVIEW_DRAW_BOTH;
	selectStart = 0;
	selectEnd = 0;
}

void BufferView::UpdateAndDraw(const GameInput& input, const ScreenInfo& screenInfo,
	const RectGL& rectGL, const RectPixelGL& rectPixelGL, const LineGL& lineGL,
	const TextGL& textGL, const FontFace& fontFace,
	uint64 sampleRate, const MemoryBlock& transient)
{
	const Vec3 COLOR_BACKGROUND = Vec3 { 0.1f, 0.4f, 1.0f };
	const Vec3 COLOR_SELECTION = Vec3 { 1.0f, 0.4f, 0.1f };
	const Vec3 COLOR_INFO_TEXT = Vec3 { 0.5f, 1.0f, 0.5f };
	const float32 ALPHA_BACKGROUND = 0.2f;

	if (IsKeyPressed(&input, KM_KEY_P)) {
		ResetControls();
	}

	Vec2Int mousePosToOrigin = input.mousePos - origin;
	float32 tMouseNorm = (float32)mousePosToOrigin.x / size.x;
	float32 tMouse = tMouseNorm / tSize.x - tOffset;
	float32 timeMouse = tMouse * numSamples / sampleRate * 1000.0f;
	uint64 sampleMouse = ClampInt((int)(tMouse * numSamples), 0, (int)numSamples);

	if (input.mouseButtons[1].isDown) {
		if (input.mouseButtons[1].transitions == 1) {
			selectStart = sampleMouse;
		}
		selectEnd = sampleMouse;
	}
	if (input.mouseButtons[2].isDown && input.mouseButtons[2].transitions == 1) {
		drawMode = (BufferViewDrawMode)(((int)drawMode + 1) % (int)BUFFERVIEW_LAST);
	}

	float32 deltaZoom = input.mouseWheelDelta * 0.001f;
	if (input.mouseWheelDelta != 0) {
		if (IsKeyPressed(&input, KM_KEY_CTRL)) {
			tSize.y *= exp(deltaZoom * 2.0f);
		}
		else {
			float32 tSizeXPrev = tSize.x;
			tSize.x *= exp(deltaZoom);
			tOffset += tMouseNorm * (1.0f / tSize.x - 1.0f / tSizeXPrev);
		}
	}

	if (input.mouseButtons[0].isDown) {
		tOffset += (float32)input.mouseDelta.x / size.x / tSize.x;
	}

	Mat4 transform = Translate(Vec3 { -1.0f, -1.0f, 0.0f })
		* Scale(Vec3 { 2.0f / screenInfo.size.x, 2.0f / screenInfo.size.y, 1.0f })
		* Translate(Vec3 { (float32)origin.x, (float32)origin.y + size.y / 2.0f, 0.0f })
		* Scale(Vec3 { (float32)size.x, (float32)size.y / 2.0f, 1.0f })
		* Scale(Vec3 { tSize.x, tSize.y, 1.0f })
		* Translate(Vec3 { tOffset, 0.0f, 0.0f });

	DrawRectPixel(rectPixelGL, screenInfo, origin, Vec2::zero, size,
		ToVec4(COLOR_BACKGROUND, ALPHA_BACKGROUND));
	if (selectStart < selectEnd) {
		Vec3 selectOrigin = {
			(float32)selectStart / numSamples,
			-1.0f,
			0.0f
		};
		Vec3 selectScale = {
			(float32)(selectEnd - selectStart) / numSamples,
			2.0f,
			1.0f
		};
		Mat4 selectTransform = Translate(selectOrigin) * Scale(selectScale);
		rectGL.Draw(transform * selectTransform, ToVec4(COLOR_SELECTION, ALPHA_BACKGROUND));
	}

	DEBUG_ASSERT(transient.size >= sizeof(LineGLData));
	LineGLData* lineData = (LineGLData*)transient.memory;

	uint8 channel = 0;
	if (drawMode == BUFFERVIEW_DRAW_1) {
		channel = 1;
	}
	FillNormalizedLineGLDataFromBuffer(buffer, numSamples, channels, channel, lineData);
	Vec4 color = Vec4::one; // TODO maybe factor this color
	if (drawMode == BUFFERVIEW_DRAW_BOTH) {
		color = Vec4 { 1.0f, 0.0f, 0.0f, 1.0f };
	}
	DrawLine(lineGL, transform, lineData, color);

	if (drawMode == BUFFERVIEW_DRAW_BOTH) {
		FillNormalizedLineGLDataFromBuffer(buffer, numSamples, channels, 1, lineData);
		DrawLine(lineGL, transform, lineData, Vec4 { 0.0f, 1.0f, 0.0f, 1.0f }); // and this one
	}

	lineData->count = marks.array.size * 3;
	for (uint64 i = 0; i < marks.array.size; i++) {
		float32 tMark = (float32)marks[i] / (numSamples - 1);
		lineData->pos[i * 3]     = Vec3 { tMark, -1.0f, 0.0f };
		lineData->pos[i * 3 + 1] = Vec3 { tMark,  1.0f, 0.0f };
		lineData->pos[i * 3 + 2] = Vec3 { tMark, -1.0f, 0.0f };
	}
	DrawLine(lineGL, transform, lineData, ToVec4(COLOR_INFO_TEXT, 0.2f));

	if (0.0f <= tMouse && tMouse <= 1.0f) {
		lineData->count = 2;
		lineData->pos[0] = Vec3 { tMouse, -1.0f, 0.0f };
		lineData->pos[1] = Vec3 { tMouse, 1.0f, 0.0f };
		DrawLine(lineGL, transform, lineData, ToVec4(COLOR_SELECTION, 1.0f));
	}

	const uint64 BUFFERVIEW_STRING_MAX_LENGTH = 256;
	const int BUFFERVIEW_TEXT_MARGIN = 10;
	char buf[BUFFERVIEW_STRING_MAX_LENGTH];

	snprintf(buf, BUFFERVIEW_STRING_MAX_LENGTH, "t %.03f | time %.03f ms | sample %llu",
		tMouse, timeMouse, sampleMouse);
	DrawText(textGL, fontFace, screenInfo, buf,
		Vec2Int { origin.x, origin.y + size.y + BUFFERVIEW_TEXT_MARGIN },
		ToVec4(COLOR_INFO_TEXT, 1.0f),
		transient
	);

	if (selectStart < selectEnd) {
		Vec2Int selectTextOrigin = {
			origin.x + size.x,
			origin.y + size.y + (int)fontFace.height * 2 + BUFFERVIEW_TEXT_MARGIN * 3
		};

		float32 selectStartMS = (float32)selectStart / sampleRate * 1000.0f;
		float32 selectEndMS = (float32)selectEnd / sampleRate * 1000.0f;
		snprintf(buf, BUFFERVIEW_STRING_MAX_LENGTH, "delta %.03f ms",
			selectEndMS - selectStartMS);
		DrawText(textGL, fontFace, screenInfo, buf,
			selectTextOrigin, Vec2 { 1.0f, 0.0f },
			ToVec4(COLOR_SELECTION, 1.0f),
			transient
		);

		selectTextOrigin.y -= fontFace.height + BUFFERVIEW_TEXT_MARGIN;
		snprintf(buf, BUFFERVIEW_STRING_MAX_LENGTH, "time %.03f - %.03f ms",
			selectStartMS, selectEndMS);
		DrawText(textGL, fontFace, screenInfo, buf,
			selectTextOrigin, Vec2 { 1.0f, 0.0f },
			ToVec4(COLOR_SELECTION, 1.0f),
			transient
		);

		selectTextOrigin.y -= fontFace.height + BUFFERVIEW_TEXT_MARGIN;
		snprintf(buf, BUFFERVIEW_STRING_MAX_LENGTH, "samples %llu - %llu",
			selectStart, selectEnd);
		DrawText(textGL, fontFace, screenInfo, buf,
			selectTextOrigin, Vec2 { 1.0f, 0.0f },
			ToVec4(COLOR_SELECTION, 1.0f),
			transient
		);
	}

	snprintf(buf, BUFFERVIEW_STRING_MAX_LENGTH, "sample rate %llu Hz | channels %u",
		sampleRate, channels);
	DrawText(textGL, fontFace, screenInfo, buf,
		Vec2Int { origin.x, origin.y - BUFFERVIEW_TEXT_MARGIN }, Vec2 { 0.0f, 1.0f },
		Vec4::one,
		transient
	);

	const char* drawModeNames[] = {
		"CHANNELS 0+1",
		"CHANNEL  0  ",
		"CHANNEL  1  "
	};
	snprintf(buf, BUFFERVIEW_STRING_MAX_LENGTH, "draw mode %s", drawModeNames[drawMode]);
	DrawText(textGL, fontFace, screenInfo, buf,
		Vec2Int { origin.x + size.x, origin.y - BUFFERVIEW_TEXT_MARGIN }, Vec2 { 1.0f, 1.0f },
		Vec4::one,
		transient
	);
}

void FillNormalizedLineGLDataFromBuffer(const float32* buffer, uint64 numSamples,
	uint8 channels, uint8 channel, LineGLData* lineData)
{
	lineData->count = numSamples;
	if (lineData->count > MAX_LINE_POINTS) {
		lineData->count = MAX_LINE_POINTS;
	}
	for (uint64 i = 0; i < lineData->count; i++) {
		float32 t = (float32)i / (numSamples - 1);
		lineData->pos[i] = { t, buffer[i * channels + channel], 0.0f };
	}
}
