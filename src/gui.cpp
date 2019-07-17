#include "gui.h"

#include "km_input.h"
#include "opengl_base.h"

void BufferView::SetPosition(Vec2Int pos, Vec2Int size, Vec2 anchor)
{
	DEBUG_ASSERT(numSamples <= MAX_LINE_POINTS);

	this->origin = { pos.x - (int)(size.x * anchor.x), pos.y - (int)(size.y * anchor.y) };
	this->size = size;
}

void BufferView::UpdateAndDraw(const GameInput& input, const ScreenInfo& screenInfo,
	const RectGL& rectGL, const LineGL& lineGL, const MemoryBlock& transient)
{
	DEBUG_ASSERT(transient.size >= sizeof(LineGLData));

	Vec2Int mousePosToOrigin = input.mousePos - origin;
	float32 tMouse = (float32)mousePosToOrigin.x / size.x;
	if (input.mouseButtons[0].isDown) {
		tCenter += input.mouseDelta.x * 0.001f;
	}

	float32 deltaZoom = input.mouseWheelDelta * 0.001f;
	if (IsKeyPressed(&input, KM_KEY_CTRL)) {
		tZoom.y += deltaZoom * 2.0f;
	}
	else {
		tZoom.x += deltaZoom;
	}

	DrawRect(rectGL, screenInfo, origin, Vec2::zero, size, Vec4 { 0.1f, 0.4f, 1.0f, 0.2f });

	Vec2 tSize = Vec2 { exp(tZoom.x), exp(tZoom.y) };
	LineGLData* lineData = (LineGLData*)transient.memory;
	Mat4 transformBox = Translate(Vec3 { -1.0f, -1.0f, 0.0f })
		* Scale(Vec3 { 2.0f / screenInfo.size.x, 2.0f / screenInfo.size.y, 1.0f })
		* Translate(Vec3 { (float32)origin.x, (float32)origin.y + size.y / 2.0f, 0.0f })
		* Scale(Vec3 { (float32)size.x, (float32)size.y / 2.0f, 1.0f });
	Mat4 transformBuffer = Translate(Vec3 { (tCenter - tSize.x / 2.0f), 0.0f, 0.0f })
		* Scale(Vec3 { tSize.x, tSize.y, 1.0f });
	
	lineData->count = numSamples;
	if (lineData->count > MAX_LINE_POINTS) {
		lineData->count = MAX_LINE_POINTS;
	}
	for (uint64 i = 0; i < lineData->count; i++) {
		float32 t = (float32)i / (lineData->count - 1);
		lineData->pos[i] = { t, buffer[i * channels/* + channel*/], 0.0f };
	}
	Vec4 color = Vec4::one;
	DrawLine(lineGL, transformBox * transformBuffer, lineData, color);

	if (0.0f <= tMouse && tMouse <= 1.0f) {
		lineData->count = 2;
		float32 tMousePos = tMouse;
		lineData->pos[0] = Vec3 { tMousePos, -1.0f, 0.0f };
		lineData->pos[1] = Vec3 { tMousePos, 1.0f, 0.0f };
		Vec4 markColor = Vec4 { 1.0f, 0.4f, 0.1f, 0.8f };
		DrawLine(lineGL, transformBox, lineData, markColor);
	}
}