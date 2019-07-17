#include "gui.h"

internal void DrawAudioBuffer(
	const GameState* gameState, const GameAudio* audio,
	const float32* buffer, uint64 bufferSizeSamples, uint8 channel,
	const int marks[], const Vec4 markColors[], int numMarks,
	Vec3 origin, Vec2 size, Vec4 color,
	MemoryBlock transient)
{
	DEBUG_ASSERT(transient.size >= sizeof(LineGLData));
	DEBUG_ASSERT(bufferSizeSamples <= MAX_LINE_POINTS);
	LineGLData* lineData = (LineGLData*)transient.memory;
	Mat4 proj = Mat4::one;
	Vec3 zoomScale = {
		1.0f + gameState->debugZoom,
		1.0f + gameState->debugZoom,
		1.0f
	};
	Mat4 view = Scale(zoomScale) * Translate(gameState->debugCamPos);
	
	lineData->count = bufferSizeSamples;
	for (uint64 i = 0; i < bufferSizeSamples; i++) {
		float32 val = buffer[i * channels + channel];
		float32 t = (float32)i / (bufferSizeSamples - 1);
		lineData->pos[i] = {
			origin.x + t * size.x,
			origin.y + size.y * val,
			origin.z
		};
	}
	DrawLine(gameState->lineGL, proj, view,
		lineData, color);

	lineData->count = 2;
	for (int i = 0; i < numMarks; i++) {
		float32 tMark = (float32)marks[i] / (bufferSizeSamples - 1);
		lineData->pos[0] = Vec3 {
			origin.x + tMark * size.x,
			origin.y,
			origin.z
		};
		lineData->pos[1] = Vec3 {
			origin.x + tMark * size.x,
			origin.y + size.y,
			origin.z
		};
		DrawLine(gameState->lineGL, proj, view,
			lineData, markColors[i]);
	}
}

void BufferView::Init(Vec2Int pos, Vec2Int size, Vec2 anchor, uint64 numSamples, const float32* buffer)
{
	this.origin = { pos.x - (int)(size.x * anchor.x), pos.y - (int)(size.y * anchor.y) };
	this.size = size;
	this.anchor = anchor;
	this.numSamples = numSamples;
	this.buffer = buffer;

	sampleStart = 0;
	sampleEnd = numSamples;
}

void BufferView::Update(const GameInput& input, float32 deltaTime)
{

}

void BufferView::Draw()
{
}