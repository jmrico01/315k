#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "km_defines.h"
#include "km_types.h"
#include "km_math.h"
#include "ogl_base.h"
#include "text.h"
#include "gui.h"
#include "load_bmp.h"

#define DEFAULT_CAM_Z 4.0f

// TODO (on linux at least) the window's top bar is part
// of the viewport, so the render scene is cropped.

// TODO get rid of these globals... getting tired of writing that
global_var char pathToApp_[PATH_MAX_LENGTH];
global_var int width_ = 1024;
global_var int height_ = 768;
global_var int clickState_ = CLICKSTATE_NONE;
global_var float scrollCumY_ = 0.0f;

global_var bool useSmoothNormals_ = false;

global_var KeyEvent keyInputBuffer[KEY_INPUT_BUFFER_MAX];
global_var uint32 keyInputBufferSize = 0;

internal void RemoveFileNameFromPath(
    char* filePath, char* dest, uint64 destLength)
{
    unsigned int lastSlash = 0;
#ifdef _WIN32
    char pathSep = '\\';
#else
    char pathSep = '/';
#endif
    while (filePath[lastSlash] != '\0') {
        lastSlash++;
    }
    // TODO unsafe!
    while (filePath[lastSlash] != pathSep) {
        lastSlash--;
    }
    if (lastSlash + 2 > destLength) {
        return;
    }
    for (unsigned int i = 0; i < lastSlash + 1; i++) {
        dest[i] = filePath[i];
    }
    dest[lastSlash + 1] = '\0';
}

internal void ErrorCallback(int err, const char* msg)
{
    printf("GLFW error code %d: %s\n", err, msg);
}

// NOTE: for now, I'm assuming input callbacks occur on the same thread
// as the main loop. have to research that.
internal void CharModsCallback(GLFWwindow* window,
    uint32 codepoint, int mods)
{
    KeyEvent keyEvent = {
        (char)codepoint,
        true
    };

    keyInputBuffer[keyInputBufferSize++] = keyEvent;
}
internal void KeyCallback(GLFWwindow* window,
    int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE) {
        if (action == GLFW_PRESS) {
            // TODO exit
            printf("TODO: ESC detected, application should exit...\n");
        }
    }

    if (key == GLFW_KEY_BACKSPACE) {
        KeyEvent keyEvent;
        keyEvent.ascii = 8;
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            keyEvent.pressed = true;
        }
        else if (action == GLFW_RELEASE) {
            keyEvent.pressed = false;
        }
        else {
            return;
        }

        keyInputBuffer[keyInputBufferSize++] = keyEvent;
    }
}
internal void MouseCallback(GLFWwindow* window,
    int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            clickState_ |= CLICKSTATE_LEFT_PRESS;
        }
        else if (action == GLFW_RELEASE) {
            clickState_ &= ~CLICKSTATE_LEFT_PRESS;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            clickState_ |= CLICKSTATE_RIGHT_PRESS;
        }
        else if (action == GLFW_RELEASE) {
            clickState_ &= ~CLICKSTATE_RIGHT_PRESS;
        }
    }
}
internal void ScrollCallback(GLFWwindow* window,
    double xOffset, double yOffset)
{
    scrollCumY_ += (float)yOffset;
}

internal void FramebufferSizeCallback(
    GLFWwindow* window, int width, int height)
{
    width_ = width;
    height_ = height;
    ResizeGL(width, height);
}

char* GetAppPath()
{
    return pathToApp_;
}

// TODO this is kind of quick and dirty.
void CatStrings(const char* str1, const char* str2, char* dest, uint64 destLen)
{
    int i = 0;
    unsigned int iDest = 0;

    if (destLen == 0) return;

    while (str1[i] != '\0') {
        if (iDest >= destLen - 1) {
            dest[destLen - 1] = '\0';
            return;
        }
        dest[iDest++] = str1[i++];
    }
    i = 0;
    while (str2[i] != '\0') {
        if (iDest >= destLen - 1) {
            dest[destLen - 1] = '\0';
            return;
        }
        dest[iDest++] = str2[i++];
    }

    dest[iDest] = '\0'; // this might be unsafe? don't wanna think about it
}

ReadFileResult ReadFile(const char* path)
{
    ReadFileResult result = {};
    char fullPath[PATH_MAX_LENGTH];
    CatStrings(GetAppPath(), path, fullPath, PATH_MAX_LENGTH);
    FILE *f = fopen(fullPath, "rb");
    if (!f) {
        printf("Couldn't open file: %s\n", path);
        return result;
    }
    fseek(f, 0, SEEK_END);
    result.size = (uint64)ftell(f);
    fseek(f, 0, SEEK_SET);

    result.data = malloc((size_t)(result.size));
    fread(result.data, (size_t)result.size, 1, f);
    fclose(f);

    return result;
}

