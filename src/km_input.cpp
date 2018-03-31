#include "km_input.h"
#include "main_platform.h"

inline bool32 WasKeyPressed(GameInput* input, KeyInputCode keyCode)
{
    return input->keyboard[keyCode].isDown
        && input->keyboard[keyCode].transitions > 0;
}

inline bool32 WasKeyReleased(GameInput* input, KeyInputCode keyCode)
{
    return !input->keyboard[keyCode].isDown
        && input->keyboard[keyCode].transitions > 0;
}