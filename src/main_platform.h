#pragma once

#include "km_defines.h"
#include "km_math.h"
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

struct ScreenInfo
{
	int width;
	int height;

	int8 colorBits;
	int8 alphaBits;
	int8 depthBits;
	int8 stencilBits;
};

enum KeyInputCodes
{
    KM_KEY_A = 0,
    KM_KEY_B,
    KM_KEY_C,
    KM_KEY_D,
    KM_KEY_E,
    KM_KEY_F,
    KM_KEY_G,
    KM_KEY_H,
    KM_KEY_I,
    KM_KEY_J,
    KM_KEY_K,
    KM_KEY_L,
    KM_KEY_M,
    KM_KEY_N,
    KM_KEY_O,
    KM_KEY_P,
    KM_KEY_Q,
    KM_KEY_R,
    KM_KEY_S,
    KM_KEY_T,
    KM_KEY_U,
    KM_KEY_V,
    KM_KEY_W,
    KM_KEY_X,
    KM_KEY_Y,
    KM_KEY_Z,

    KM_KEY_1,
    KM_KEY_2,
    KM_KEY_3,
    KM_KEY_4,
    KM_KEY_5,
    KM_KEY_6,
    KM_KEY_7,
    KM_KEY_8,
    KM_KEY_9,
    KM_KEY_0,
    KM_KEY_NUMPAD_1,
    KM_KEY_NUMPAD_2,
    KM_KEY_NUMPAD_3,
    KM_KEY_NUMPAD_4,
    KM_KEY_NUMPAD_5,
    KM_KEY_NUMPAD_6,
    KM_KEY_NUMPAD_7,
    KM_KEY_NUMPAD_8,
    KM_KEY_NUMPAD_9,
    KM_KEY_NUMPAD_0,

    KM_KEY_ESC,
    KM_KEY_ENTER,
    KM_KEY_BACKSPACE,
    KM_KEY_TAB,

    KM_KEY_ARROW_UP,
    KM_KEY_ARROW_DOWN,
    KM_KEY_ARROW_LEFT,
    KM_KEY_ARROW_RIGHT,

    KM_KEY_LAST // Always keep this at the end.
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

	GameControllerInput controllers[4];
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
	GameMemory* memory, ScreenInfo screenInfo, GameInput* input, OpenGLFunctions* glFunctions)
typedef GAME_UPDATE_AND_RENDER_FUNC(GameUpdateAndRenderFunc);