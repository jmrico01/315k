#pragma once

#if GAME_SLOW
global_var DEBUGPlatformPrintFunc* debugPrint_;
#define DEBUG_ASSERT(expression) if (!(expression)) { *(int *)0 = 0; }
#define DEBUG_PANIC(format, ...) debugPrint_(format, ##__VA_ARGS__); \
    *(int *)0 = 0;
#define DEBUG_PRINT(format, ...) debugPrint_(format, ##__VA_ARGS__)
#else
#define DEBUG_ASSERT(expression)
#define DEBUG_PANIC(format, ...)
#define DEBUG_PRINT(format, ...)
#endif