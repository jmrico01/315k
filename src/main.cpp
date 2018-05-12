#include "main.h"

#include "main_platform.h"
#include "km_debug.h"
#include "km_defines.h"
#include "km_input.h"
#include "km_math.h"
#include "opengl.h"
#include "opengl_funcs.h"
#include "opengl_base.h"

extern "C" GAME_UPDATE_AND_RENDER_FUNC(GameUpdateAndRender)
{
    // NOTE: for clarity
    // A call to this function means the following has happened:
    //  - A frame has been displayed to the user
    //  - The latest user input has been processed by the platform layer
    //
    // This function is expected to update the state of the game
    // and draw the frame that will be displayed, ideally, some constant
    // amount of time in the future.
	DEBUG_ASSERT(sizeof(GameState) <= memory->permanentStorageSize);

	GameState *gameState = (GameState*)memory->permanentStorage;
    if (memory->DEBUGShouldInitGlobalFuncs) {
	    // Initialize global function names
#if GAME_SLOW
        debugPrint_ = platformFuncs->DEBUGPlatformPrint;
#endif
        #define FUNC(returntype, name, ...) name = \
        platformFuncs->glFunctions.name;
            GL_FUNCTIONS_BASE
            GL_FUNCTIONS_ALL
        #undef FUNC

        memory->DEBUGShouldInitGlobalFuncs = false;
    }
	if (!memory->isInitialized) {
        glClearColor(0.0f, 0.0f, 0.1f, 0.0f);
        // Very explicit depth testing setup (DEFAULT VALUES)
        // NDC is left-handed with this setup
        // (very subtle left-handedness definition:
        //  front objects have z = -1, far objects have z = 1)
        glEnable(GL_DEPTH_TEST);
        // Nearer objects have less z than farther objects
        glDepthFunc(GL_LESS);
        // Depth buffer clears to farthest z-value (1)
        glClearDepth(1.0);
        // Depth buffer transforms -1 to 1 range to 0 to 1 range
        glDepthRange(0.0, 1.0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Initialize audio state
        gameState->audioState.runningSampleIndex = 0;
        gameState->audioState.amplitude = 0.1f;
        gameState->audioState.tSine1 = 0.0f;
        gameState->audioState.tSine2 = 0.0f;
#if GAME_INTERNAL
        gameState->audioState.debugView = false;
#endif

		gameState->pos = Vec3::zero;

		// Isometric angle to which completely upward-facing
		// 90 degree angles get transformed
		gameState->angle = 2.0f * PI_F / 3.0f;

        gameState->lineGL = InitLineGL(thread,
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);
        gameState->textGL = InitTextGL(thread,
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);

        FT_Error error = FT_Init_FreeType(&gameState->ftLibrary);
        if (error) {
            DEBUG_PRINT("FreeType init error: %d\n", error);
        }
        gameState->fontFace = LoadFontFace(thread, gameState->ftLibrary,
            "data/fonts/computer-modern/serif.ttf", 18,
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);

		// TODO this may be more appropriate to do in the platform layer
		memory->isInitialized = true;
	}

	const GameControllerInput* input0 = &input->controllers[0];

	float32 speed = 0.01f;
	Vec3 playerRight = Normalize(Vec3{ 1.0f, 1.0f, 0.0f });
	Vec3 playerForward = Normalize(Vec3{ -1.0f, 1.0f, 0.0f });
	Vec3 vel = Vec3::zero;
	if (input0->isConnected) {
		vel = (input0->leftEnd.x * playerRight
        + input0->leftEnd.y * playerForward) * speed;
	}
	if (input->keyboard[KM_KEY_D].isDown) {
		vel += playerRight * speed;
	}
	if (input->keyboard[KM_KEY_A].isDown) {
		vel -= playerRight * speed;
	}
	if (input->keyboard[KM_KEY_W].isDown) {
		vel += playerForward * speed;
	}
	if (input->keyboard[KM_KEY_S].isDown) {
		vel -= playerForward * speed;
	}
	gameState->pos += vel;

	//screenInfo.width;
	//screenInfo.height;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*if (input0->x.isDown) {
		gameState->angle -= 0.005f;
    }
	if (input0->b.isDown) {
		gameState->angle += 0.005f;
    }*/

	float isoZ = PI_F / 4.0f;
	float isoX = acosf(tanf((PI_F - gameState->angle) / 2.0f));
 	Quat rotationZ = QuatFromAngleUnitAxis(-isoZ, Vec3{ 0.0f, 0.0f, 1.0f });
	Quat rotationX = QuatFromAngleUnitAxis(-isoX, Vec3{ 1.0f, 0.0f, 0.0f });

	float zHalfRange = 10.0f;
	Mat4 proj = Scale(Vec3 {
        (float32)screenInfo.size.y / (float32)screenInfo.size.x,
		1.0f,
        1.0f / zHalfRange
    });
	Mat4 view = //Scale(Vec3{ 1.0f, 1.0f, 1.0f / zRange }) *
		UnitQuatToMat4(rotationX) *
		UnitQuatToMat4(rotationZ) *
		Translate(-gameState->pos) *
		Scale(Vec3 { 1.0f, 1.0f, -1.0f });

    proj = Mat4::one;
    view = Translate(-gameState->pos);

	//float32 boxSize = 0.1f;
	float32 boxGray = 0.5f;

	if (input->controllers[0].a.isDown) {
		boxGray = 1.0f;
    }

	/*Vec3 centeredPos = {
        gameState->pos.x - boxSize / 2.0f,
		gameState->pos.y - boxSize / 2.0f,
		0.01f
    };
	Mat4 playerMat = Translate(centeredPos)
        * Scale(Vec3{ boxSize, boxSize, 1.0f });
	DrawRect(thread, gameState->rectShader, gameState->rectVAO,
		proj * view * playerMat, boxGray, boxGray, boxGray, 1.0f);*/

    char fpsStr[128];
    sprintf(fpsStr, "FPS: %f", 1.0f / deltaTime);
    Vec2Int fpsPos = {
        screenInfo.size.x - 10,
        screenInfo.size.y - 10,
    };
    DrawText(gameState->textGL, gameState->fontFace, screenInfo,
        fpsStr, fpsPos, Vec2 { 1.0f, 1.0f }, Vec4::one);

    // Audio output
    float32 baseTone = 261.0f;
    float32 tone1Hz = baseTone
        * (1.0f + 0.5f * input->controllers[0].leftEnd.x)
        * (1.0f + 0.1f * input->controllers[0].leftEnd.y);
    float32 tone2Hz = baseTone
        * (1.0f + 0.5f * input->controllers[0].rightEnd.x)
        * (1.0f + 0.1f * input->controllers[0].rightEnd.y);

    AudioState* audioState = &gameState->audioState;
    if (input->controllers[0].x.isDown) {
        audioState->amplitude -= 0.01f;
    }
    if (input->controllers[0].b.isDown) {
        audioState->amplitude += 0.01f;
    }
    audioState->amplitude = ClampFloat32(audioState->amplitude, 0.0f, 1.0f);
    audioState->runningSampleIndex += audio->fillStartDelta;
    audioState->tSine1 += 2.0f * PI_F * tone1Hz
        * audio->fillStartDelta / audio->sampleRate;
    audioState->tSine2 += 2.0f * PI_F * tone2Hz
        * audio->fillStartDelta / audio->sampleRate;

    for (int i = 0; i < audio->fillLength; i++) {
        float32 tSine1Off = 2.0f * PI_F * tone1Hz
            * i / audio->sampleRate;
        float32 tSine2Off = 2.0f * PI_F * tone2Hz
            * i / audio->sampleRate;
        uint32 ind = (audio->fillStart + i) % audio->bufferSizeSamples;
        int16 sin1Sample = (int16)(INT16_MAXVAL * audioState->amplitude * sinf(
            audioState->tSine1 + tSine1Off));
        int16 sin2Sample = (int16)(INT16_MAXVAL * audioState->amplitude * sinf(
            audioState->tSine2 + tSine2Off));
        audio->buffer[ind * audio->channels]      = sin1Sample;
        audio->buffer[ind * audio->channels + 1]  = sin2Sample;
    }

#if GAME_INTERNAL
    if (WasKeyPressed(input, KM_KEY_G)) {
        audioState->debugView = !audioState->debugView;
    }
    if (audioState->debugView) {
        DEBUG_ASSERT(memory->transientStorageSize >= sizeof(LineGLData));
        DEBUG_ASSERT(audio->bufferSizeSamples <= MAX_LINE_POINTS);
        LineGLData* lineData = (LineGLData*)memory->transientStorage;
        lineData->count = audio->bufferSizeSamples;
        float32 length = 1.0f;
        float32 height = 1.0f;
        float32 offset = 1.0f;
        for (int i = 0; i < audio->bufferSizeSamples; i++) {
            int16 val = audio->buffer[i * audio->channels];
            float32 normVal = (float32)val / INT16_MAXVAL;
            float32 t = (float32)i / (audio->bufferSizeSamples - 1);
            lineData->pos[i] = {
                t * length - length / 2.0f,
                height * normVal + offset,
                0.0f
            };
        }
        DrawLine(gameState->lineGL, proj, view,
            lineData, Vec4::one);
        for (int i = 0; i < audio->bufferSizeSamples; i++) {
            int16 val = audio->buffer[i * audio->channels + 1];
            float32 normVal = (float32)val / INT16_MAXVAL;
            float32 t = (float32)i / (audio->bufferSizeSamples - 1);
            lineData->pos[i] = {
                t * length - length / 2.0f,
                height * normVal - offset,
                0.0f
            };
        }
        DrawLine(gameState->lineGL, proj, view,
            lineData, Vec4::one);
    }
#endif
}

#include "km_input.cpp"
#include "opengl_base.cpp"
#include "text.cpp"