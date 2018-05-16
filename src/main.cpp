#include "main.h"

#include "main_platform.h"
#include "km_debug.h"
#include "km_defines.h"
#include "km_input.h"
#include "km_math.h"
#include "opengl.h"
#include "opengl_funcs.h"
#include "opengl_base.h"

global_var Vec4 backgroundColor_ = Vec4 { 0.0f, 0.0f, 0.0f, 0.0f };
global_var Vec4 lineColor_ = Vec4 { 0.9f, 0.9f, 0.9f, 1.0f };
global_var Vec4 lineDarkColor_ = Vec4 { 0.25f, 0.25f, 0.25f, 1.0f };

//global_var Vec4 circleIdleColor_ = lineColor_;
global_var Vec4 circleSelectedColor_ = Vec4 { 0.6f, 1.0f, 0.7f, 1.0f };

MarkerGL InitMarkerGL(const ThreadContext* thread,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory)
{
    MarkerGL markerGL;
    const GLfloat vertices[] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f,
        0.5f, 0.5f,
        -0.5f, 0.5f,
        -0.5f, -0.5f
    };

    glGenVertexArrays(1, &markerGL.vertexArray);
    glBindVertexArray(markerGL.vertexArray);

    glGenBuffers(1, &markerGL.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, markerGL.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // match shader layout location
        2, // size (vec2)
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0, // stride
        (void*)0 // array buffer offset
    );

    glBindVertexArray(0);

    markerGL.programID = LoadShaders(thread,
        "shaders/marker.vert", "shaders/marker.frag",
        DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
    
    return markerGL;
}

CircleGL InitCircleGL(const ThreadContext* thread,
    DEBUGPlatformReadFileFunc* DEBUGPlatformReadFile,
    DEBUGPlatformFreeFileMemoryFunc* DEBUGPlatformFreeFileMemory)
{
    CircleGL circleGL;
    const GLfloat vertices[] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f,
        0.5f, 0.5f,
        -0.5f, 0.5f,
        -0.5f, -0.5f
    };

    glGenVertexArrays(1, &circleGL.vertexArray);
    glBindVertexArray(circleGL.vertexArray);

    glGenBuffers(1, &circleGL.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, circleGL.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // match shader layout location
        2, // size (vec2)
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0, // stride
        (void*)0 // array buffer offset
    );

    glBindVertexArray(0);

    circleGL.programID = LoadShaders(thread,
        "shaders/circle.vert", "shaders/circle.frag",
        DEBUGPlatformReadFile, DEBUGPlatformFreeFileMemory);
    
    return circleGL;
}

