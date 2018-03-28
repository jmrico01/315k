#pragma once

#if GAME_SLOW
#define ASSERT(expression) if (!(expression)) { *(int *)0 = 0; }
#else
#define ASSERT(expression)
#endif