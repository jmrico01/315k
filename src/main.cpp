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
#include "halfedge.h"
#include "filters.h"

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
            printf("TODO: ESC detected, application should exit...");
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

void UseFaceNormals(Button* button, void* data)
{
    useSmoothNormals_ = false;
    printf("Now rendering with face normals\n");
}
void UseVertexNormals(Button* button, void* data)
{
    useSmoothNormals_ = true;
    printf("Now rendering with vertex normals\n");
}

void ClearSelection(Button* button, void* data)
{
    SharedState* state = (SharedState*)data;
    state->selectedVerts.Clear();
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
    
    const char* MODEL_START = "cube.obj";
    //const char* MODEL_START = "tetrahedron.obj";
    { // Manually add model loading filter
        FilterEntry modelLoadFilter;
        modelLoadFilter.idx = -1;
        modelLoadFilter.name = "Load 3D Model (.obj)";
        modelLoadFilter.updateFunc = FilterUpdateModelLoad;
        modelLoadFilter.applyFunc = FilterModelLoad;

        ModelLoadData* modelLoadData =
            (ModelLoadData*)malloc(sizeof(ModelLoadData));
        modelLoadData->inputField = CreateInputField(
            Vec2::zero, Vec2::zero,
            MODEL_START,
            { 1.0f, 1.0f, 1.0f, 0.2f },
            { 1.0f, 1.0f, 1.0f, 0.4f },
            { 1.0f, 1.0f, 1.0f, 0.5f },
            { 1.0f, 1.0f, 1.0f, 0.9f }
        );
        modelLoadFilter.data = (void*)modelLoadData;

        filters_.Append(modelLoadFilter);
    }

    DynamicArray<Button> miscButtons;
    { // Create other UI buttons
        const char* faceText = "Use Face Normals";
        miscButtons.Append(CreateButton(
            { 500.0f, 10.0f },
            {
                (float32)GetTextWidth(cmSerif16, faceText),
                (float32)cmSerif16.height
            },
            faceText, UseFaceNormals,
            { 1.0f, 1.0f, 0.0f, 0.2f },
            { 1.0f, 1.0f, 0.0f, 0.5f },
            { 1.0f, 1.0f, 0.0f, 0.6f },
            { 0.8f, 0.8f, 0.8f, 1.0f }
        ));
        const char* vertexText = "Use Vertex Normals";
        miscButtons.Append(CreateButton(
            { 500.0f, 10.0f + cmSerif16.height + 10.0f },
            {
                (float32)GetTextWidth(cmSerif16, vertexText),
                (float32)cmSerif16.height
            },
            vertexText, UseVertexNormals,
            { 1.0f, 1.0f, 0.0f, 0.2f },
            { 1.0f, 1.0f, 0.0f, 0.5f },
            { 1.0f, 1.0f, 0.0f, 0.6f },
            { 0.8f, 0.8f, 0.8f, 1.0f }
        ));
        const char* clearSelectText = "Clear Selection";
        miscButtons.Append(CreateButton(
            { 800.0f, 10.0f },
            {
                (float32)GetTextWidth(cmSerif16, clearSelectText),
                (float32)cmSerif16.height
            },
            clearSelectText, ClearSelection,
            { 1.0f, 1.0f, 0.0f, 0.2f },
            { 1.0f, 1.0f, 0.0f, 0.5f },
            { 1.0f, 1.0f, 0.0f, 0.6f },
            { 0.8f, 0.8f, 0.8f, 1.0f }
        ));
    }

    // Create buttons for adding filters
    DynamicArray<Button> filterButtons;
    for (int i = 0; i < numFilters_; i++) {
        Vec2 origin = { 10.0f, 10.0f + 1.1f * cmSerif.height * i };
        Vec2 size = {
            (float32)GetTextWidth(cmSerif, filterInfo_[i].name),
            (float32)cmSerif.height
        };
        filterButtons.Append(CreateButton(origin, size,
            filterInfo_[i].name, filterInfo_[i].callback,
            { 1.0f, 1.0f, 0.0f, 0.2f },
            { 1.0f, 1.0f, 0.0f, 0.5f },
            { 1.0f, 1.0f, 0.0f, 0.6f },
            { 0.8f, 0.8f, 0.8f, 1.0f }
        ));
    }

    SharedState state;

    state.mesh = HalfEdgeMeshFromObj(MODEL_START);
    if (state.mesh.vertices.size == 0) {
        printf("not loaded\n");
    }
    state.meshGL = LoadHalfEdgeMeshGL(state.mesh, useSmoothNormals_);

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

    state.cameraPos = { 0.0f, 0.0f, DEFAULT_CAM_Z };
    state.modelRot =
        QuatFromAngleUnitAxis(PI_F / 6.0f, Vec3::unitX)
        * QuatFromAngleUnitAxis(-PI_F / 4.0f, Vec3::unitY);

    int clickStatePrev = clickState_;
    int enterState = GLFW_RELEASE, enterStatePrev = GLFW_RELEASE;
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

        const float ZOOM_STEP = 0.95f;
        if ((clickState_ & CLICKSTATE_LEFT_PRESS) != 0) {
            Vec2 dMouse = mousePos - mousePosPrev;
            float speed = 0.01f;
            state.modelRot =
                QuatFromAngleUnitAxis(dMouse.x * speed, Vec3::unitY)
                * QuatFromAngleUnitAxis(-dMouse.y * speed, Vec3::unitX)
                * state.modelRot;
            state.modelRot = Normalize(state.modelRot);
        }
        state.cameraPos.z = DEFAULT_CAM_Z * powf(ZOOM_STEP, scrollCumY_);

        // 3D rendering start
        glEnable(GL_DEPTH_TEST);

        Mat4 proj = Projection(110.0f, (float32)width_ / (float32)height_,
            0.1f, 10.0f);
        //Mat4 view = Translate(-state.cameraPos) * Rotate(state.modelRotation);
        Mat4 view = Translate(-state.cameraPos)
            * UnitQuatToMat4(state.modelRot);
        DrawHalfEdgeMeshGL(state.meshGL, proj, view);

        // Draw axis lines
        const float AXIS_LINE_LEN = 20.0f;
        for (int i = 0; i < 3; i++) {
            Vec3 v1 = Vec3::zero;
            v1.e[i] = -AXIS_LINE_LEN / 2.0f;
            Vec3 v2 = Vec3::zero;
            v2.e[i] = AXIS_LINE_LEN / 2.0f;
            Vec4 color = { 0.2f, 0.2f, 0.2f, 1.0f };

            color.e[i] = 0.4f;
            DrawLine(lineGL, proj, view, v1, Vec3::zero, color);
            color.e[i] = 1.0f;
            DrawLine(lineGL, proj, view, Vec3::zero, v2, color);
        }

        glDisable(GL_DEPTH_TEST);
        // 3D rendering end

        { // Mesh vertex selection
            DynamicArray<MouseCastFaceOut> mouseCastFaces;
            MouseCastMeshFaces(state.mesh, mousePos,
                proj, view, width_, height_,
                mouseCastFaces);
            
            uint32 closest = mouseCastFaces.size;
            float closestDist = 1000.0f;
            for (uint32 i = 0; i < mouseCastFaces.size; i++) {
                if (mouseCastFaces[i].dist < closestDist) {
                    closestDist = mouseCastFaces[i].dist;
                    closest = i;
                }
            }

            if (closest != mouseCastFaces.size) {
                bool add = false;
                if ((clickState_ & CLICKSTATE_LEFT_PRESS) != 0
                && (clickStatePrev & CLICKSTATE_LEFT_PRESS) == 0) {
                    add = true;
                }

                Mat4 toScreen = HomogeneousToScreen();
                uint32 edge = state.mesh.faces[
                    mouseCastFaces[closest].face].halfEdge;
                uint32 e = edge;
                do {
                    uint32 v = state.mesh.halfEdges[e].vertex;
                    Vec3 vWorld = state.mesh.vertices[v].pos;
                    Vec4 vWorld4 = { vWorld.x, vWorld.y, vWorld.z, 1.0f };
                    Vec4 vScreen4 = proj * view * vWorld4;
                    vScreen4 /= vScreen4.w;
                    vScreen4 = toScreen * vScreen4;
                    Vec3 vScreen = { vScreen4.x, vScreen4.y, vScreen4.z };

                    DrawRect(rectGL, vScreen, { 0.5f, 0.5f }, { 6.0f, 6.0f },
                        { 1.0f, 0.0f, 1.0f, 0.6f });
                    if (add) {
                        bool present = false;
                        for (uint32 i = 0; i < state.selectedVerts.size; i++) {
                            if (state.selectedVerts[i] == v) {
                                present = true;
                                break;
                            }
                        }
                        if (!present) {
                            state.selectedVerts.Append(v);
                        }
                    }

                    e = state.mesh.halfEdges[e].next;
                } while (e != edge);
            }

            Mat4 toScreen = HomogeneousToScreen();
            for (uint32 i = 0; i < state.selectedVerts.size; i++) {
                Vec3 vWorld = state.mesh.vertices[state.selectedVerts[i]].pos;
                Vec4 vWorld4 = { vWorld.x, vWorld.y, vWorld.z, 1.0f };
                Vec4 vScreen4 = proj * view * vWorld4;
                vScreen4 /= vScreen4.w;
                vScreen4 = toScreen * vScreen4;
                Vec3 vScreen = { vScreen4.x, vScreen4.y, vScreen4.z };

                DrawRect(rectGL, vScreen, { 0.5f, 0.5f }, { 6.0f, 6.0f },
                    { 1.0f, 0.0f, 1.0f, 1.0f });
            }
        }

        bool applyFilters = false;

        uint32 nfPrev = filters_.size;
        UpdateButtons(filterButtons.data, filterButtons.size,
            mousePos, clickState_, &state);
        DrawButtons(filterButtons.data, filterButtons.size,
            rectGL, textGL, cmSerif);
        if (nfPrev != filters_.size) {
            applyFilters = true;
        }

        bool smoothNormalsPrev = useSmoothNormals_;
        UpdateButtons(miscButtons.data, miscButtons.size,
            mousePos, clickState_, &state);
        DrawButtons(miscButtons.data, miscButtons.size,
            rectGL, textGL, cmSerif16);
        if (smoothNormalsPrev != useSmoothNormals_) {
            applyFilters = true;
        }

        Vec3 filterOrigin = { 150.0f, 10.0f, 0.0f };
        DrawText(textGL, cmSerif16, "Press Enter to reapply all filters",
            filterOrigin, { 0.0f, 0.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f });
        filterOrigin.y += (float32)cmSerif16.height;
        for (uint32 i = 0; i < filters_.size; i++) {
            Vec3 filterPos = filterOrigin;
            filterPos.y += 3.0f * (float32)cmSerif16.height * i;
            filters_[i].updateFunc(&filters_[i], filterPos,
                rectGL, textGL, cmSerif16,
                mousePos, clickState_, keyInputBuffer, keyInputBufferSize);
        }

        if (filtersToDelete_.size > 0) {
            for (uint32 i = 0; i < filtersToDelete_.size; i++) {
                FilterEntry entry = filters_[filtersToDelete_[i]];
                if (filtersToDelete_[i] != entry.idx) {
                    printf("ERROR: filter indices didn't match on deletion\n");
                }

                printf("Removing filter #%d: %s\n", entry.idx, entry.name);
                for (uint32 j = entry.idx + 1; j < filters_.size; j++) {
                    filters_[j].idx -= 1;
                }
                // Free memory
                if (entry.data != nullptr) {
                    free(entry.data);
                }
                filters_.Remove(entry.idx);
            }

            filtersToDelete_.Clear();
            applyFilters = true;
        }

        enterStatePrev = enterState;
        enterState = glfwGetKey(window, GLFW_KEY_ENTER);
        if (applyFilters
        || (enterState == GLFW_PRESS && enterStatePrev == GLFW_RELEASE)) {
            printf("=> Reapplying filter list...\n");
            // Re-load mesh first
            filters_[0].applyFunc(&filters_[0], &state);
            state.selectedVerts.Clear();
            for (uint32 i = 1; i < filters_.size; i++) {
                state.allVerts.Clear();
                for (uint32 v = 0; v < state.mesh.vertices.size; v++) {
                    state.allVerts.Append(v);
                }
                filters_[i].applyFunc(&filters_[i], &state);
            }
            printf("-> Reloading mesh into OpenGL...\n");
            FreeHalfEdgeMeshGL(&state.meshGL);
            state.meshGL = LoadHalfEdgeMeshGL(state.mesh, useSmoothNormals_);
            printf("=> Done.\n\n");
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
#include "halfedge.cpp"
#include "filters.cpp"

#include "glew.c"
//#undef internal
//#undef local_persist
//#undef global_var
//#include "lodepng.cpp"