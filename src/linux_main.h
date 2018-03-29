#pragma once

#include "km_defines.h"
#include "main_platform.h"

// Looked up with xev command
#define KEYCODE_W           25
#define KEYCODE_A           38
#define KEYCODE_S           39
#define KEYCODE_D           40
#define KEYCODE_Q           24
#define KEYCODE_E           26
#define KEYCODE_UP          111
#define KEYCODE_DOWN        116
#define KEYCODE_LEFT        113
#define KEYCODE_RIGHT       114
#define KEYCODE_ESCAPE      9
#define KEYCODE_ENTER       36
#define KEYCODE_SPACE       65
#define KEYCODE_P           33
#define KEYCODE_L           46

#define KEYCODE_SHIFT_L     50
#define KEYCODE_SHIFT_R     62
#define KEYCODE_CTRL_L      37
#define KEYCODE_CTRL_R      105
#define KEYCODE_ALT_L       64
#define KEYCODE_ALT_R       108
#define KEYCODE_SUPER       133

#define KEYCODE_PLUS        21
#define KEYCODE_MINUS       20

#define KEYCODE_F1          67
#define KEYCODE_F10         76
#define KEYCODE_F11         95
#define KEYCODE_F12         96

#define KEYCODE_SHIFT_MASK  0x01
#define KEYCODE_CTRL_MASK   0x04
#define KEYCODE_ALT_MASK    0x08

#define BYTES_PER_PIXEL 4
struct LinuxOffscreenBuffer
{
    // Pixels are always 32-bits wide, little endian 0x AA RR GG BB, or [0xBB, 0xGG, 0xRR, 0xAA] in memory
    uint32 Width;
    uint32 Height;
    uint32 Pitch;
    uint8 *Memory;   // pointer to texture buffer
};

struct LinuxWindowDimension
{
    uint32 Width;
    uint32 Height;
};

struct LinuxDebugTimeMarker
{
    uint32 OutputPlayCursor;
    uint32 OutputWriteCursor;
    uint32 OutputLocation;
    uint32 OutputByteCount;
    uint32 ExpectedFlipPlayCursor;

    uint32 FlipPlayCursor;
    uint32 FlipWriteCursor;
};

struct LinuxGameCode
{
    //void *GameLibHandle;
    //ino_t GameLibID;

    // IMPORTANT(casey): Either of the callbacks can be 0!  You must
    // check before calling.
    //game_update_and_render *UpdateAndRender;
    //game_get_sound_samples *GetSoundSamples;
    //debug_game_frame_end *DEBUGFrameEnd;

    bool32 IsValid;
};

enum linux_memory_block_flag
{
    LINUXMEM_ALLOCATED_DURING_LOOP  = 0x1,
    LINUXMEM_FREED_DURING_LOOP      = 0x2
};
struct LinuxMemoryBlock
{
    GameMemory memory;
    LinuxMemoryBlock *Prev;
    LinuxMemoryBlock *Next;
    uint64 LoopingFlags;
};

struct linux_saved_memory_block
{
    uint64 BasePointer;
    uint64 Size;
};

#define LINUX_STATE_FILE_NAME_COUNT  512
struct LinuxState
{
    // NOTE(casey): To touch the memory ring, you must
    // take the memory mutex!
    //ticket_mutex MemoryMutex;
    //linux_memory_block MemorySentinel;

    int32 RecordingHandle;
    int32 InputRecordingIndex;

    int32 PlaybackHandle;
    int32 InputPlayingIndex;

    char EXEFileName[LINUX_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
};

/*struct linux_thread_startup
{
    platform_work_queue *Queue;
};*/