void DrawMarker(MarkerGL markerGL, ScreenInfo screenInfo,
    Vec2Int pos, Vec2Int size, Vec4 color)
{
    RectCoordsNDC ndc = ToRectCoordsNDC(pos, size, screenInfo);

    GLint loc;
    glUseProgram(markerGL.programID);
    loc = glGetUniformLocation(markerGL.programID, "posCenter");
    glUniform3fv(loc, 1, &ndc.pos.e[0]);
    loc = glGetUniformLocation(markerGL.programID, "size");
    glUniform2fv(loc, 1, &ndc.size.e[0]);
    loc = glGetUniformLocation(markerGL.programID, "color");
    glUniform4fv(loc, 1, &color.e[0]);

    glBindVertexArray(markerGL.vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void DrawCircle(CircleGL circleGL, ScreenInfo screenInfo,
    Vec2Int pos, Vec2Int size, Vec4 color)
{
    RectCoordsNDC ndc = ToRectCoordsNDC(pos, size, screenInfo);

    GLint loc;
    glUseProgram(circleGL.programID);
    loc = glGetUniformLocation(circleGL.programID, "posCenter");
    glUniform3fv(loc, 1, &ndc.pos.e[0]);
    loc = glGetUniformLocation(circleGL.programID, "size");
    glUniform2fv(loc, 1, &ndc.size.e[0]);
    loc = glGetUniformLocation(circleGL.programID, "color");
    glUniform4fv(loc, 1, &color.e[0]);

    glBindVertexArray(circleGL.vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

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
        // Very explicit depth testing setup (DEFAULT VALUES)
        // NDC is left-handed with this setup
        // (very subtle left-handedness definition:
        //  front objects have z = -1, far objects have z = 1)
        glEnable(GL_DEPTH_TEST);
        // Nearer objects have less z than farther objects
        glDepthFunc(GL_LEQUAL);
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

        gameState->circlePos = 0;
		gameState->debugCamPos = Vec3::zero;

        gameState->rectGL = InitRectGL(thread,
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);
        gameState->texturedRectGL = InitTexturedRectGL(thread,
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);
        gameState->lineGL = InitLineGL(thread,
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);
        gameState->textGL = InitTextGL(thread,
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);

        gameState->markerGL = InitMarkerGL(thread,
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);
        gameState->circleGL = InitCircleGL(thread,
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);

        FT_Error error = FT_Init_FreeType(&gameState->ftLibrary);
        if (error) {
            DEBUG_PRINT("FreeType init error: %d\n", error);
        }
        gameState->fontFace = LoadFontFace(thread, gameState->ftLibrary,
            "data/fonts/source-code-pro/regular.ttf", 24,
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);

        glGenFramebuffers(NUM_FRAMEBUFFERS, gameState->framebuffers);
        for (int i = 0; i < NUM_FRAMEBUFFERS; i++) {
            gameState->colorBuffers[i] = 0;
        }

        const GLfloat vertices[] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            1.0f, 1.0f,
            1.0f, 1.0f,
            -1.0f, 1.0f,
            -1.0f, -1.0f
        };
        const GLfloat uvs[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f
        };

        glGenVertexArrays(1, &gameState->screenQuadVertexArray);
        glBindVertexArray(gameState->screenQuadVertexArray);

        glGenBuffers(1, &gameState->screenQuadVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, gameState->screenQuadVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
            GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, // match shader layout location
            2, // size (vec2)
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0, // stride
            (void*)0 // array buffer offset
        );

        glGenBuffers(1, &gameState->screenQuadUVBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, gameState->screenQuadUVBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, // match shader layout location
            2, // size (vec2)
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0, // stride
            (void*)0 // array buffer offset
        );

        glBindVertexArray(0);

        gameState->screenShader = LoadShaders(thread,
            "shaders/screen.vert", "shaders/screen.frag",
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);
        gameState->bloomExtractShader = LoadShaders(thread,
            "shaders/screen.vert", "shaders/bloomExtract.frag",
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);
        gameState->bloomBlendShader = LoadShaders(thread,
            "shaders/screen.vert", "shaders/bloomBlend.frag",
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);
        gameState->blurShader = LoadShaders(thread,
            "shaders/screen.vert", "shaders/blur.frag",
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);
        gameState->grainShader = LoadShaders(thread,
            "shaders/screen.vert", "shaders/grain.frag",
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);
        gameState->fxaaShader = LoadShaders(thread,
            "shaders/screen.vert", "shaders/fxaa.frag",
            platformFuncs->DEBUGPlatformReadFile,
            platformFuncs->DEBUGPlatformFreeFileMemory);

		// TODO this may be more appropriate to do in the platform layer
		memory->isInitialized = true;
	}
    if (screenInfo.changed) {
        if (gameState->colorBuffers[0] != 0) {
            glDeleteTextures(NUM_FRAMEBUFFERS, gameState->colorBuffers);
        }
        glGenTextures(NUM_FRAMEBUFFERS, gameState->colorBuffers);

        for (int i = 0; i < NUM_FRAMEBUFFERS; i++) {
            glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                screenInfo.size.x, screenInfo.size.y,
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                NULL
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D, gameState->colorBuffers[i], 0);
        }

        DEBUG_PRINT("Updated screen-size dependent info\n");
    }

    if (WasKeyPressed(input, KM_KEY_A)
    || WasKeyPressed(input, KM_KEY_ARROW_LEFT)) {
        gameState->circlePos--;
    }
    if (WasKeyPressed(input, KM_KEY_D)
    || WasKeyPressed(input, KM_KEY_ARROW_RIGHT)) {
        gameState->circlePos++;
    }
    if (gameState->circlePos < 0) {
        gameState->circlePos = 0;
    }
    if (gameState->circlePos > 11) {
        gameState->circlePos = 11;
    }

    // Debug camera position control
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
	gameState->debugCamPos += vel;

    // ------------------------- Begin Rendering -------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[0]);
    //glDisable(GL_DEPTH_TEST);
    glClearColor(backgroundColor_.r, backgroundColor_.g,
        backgroundColor_.b, backgroundColor_.a);
	glClear(GL_COLOR_BUFFER_BIT);

    int lineWidth = screenInfo.size.y / 140;
    float32 slotWidthPix = screenInfo.size.x / 12.0f;
    int circleDiameter = (int)(slotWidthPix * 0.6f);

    for (int i = 0; i < 12; i++) {
        Vec2Int slotLinePos = {
            (int)(slotWidthPix * i),
            screenInfo.size.y / 2
        };
        Vec4 color = lineColor_;
        if (i == 1 || i == 3 || i == 6 || i == 8 || i == 10) {
            // Black keys on piano
            color = lineDarkColor_;
        }
        DrawRect(gameState->rectGL, screenInfo,
            slotLinePos,
            Vec2 { 0.0f, 0.5f },
            Vec2Int { (int)slotWidthPix, lineWidth },
            color
        );
    }
    for (int i = 0; i < 13; i++) {
        Vec2Int markerPos = {
            (int)(slotWidthPix * i),
            screenInfo.size.y / 2
        };
        DrawMarker(gameState->markerGL, screenInfo,
            markerPos,
            Vec2Int { lineWidth, lineWidth * 6 },
            lineColor_
        );
    }

    Vec2Int circlePos = {
        (int)(slotWidthPix * gameState->circlePos + slotWidthPix / 2),
        screenInfo.size.y / 2
    };
    DrawCircle(gameState->circleGL, screenInfo,
        circlePos,
        Vec2Int { circleDiameter, circleDiameter },
        circleSelectedColor_
    );

    // Post processing passes
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    GLint loc;

    /*Vec2 invScreenSize = {
        1.0f / screenInfo.size.x,
        1.0f / screenInfo.size.y
    };
    loc = glGetUniformLocation(gameState->bloomExtractShader,
        "invScreenSize");
    glUniform2fv(loc, 1, &invScreenSize.e[0]);*/

    // -------------------- BLOOM --------------------
    // Extract high-luminance pixels
    glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[1]);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(gameState->screenQuadVertexArray);
    glUseProgram(gameState->bloomExtractShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[0]);
    loc = glGetUniformLocation(gameState->bloomExtractShader,
        "framebufferTexture");
    glUniform1i(loc, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Blur high-luminance pixels
    const int BLUR_PASSES = 15;
    for (int i = 0; i < BLUR_PASSES; i++) {  
        // Horizontal pass
        glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[2]);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(gameState->screenQuadVertexArray);
        glUseProgram(gameState->blurShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[1]);
        loc = glGetUniformLocation(gameState->blurShader,
            "framebufferTexture");
        glUniform1i(loc, 0);
        loc = glGetUniformLocation(gameState->blurShader,
            "isHorizontal");
        glUniform1i(loc, 1);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Vertical pass
        glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[1]);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(gameState->screenQuadVertexArray);
        glUseProgram(gameState->blurShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[2]);
        loc = glGetUniformLocation(gameState->blurShader,
            "framebufferTexture");
        glUniform1i(loc, 0);
        loc = glGetUniformLocation(gameState->blurShader,
            "isHorizontal");
        glUniform1i(loc, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // Blend pass
    glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[2]);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(gameState->screenQuadVertexArray);
    glUseProgram(gameState->bloomBlendShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[0]);
    loc = glGetUniformLocation(gameState->bloomBlendShader,
        "scene");
    glUniform1i(loc, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[1]);
    loc = glGetUniformLocation(gameState->bloomBlendShader,
        "bloomBlur");
    glUniform1i(loc, 1);
    loc = glGetUniformLocation(gameState->bloomBlendShader,
        "bloomMag");
    glUniform1f(loc, 0.4f);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // -------------------- GRAIN --------------------
    // nothing to see here for now

    // --------------------RENDER TO SCREEN --------------------
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(gameState->screenQuadVertexArray);
    glUseProgram(gameState->screenShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[2]);
    loc = glGetUniformLocation(gameState->screenShader,
        "framebufferTexture");
    glUniform1i(loc, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // -------------------------- End Rendering --------------------------

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
    //if (!input->controllers[0].isConnected) {
        float32 screenHalfWidth = screenInfo.size.x / 2.0f;
        float32 screenHalfHeight = screenInfo.size.y / 2.0f;
        float32 mouseOffsetX = (input->mousePos.x - screenHalfWidth)
            / screenHalfWidth;
        float32 mouseOffsetY = (input->mousePos.y - screenHalfHeight)
            / screenHalfHeight;
        tone1Hz = baseTone * (1.0f + 0.5f * mouseOffsetX);
        tone2Hz = baseTone * (1.0f + 0.5f * mouseOffsetY);
    //}

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

        audio->buffer[ind * audio->channels]      = 0;
        audio->buffer[ind * audio->channels + 1]  = 0;
    }

#if GAME_INTERNAL
    if (WasKeyPressed(input, KM_KEY_G)) {
        audioState->debugView = !audioState->debugView;
    }
    if (audioState->debugView) {
        DEBUG_ASSERT(memory->transientStorageSize >= sizeof(LineGLData));
        DEBUG_ASSERT(audio->bufferSizeSamples <= MAX_LINE_POINTS);
        Mat4 proj = Mat4::one;
        Mat4 view = Translate(-gameState->debugCamPos);

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