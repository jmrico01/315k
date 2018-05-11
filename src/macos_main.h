#pragma once

#include "km_defines.h"

#define MACOS_STATE_FILE_NAME_COUNT  512

struct MacOSState
{
	uint64 gameMemorySize;
	void* gameMemoryBlock;
};
