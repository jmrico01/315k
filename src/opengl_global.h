#pragma once

#include "opengl.h"

// TODO maybe there's a better way of doing this...
#define FUNC(returntype, name, ...) extern name##Func* name;
    GL_FUNCTIONS_BASE
    GL_FUNCTIONS_ALL
#undef FUNC