#pragma once

#include <stdlib.h>

#include "km_defines.h"
#include "km_log.h"
#include "km_math.h"
#include "main.h"
#include "main_platform.h"
#include "opengl_base.h"
#include "text.h"

#if GAME_SLOW
#define DEBUG_ASSERTF(expression, format, ...) if (!(expression)) { \
    LOG_ERROR("Assert failed:\n"); \
    LOG_ERROR(format, ##__VA_ARGS__); \
    flushLogs_(logState_); \
    abort(); }
#define DEBUG_ASSERT(expression) DEBUG_ASSERTF(expression, "")
#define DEBUG_PANIC(format, ...) \
	LOG_ERROR("PANIC!\n"); \
	LOG_ERROR(format, ##__VA_ARGS__); \
	flushLogs_(logState_); \
    abort();
#elif GAME_INTERNAL
#define DEBUG_ASSERTF(expression, format, ...) if (!(expression)) { \
    LOG_ERROR("Assert failed\n"); \
    LOG_ERROR(format, ##__VA_ARGS__); \
    flushLogs_(logState_); }
#define DEBUG_ASSERT(expression) DEBUG_ASSERTF(expression, "")
#define DEBUG_PANIC(format, ...) \
    LOG_ERROR("PANIC!\n"); \
    LOG_ERROR(format, ##__VA_ARGS__); \
    flushLogs_(logState_);
#else
// TODO rethink these macros maybe, at least the panic
#define DEBUG_ASSERTF(expression, format, ...)
#define DEBUG_ASSERT(expression)
#define DEBUG_PANIC(format, ...)
#endif

#define INPUT_BUFFER_SIZE 2048