Mat4 HomogeneousToScreen()
{
    return Scale({ (float32)width_, (float32)height_, 1.0f })
        * Translate({ 0.5f, 0.5f, 0.0f }) * Scale({ 0.5f, 0.5f, 1.0f });
}

int main(int argc, char* argv[])
{
    RemoveFileNameFromPath(argv[0], pathToApp_, PATH_MAX_LENGTH);
    printf("Path to C application: %s\n", pathToApp_);

    // Initialize GLFW.
    if (glfwInit() != GLFW_TRUE) {
        return 1;
    }
    // TODO do this only in debug build?
    glfwSetErrorCallback(&ErrorCallback);
    
    // Try to get OpenGL 3.3 context on window creation.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Create GLFW window.
    GLFWwindow* window = glfwCreateWindow(
        width_, height_, "315K", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    printf("GLFW: Created window\n");

    // Set window's GL context as the current context.
    glfwMakeContextCurrent(window);
    // Initialize GLEW for OpenGL 3.3 procedures.
    glewExperimental = true;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        glfwTerminate();
        printf("GLEW: Error - %s\n", glewGetErrorString(err));
    }
    printf("GLEW: Initialized\n");

    // Verify that we obtained an OpenGL 3.3+ context.
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    int glVerMajor, glVerMinor;
    glGetIntegerv(GL_MAJOR_VERSION, &glVerMajor);
    glGetIntegerv(GL_MINOR_VERSION, &glVerMinor);
    if (glVerMajor < 3 || (glVerMajor == 3 && glVerMinor < 3)) {
        glfwTerminate();
        printf("Requires OpenGL 3.3 or later.\n");
        printf("If your hardware supports it, update your drivers.\n");
        return 1;
    }
    printf("GLSL version:   %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("GL renderer:    %s\n", glGetString(GL_RENDERER));
    
    // Initialize app-specific OpenGL.
    // TODO error checking?
    InitOpenGL();
    ResizeGL(width_, height_);
    
    FT_Library library;
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        printf("FreeType init error: %d\n", error);
        return 1;
    }

    // NOTE this isn't like "create a rect instance", but more like
    // "initialize rect drawing in general"
    RectGL rectGL = CreateRectGL();
    //TexturedRectGL texturedRectGL = CreateTexturedRectGL();
    TextGL textGL = CreateTextGL();
    LineGL lineGL = CreateLineGL();

    //GLuint textureKM = OpenGLLoadBMP("data/images/kapricorn.bmp");
    FontFace cmSerif16 = LoadFontFace(
        library, "data/fonts/computer-modern/serif.ttf", 16);
    FontFace cmSerif = LoadFontFace(
        library, "data/fonts/computer-modern/serif.ttf", 24);
    /*FontFace cmSerif32 = LoadFontFace(
        library, "data/fonts/computer-modern/serif.ttf", 32);
    FontFace cmSerifBold = LoadFontFace(
        library, "data/fonts/computer-modern/serif-bold.ttf", 48);
    FontFace cmSerifBold128 = LoadFontFace(
        library, "data/fonts/computer-modern/serif-bold.ttf", 128);*/

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    // Set up input handling
    glfwSetCharModsCallback(window, &CharModsCallback);
    glfwSetKeyCallback(window, &KeyCallback);
    glfwSetMouseButtonCallback(window, &MouseCallback);
    glfwSetScrollCallback(window, &ScrollCallback);

    // Catch all GL errors before loop
    {
        GLenum glError;
        while ((glError = glGetError()) != GL_NO_ERROR) {
            printf("GL ERROR: %x\n", glError);
        }
    }

    Vec2 mousePos = Vec2::zero, mousePosPrev = Vec2::zero;

    int clickStatePrev = clickState_;
    printf("\n");

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        { // Get mouse position
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            mouseY = (double)height_ - mouseY;
            mousePosPrev = mousePos;
            mousePos = { (float)mouseX, (float)mouseY };
        }

        // Clear all key events
        keyInputBufferSize = 0;

        clickStatePrev = clickState_;
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Catch all GL errors during game loop
        {
            GLenum glError;
            while ((glError = glGetError()) != GL_NO_ERROR) {
                printf("GL ERROR: %x\n", glError);
            }
        }
    }

    glfwTerminate();
    return 0;
}

#include "km_lib.cpp"
#include "ogl_base.cpp"
#include "text.cpp"
#include "load_bmp.cpp"
#include "gui.cpp"

#include "glew.c"
//#undef internal
//#undef local_persist
//#undef global_var
//#include "lodepng.cpp"