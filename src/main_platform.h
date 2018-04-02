#pragma once

#include "km_defines.h"
#include "km_math.h"
#include "km_input.h"
#include "opengl.h"

struct ThreadContext
{
	int placeholder;
};

// ---------------------------- Platform functions ----------------------------
#if GAME_INTERNAL

struct DEBUGReadFileResult
{
	uint64 size;
	void* data;
};

#define DEBUG_PLATFORM_PRINT_FUNC(name) void name(const char* format, ...)
typedef DEBUG_PLATFORM_PRINT_FUNC(DEBUGPlatformPrintFunc);

#define DEBUG_PLATFORM_FREE_FILE_MEMORY_FUNC(name) \
    void name(ThreadContext* thread, DEBUGReadFileResult* file)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY_FUNC(DEBUGPlatformFreeFileMemoryFunc);

#define DEBUG_PLATFORM_READ_FILE_FUNC(name) \
    DEBUGReadFileResult name(ThreadContext* thread, const char* fileName)
typedef DEBUG_PLATFORM_READ_FILE_FUNC(DEBUGPlatformReadFileFunc);

#define DEBUG_PLATFORM_WRITE_FILE_FUNC(name) \
    bool32 name(ThreadContext* thread, const char* fileName, \
        uint32 memorySize, const void* memory)
typedef DEBUG_PLATFORM_WRITE_FILE_FUNC(DEBUGPlatformWriteFileFunc);

#endif

#define MAX_KEYS_PER_FRAME 256

struct ScreenInfo
{
	int width;
	int height;

	int8 colorBits;
	int8 alphaBits;
	int8 depthBits;
	int8 stencilBits;
};

struct GameButtonState
{
	int transitions;
	bool32 isDown;
};

struct GameControllerInput
{
	bool32 isConnected;

	Vec2 start;
	Vec2 end;

	union
	{
		GameButtonState buttons[6];
		struct
		{
			GameButtonState a;
			GameButtonState b;
			GameButtonState x;
			GameButtonState y;
			GameButtonState lShoulder;
			GameButtonState rShoulder;
		};
	};
};

struct GameInput
{
	GameButtonState mouseButtons[5];
	int32 mouseX, mouseY, mouseWheel;

    GameButtonState keyboard[KM_KEY_LAST];
    char keyboardString[MAX_KEYS_PER_FRAME];
    uint32 keyboardStringLen;

	GameControllerInput controllers[4];
};

struct GameAudio
{
    uint32 channels;
    uint32 sampleRate;
    
    uint32 bufferSize;
    int16* buffer;
};

struct GameMemory
{
	bool32 isInitialized;

	uint64 permanentStorageSize;
	// Required to be cleared to zero at startup
	void* permanentStorage;

	uint64 transientStorageSize;
	// Required to be cleared to zero at startup
	void* transientStorage;

	DEBUGPlatformPrintFunc*			    DEBUGPlatformPrint;
	DEBUGPlatformFreeFileMemoryFunc*	DEBUGPlatformFreeFileMemory;
	DEBUGPlatformReadFileFunc*			DEBUGPlatformReadFile;
	DEBUGPlatformWriteFileFunc*			DEBUGPlatformWriteFile;

    #if GAME_INTERNAL
    bool32 DEBUGShouldInitGlobals;
    #endif
};

// ------------------------------ Game functions ------------------------------
#define GAME_UPDATE_AND_RENDER_FUNC(name) void name(ThreadContext* thread, \
	GameMemory* memory, ScreenInfo screenInfo, GameInput* input, \
    GameAudio* audio, OpenGLFunctions* glFunctions)
typedef GAME_UPDATE_AND_RENDER_FUNC(GameUpdateAndRenderFunc);