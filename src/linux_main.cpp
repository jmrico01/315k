#include "linux_main.h"

#include <sys/mman.h>       // memory functions
#include <sys/stat.h>       // file stat functions
#include <fcntl.h>          // file open/close functions
#include <unistd.h>         // symbolic links & other
#include <dlfcn.h>          // dynamic linking functions
//#include <dirent.h>
//#include <sys/sysinfo.h>  // get_nprocs
//#include <sys/wait.h>     // waitpid
//#include <unistd.h>       // usleep
//#include <pthread.h>      // threading
//#include <time.h>         // CLOCK_MONOTONIC, clock_gettime
//#include <semaphore.h>    // sem_init, sem_wait, sem_post
//#include <alloca.h>       // alloca

#include <stdarg.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <alsa/asoundlib.h>

#include "km_debug.h"
#include "km_math.h"

#define SAMPLERATE 44100
#define AUDIO_BUFFER_SIZE_MILLISECONDS  1000

global_var char pathToApp_[LINUX_STATE_FILE_NAME_COUNT];
global_var bool32 running_;
//global_var bool32 GlobalRunning;
//global_var bool32 GlobalPause;
//global_var bool32 GlobalFullscreen;

// From km_debug.h
#if GAME_SLOW
DEBUGPlatformPrintFunc* debugPrint_;
#endif

// Required GLX functions
typedef GLXContext  glXCreateContextAttribsARBFunc(
    Display* display, GLXFBConfig config, GLXContext shareContext,
    Bool direct, const int* attribList);
global_var glXCreateContextAttribsARBFunc* glXCreateContextAttribsARB;
typedef void        glXSwapIntervalEXTFunc(
    Display* display, GLXDrawable drawable, int interval);
global_var glXSwapIntervalEXTFunc* glXSwapIntervalEXT;

#define LOAD_GLX_FUNCTION(name) \
    name = (name##Func*)glXGetProcAddress((const GLubyte*)#name)

global_var int linuxOpenGLAttribs[] =
{
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 3,
    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#if GAME_INTERNAL
        | GLX_CONTEXT_DEBUG_BIT_ARB
#endif
        ,
#if 0
    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#else
    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
#endif
    0,
};

enum
{
    _NET_WM_STATE_REMOVE = 0,
    _NET_WM_STATE_ADD = 1,
    _NET_WM_STATE_TOGGLE = 2
};

#if 0
platform_api Platform;

global_variable linux_state GlobalLinuxState;
global_variable bool32 GlobalSoftwareRendering;
global_variable bool32 GlobalRunning;
global_variable bool32 GlobalPause;
global_variable bool32 GlobalFullscreen;
global_variable linux_offscreen_buffer GlobalBackbuffer;
global_variable bool32 DEBUGGlobalShowCursor;
global_variable uint32 GlobalWindowPositionX;
global_variable uint32 GlobalWindowPositionY;
global_variable Cursor GlobalHiddenCursor;

typedef void   type_glDebugMessageCallbackARB(GLDEBUGPROC callback, const void *userParam);

typedef GLXContext type_glXCreateContextAttribsARB(Display *dpy, GLXFBConfig config, GLXContext shareContext,
                                                   Bool direct, const int *attribList);
typedef void   type_glXSwapIntervalEXT(Display *dpy, GLXDrawable drawable, int interval);

typedef const GLubyte *type_glGetStringi(GLenum name, GLuint index);

typedef void   type_glGenBuffers(GLsizei n, GLuint *buffers);
typedef void   type_glBindBuffer(GLenum target, GLuint buffer);
typedef void   type_glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void   type_glDrawBuffers(GLsizei n, const GLenum *bufs);

typedef void   type_glGenFramebuffers(GLsizei n, GLuint *framebuffer);
typedef GLenum type_glCheckFramebufferStatus(GLenum target);
typedef void   type_glBindFramebuffer(GLenum target, GLuint framebuffer);
typedef void   type_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void   type_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void   type_glDeleteFramebuffers(GLsizei n, const GLuint *framebuffer);

typedef GLuint type_glCreateShader(GLenum type);
typedef void   type_glShaderSource(GLuint shader, GLsizei count, GLchar **string, GLint *length);
typedef void   type_glCompileShader(GLuint shader);
typedef void   type_glAttachShader(GLuint program, GLuint shader);
typedef void   type_glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void   type_glDeleteShader(GLuint shader);

typedef GLuint type_glCreateProgram(void);
typedef void   type_glLinkProgram(GLuint program);
typedef void   type_glValidateProgram(GLuint program);
typedef void   type_glGetProgramiv(GLuint program, GLenum pname, GLint *params);
typedef void   type_glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void   type_glUseProgram(GLuint program);
typedef void   type_glDeleteProgram(GLuint program);

typedef GLint type_glGetUniformLocation(GLuint program, const GLchar *name);
typedef void type_glUniform1i(GLint location, GLint v0);
typedef void type_glUniform1f(GLint location, GLfloat v0);
typedef void type_glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
typedef void type_glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
typedef void type_glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
typedef void type_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

typedef void type_glGenVertexArrays(GLsizei n, GLuint *arrays);
typedef void type_glBindVertexArray(GLuint array);

typedef GLint type_glGetAttribLocation(GLuint program, const GLchar *name);
typedef void type_glEnableVertexAttribArray(GLuint index);
typedef void type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *offset);
typedef void type_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void type_glDisableVertexAttribArray(GLuint index);

typedef void type_glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);

#define GL_DEBUG_CALLBACK(Name) void Name(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)

#define OpenGLGlobalFunction(Name) global_variable type_##Name *Name;

OpenGLGlobalFunction(glDebugMessageCallbackARB);

OpenGLGlobalFunction(glXCreateContextAttribsARB);
OpenGLGlobalFunction(glGetStringi);

OpenGLGlobalFunction(glGenBuffers);
OpenGLGlobalFunction(glBindBuffer);
OpenGLGlobalFunction(glBufferData);
OpenGLGlobalFunction(glDrawBuffers);

OpenGLGlobalFunction(glGenFramebuffers);
OpenGLGlobalFunction(glCheckFramebufferStatus);
OpenGLGlobalFunction(glBindFramebuffer);
OpenGLGlobalFunction(glFramebufferTexture2D);
OpenGLGlobalFunction(glBlitFramebuffer);
OpenGLGlobalFunction(glDeleteFramebuffers);

OpenGLGlobalFunction(glCreateShader);
OpenGLGlobalFunction(glShaderSource);
OpenGLGlobalFunction(glCompileShader);
OpenGLGlobalFunction(glAttachShader);
OpenGLGlobalFunction(glGetShaderInfoLog);
OpenGLGlobalFunction(glDeleteShader);

OpenGLGlobalFunction(glCreateProgram);
OpenGLGlobalFunction(glLinkProgram);
OpenGLGlobalFunction(glValidateProgram);
OpenGLGlobalFunction(glGetProgramiv);
OpenGLGlobalFunction(glGetProgramInfoLog);
OpenGLGlobalFunction(glUseProgram);
OpenGLGlobalFunction(glDeleteProgram);

OpenGLGlobalFunction(glGetUniformLocation);
OpenGLGlobalFunction(glUniform1i);
OpenGLGlobalFunction(glUniform1f);
OpenGLGlobalFunction(glUniform2fv);
OpenGLGlobalFunction(glUniform3fv);
OpenGLGlobalFunction(glUniform4fv);
OpenGLGlobalFunction(glUniformMatrix4fv);

OpenGLGlobalFunction(glGenVertexArrays);
OpenGLGlobalFunction(glBindVertexArray);

OpenGLGlobalFunction(glGetAttribLocation);
OpenGLGlobalFunction(glEnableVertexAttribArray);
OpenGLGlobalFunction(glVertexAttribPointer);
OpenGLGlobalFunction(glVertexAttribIPointer);
OpenGLGlobalFunction(glDisableVertexAttribArray);

OpenGLGlobalFunction(glTexImage2DMultisample);
#endif

// NOTE(michiel): Explicit wrappers around dlsym, dlopen and dlclose
internal void* LinuxLoadFunction(void* libHandle, const char* name)
{
    void* symbol = dlsym(libHandle, name);
    if (!symbol) {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
    }
    // TODO(michiel): Check if lib with underscore exists?!
    return symbol;
}

internal void* LinuxLoadLibrary(const char *libName)
{
    void* handle = dlopen(libName, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
    }
    return handle;
}

internal void LinuxUnloadLibrary(void *handle)
{
    if (handle != NULL) {
        dlclose(handle);
        handle = NULL;
    }
}

internal int StringLength(const char* string)
{
	int length = 0;
	while (*string++) {
		length++;
    }

	return length;
}
internal void CatStrings(
	size_t sourceACount, const char* sourceA,
	size_t sourceBCount, const char* sourceB,
	size_t destCount, char* dest)
{
	for (size_t i = 0; i < sourceACount; i++) {
		*dest++ = *sourceA++;
    }

	for (size_t i = 0; i < sourceBCount; i++) {
		*dest++ = *sourceB++;
    }

	*dest++ = '\0';
}
internal inline bool32 IsWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}
internal bool32 StringsAreEqual(
    const char* str1, size_t strLen1,
    const char* str2, size_t strLen2)
{
    if (strLen1 != strLen2) {
        return false;
    }

    size_t i = 0;
    while (i < strLen1) {
        if (str1[i] != str2[i]) {
            return false;
        }
        i++;
    }

    return true;
}

internal inline uint32 SafeTruncateUInt64(uint64 value)
{
	// TODO defines for maximum values
	DEBUG_ASSERT(value <= 0xFFFFFFFF);
	uint32 result = (uint32)value;
	return result;
}

internal void RemoveFileNameFromPath(
    const char* filePath, char* dest, uint64 destLength)
{
    unsigned int lastSlash = 0;
	// TODO confused... some cross-platform code inside a win32 file
	//	maybe I meant to pull this out sometime?
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

internal void LinuxGetEXEFileName(LinuxState *state)
{
    ssize_t numRead = readlink("/proc/self/exe",
        state->exeFilePath, ARRAY_COUNT(state->exeFilePath) - 1);
    state->exeFilePath[numRead] = '\0';

    if (numRead > 0) {
        state->exeOnePastLastSlash = state->exeFilePath;
        for (char* scan = state->exeFilePath; *scan != '\0'; scan++) {
            if (*scan == '/') {
                state->exeOnePastLastSlash = scan + 1;
            }
        }
    }
}

internal void LinuxBuildEXEPathFileName(
    LinuxState *state, const char *fileName,
    int dstLen, char *dst)
{
    CatStrings(
        state->exeOnePastLastSlash - state->exeFilePath, state->exeFilePath,
        StringLength(fileName), fileName,
        dstLen, dst);
}

internal inline ino_t LinuxFileId(char *FileName)
{
    struct stat Attr = {};
    if (stat(FileName, &Attr)) {
        Attr.st_ino = 0;
    }

    return Attr.st_ino;
}

#if GAME_INTERNAL

DEBUG_PLATFORM_PRINT_FUNC(DEBUGPlatformPrint)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

}

DEBUG_PLATFORM_FREE_FILE_MEMORY_FUNC(DEBUGPlatformFreeFileMemory)
{
    if (file->data) {
        munmap(file->data, file->size);
        file->data = 0;
    }
    file->size = 0;
}

DEBUG_PLATFORM_READ_FILE_FUNC(DEBUGPlatformReadFile)
{
    DEBUGReadFileResult result = {};

    int32 fileHandle = open(fileName, O_RDONLY);
    if (fileHandle >= 0) {
        off_t fileSize64 = lseek(fileHandle, 0, SEEK_END);
        lseek(fileHandle, 0, SEEK_SET);

        if (fileSize64 > 0) {
            uint32 fileSize32 = SafeTruncateUInt64(fileSize64);
            result.data = mmap(NULL, fileSize32, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (result.data) {
                ssize_t bytesRead = read(fileHandle,
                    result.data, result.size);
                if ((ssize_t)fileSize32 == bytesRead) {
                    // NOTE(casey): File read successfully
                    result.size = fileSize32;
                }
                else {
                    // TODO(michiel): Logging
                    DEBUGPlatformFreeFileMemory(thread, &result);
                }
            }
        }
        else {
            // TODO(michiel): Logging
        }

        close(fileHandle);
    }
    else {
        // TODO(casey): Logging
    }

    return result;
}

DEBUG_PLATFORM_WRITE_FILE_FUNC(DEBUGPlatformWriteFile)
{
    bool32 result = false;

    int32 fileHandle = open(fileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fileHandle >= 0) {
        ssize_t bytesWritten = write(fileHandle, memory, memorySize);
        if (fsync(fileHandle) >= 0) {
            result = (bytesWritten == (ssize_t)memorySize);
        }
        else {
            // TODO(casey): Logging
        }

        close(fileHandle);
    }
    else {
        // TODO(casey): Logging
    }

    return result;
}

#endif

// Dynamic code loading
internal bool32 LinuxLoadGameCode(
    LinuxGameCode* gameCode, const char* DLLName, ino_t FileID)
{
    if (gameCode->GameLibID != FileID) {
        LinuxUnloadLibrary(gameCode->GameLibHandle);
        gameCode->GameLibID = FileID;
        gameCode->IsValid = false;

        gameCode->GameLibHandle = LinuxLoadLibrary(DLLName);
        if (gameCode->GameLibHandle) {
            *(void **)(&gameCode->gameUpdateAndRender) = LinuxLoadFunction(
                gameCode->GameLibHandle, "GameUpdateAndRender");
        }
    }

    if (!gameCode->IsValid) {
        LinuxUnloadLibrary(gameCode->GameLibHandle);
        gameCode->GameLibID = 0;
        gameCode->gameUpdateAndRender = 0;
    }

    return gameCode->IsValid;
}

internal void LinuxUnloadGameCode(LinuxGameCode *GameCode)
{
    LinuxUnloadLibrary(GameCode->GameLibHandle);
    GameCode->GameLibID = 0;
    GameCode->IsValid = false;
    GameCode->gameUpdateAndRender = 0;
}

internal bool32 LinuxLoadGLXExtensions()
{
    Display* tempDisplay = XOpenDisplay(NULL);
    if (!tempDisplay) {
        DEBUG_PRINT("Failed to open display on GLX extension load\n");
        return false;
    }

    int dummy;
    if (!glXQueryExtension(tempDisplay, &dummy, &dummy)) {
        DEBUG_PRINT("GLX extension query failed\n");
        return false;
    }

    int displayBufferAttribs[] = {
        GLX_RGBA,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER,
        None
    };
    XVisualInfo* visuals = glXChooseVisual(tempDisplay,
        DefaultScreen(tempDisplay), displayBufferAttribs);
    if (!visuals) {
        DEBUG_PRINT("Failed to choose GLX visual\n");
        return false;
    }

    visuals->screen = DefaultScreen(tempDisplay);

    XSetWindowAttributes attribs = {};
    Window root = RootWindow(tempDisplay, visuals->screen);
    attribs.colormap = XCreateColormap(tempDisplay, root, visuals->visual,
        AllocNone);

    Window glWindow = XCreateWindow(tempDisplay, root,
        0, 0,
        10, 10,
        0, visuals->depth, InputOutput, visuals->visual,
        CWColormap, &attribs);
    if (!glWindow) {
        DEBUG_PRINT("Failed to create dummy GL window\n");
        return false;
    }

    GLXContext context = glXCreateContext(tempDisplay, visuals, NULL, true);
    if (!glXMakeCurrent(tempDisplay, glWindow, context)) {
        DEBUG_PRINT("Failed to make GLX context current\n");
        return false;
    }

    char* extensions = (char*)glXQueryExtensionsString(tempDisplay,
        visuals->screen);
    //DEBUG_PRINT("Supported extensions: %s\n", extensions);
    char* at = extensions;
    while (*at) {
        while (IsWhitespace(*at)) {
            at++;
        }
        char* end = at;
        while (*end && !IsWhitespace(*end)) {
            end++;
        }
        //size_t count = end - at;

        /*const char* srgbFramebufferExtName =
            "GLX_EXT_framebuffer_sRGB";
        const char* srgbFramebufferArbName =
            "GLX_ARB_framebuffer_sRGB";
        if (0) {
        }
        else if (StringsAreEqual(at, count,
        srgbFramebufferExtName, StringLength(srgbFramebufferExtName))) {
            //OpenGL.SupportsSRGBFramebuffer = true;
        }
        else if (StringsAreEqual(at, count,
        srgbFramebufferArbName, StringLength(srgbFramebufferArbName))) {
            //OpenGL.SupportsSRGBFramebuffer = true;
        }*/

        at = end;
    }

    glXMakeCurrent(tempDisplay, None, NULL);

    glXDestroyContext(tempDisplay, context);
    XDestroyWindow(tempDisplay, glWindow);

    XFreeColormap(tempDisplay, attribs.colormap);
    XFree(visuals);

    XCloseDisplay(tempDisplay);

    return true;
}

internal GLXFBConfig* LinuxGetOpenGLFramebufferConfig(Display *display)
{
    int visualAttribs[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, True,
        //GLX_SAMPLE_BUFFERS  , 1,
        //GLX_SAMPLES         , 4,
        None
    };

    /*if (!OpenGL.SupportsSRGBFramebuffer) {
        visualAttribs[22] = None;
    }*/

    int framebufferCount;
    GLXFBConfig* framebufferConfig = glXChooseFBConfig(
        display, DefaultScreen(display), visualAttribs, &framebufferCount);
    DEBUG_ASSERT(framebufferCount >= 1);

    return framebufferConfig;
}

#define LOAD_GL_FUNCTION(name) \
    glFuncs->name = (name##Func*)glXGetProcAddress((const GLubyte*)#name); \
    if (!glFuncs->name) { \
        DEBUG_PANIC("OpenGL function load failed: %s", #name); \
    }

internal bool32 LinuxLoadBaseGLFunctions(OpenGLFunctions* glFuncs)
{
	// Generate function loading code
#define FUNC(returntype, name, ...) LOAD_GL_FUNCTION(name);
	GL_FUNCTIONS_BASE
#undef FUNC

    return true;
}

internal bool32 LinuxLoadAllGLFunctions(OpenGLFunctions* glFuncs)
{
	// Generate function loading code
#define FUNC(returntype, name, ...) LOAD_GL_FUNCTION(name);
	GL_FUNCTIONS_ALL
#undef FUNC

    return true;
}

internal bool32 LinuxInitOpenGL(
    OpenGLFunctions* glFuncs,
    Display* display, GLXDrawable glWindow,
    int width, int height)
{
    if (!LinuxLoadBaseGLFunctions(glFuncs)) {
        DEBUG_PRINT("Failed to load base GL functions\n");
        return false;
    }

	glFuncs->glViewport(0, 0, width, height);

	// Set v-sync
    LOAD_GLX_FUNCTION(glXSwapIntervalEXT);
    if (glXSwapIntervalEXT) {
        glXSwapIntervalEXT(display, glWindow, 1);
    }
    else {
        // TODO no vsync. logging? just exit? just exit for now
        DEBUG_PRINT("Failed to load glXSwapIntervalEXT (no vsync)\n");
        //return false;
    }

	const GLubyte* vendorString = glFuncs->glGetString(GL_VENDOR);
	DEBUG_PRINT("GL_VENDOR: %s\n", vendorString);
	const GLubyte* rendererString = glFuncs->glGetString(GL_RENDERER);
	DEBUG_PRINT("GL_RENDERER: %s\n", rendererString);
	const GLubyte* versionString = glFuncs->glGetString(GL_VERSION);
	DEBUG_PRINT("GL_VERSION: %s\n", versionString);

	int32 majorVersion = versionString[0] - '0';
	int32 minorVersion = versionString[2] - '0';

	if (majorVersion < 3 || (majorVersion == 3 && minorVersion < 3)) {
		// TODO logging. opengl version is less than 3.3
		return false;
	}

	if (!LinuxLoadAllGLFunctions(glFuncs)) {
        DEBUG_PRINT("Failed to load all GL functions\n");
		return false;
	}

	const GLubyte* glslString =
        glFuncs->glGetString(GL_SHADING_LANGUAGE_VERSION);
    DEBUG_PRINT("GL_SHADING_LANGUAGE_VERSION: %s\n", glslString);

    return true;
}

//
// NOTE(michiel): X11 Window requests and drawing
//

#if 0
internal v2i
LinuxGetWindowDimension()
{

}
#endif

internal void LinuxProcessKeyboardMessage(
    GameButtonState* newState, bool32 isDown)
{
    if (newState->isDown != isDown) {
        newState->isDown = isDown;
        newState->transitions++;
    }
}

internal void LinuxGetInputFileLocation(
    LinuxState* state, bool32 inputStream,
    int32 slotInd, int32 dstCount, char *dst)
{
    char temp[64];
    snprintf(temp, sizeof(temp), "loop_edit_%d_%s.hmi", slotInd, inputStream ? "input" : "state");
    LinuxBuildEXEPathFileName(state, temp, dstCount, dst);
}

internal LinuxWindowDimension LinuxGetWindowDimension(
    Display* display, Window window)
{
    XWindowAttributes WindowAttribs = {};
    XGetWindowAttributes(display, window, &WindowAttribs);

    LinuxWindowDimension result = {};
    result.Width = WindowAttribs.width;
    result.Height = WindowAttribs.height;
    return result;
}

#if 0
internal void ToggleFullscreen(Display *display, Window window)
{
    GlobalFullscreen = !GlobalFullscreen;
    Atom FullscreenAtom = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    Atom WindowState = XInternAtom(display, "_NET_WM_STATE", False);
    int32 Mask = SubstructureNotifyMask | SubstructureRedirectMask;
    // int32 Mask = StructureNotifyMask | ResizeRedirectMask;
    XEvent event = {};
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    // event.xclient.display = display;
    event.xclient.window = window;
    event.xclient.message_type = WindowState;
    event.xclient.format = 32;
    event.xclient.data.l[0] = (GlobalFullscreen ?
        _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE); // set (2 is toggle)
    event.xclient.data.l[1] = (long)FullscreenAtom;
    event.xclient.data.l[2] = 0;

    XSendEvent(display, DefaultRootWindow(display), False, Mask, &event);
    // XFlush(display);
}
#endif

internal inline Vec2 LinuxGetMousePosition(Display* display, Window window)
{
    Window RetRoot, RetWin;
    int32 RootX, RootY;
    int32 WinX, WinY;
    uint32 Mask;
    bool32 querySuccess = XQueryPointer(display, window,
        &RetRoot, &RetWin,
        &RootX, &RootY, &WinX, &WinY, &Mask);

    // TODO initializing this with empty brackets crashes GCC... wtf
    Vec2 result = { 0.0f, 0.0f };
    if (querySuccess) {
        result.x = (float32)WinX;
        result.y = (float32)WinY;
    }
    return result;
}

#if 0
internal void LinuxVerifyMemoryListIntegrity()
{
    BeginTicketMutex(&GlobalLinuxState.MemoryMutex);
    local_persist uint32 FailCounter;
    LinuxMemoryBlock *Sentinel = &GlobalLinuxState.MemorySentinel;
    for (LinuxMemoryBlock *SourceBlock = Sentinel->Next;
    SourceBlock != Sentinel;
    SourceBlock = SourceBlock->Next) {
        Assert(SourceBlock->Block.Size <= U32Max);
    }
    FailCounter++;
    EndTicketMutex(&GlobalLinuxState.MemoryMutex);
}

internal void LinuxFreeMemoryBlock(LinuxMemoryBlock *Block)
{
    uint32 Size = Block->Block.Size;
    uint64 Flags = Block->Block.Flags;
    umm PageSize = sysconf(_SC_PAGESIZE);
    umm TotalSize = Size + sizeof(LinuxMemoryBlock);
    if(Flags & PlatformMemory_UnderflowCheck) {
        TotalSize = Size + 2*PageSize;
    }
    else if(Flags & PlatformMemory_OverflowCheck) {
        umm SizeRoundedUp = AlignPow2(Size, PageSize);
        TotalSize = SizeRoundedUp + 2*PageSize;
    }

    BeginTicketMutex(&GlobalLinuxState.MemoryMutex);
    Block->Prev->Next = Block->Next;
    Block->Next->Prev = Block->Prev;
    EndTicketMutex(&GlobalLinuxState.MemoryMutex);

    munmap(Block, TotalSize);
}

internal void
LinuxClearBlocksByMask(linux_state *State, u64 Mask)
{
    for(LinuxMemoryBlock *BlockIter = State->MemorySentinel.Next;
        BlockIter != &State->MemorySentinel;
        )
    {
        LinuxMemoryBlock *Block = BlockIter;
        BlockIter = BlockIter->Next;

        if((Block->LoopingFlags & Mask) == Mask)
        {
            LinuxFreeMemoryBlock(Block);
        }
        else
        {
            Block->LoopingFlags = 0;
        }
    }
}
#endif

inline bool32 LinuxIsInLoop(LinuxState *state)
{
    bool32 result = ((state->InputRecordingIndex != 0)
        || (state->InputPlayingIndex));
    return result;
}

#if 0
PLATFORM_ALLOCATE_MEMORY(LinuxAllocateMemory)
{
    // NOTE(casey): We require memory block headers not to change the cache
    // line alignment of an allocation
    DEBUG_ASSERT(sizeof(LinuxMemoryBlock) == 64);

    umm PageSize = sysconf(_SC_PAGESIZE);
    umm TotalSize = Size + sizeof(LinuxMemoryBlock);
    umm BaseOffset = sizeof(LinuxMemoryBlock);
    umm ProtectOffset = 0;
    if (Flags & PlatformMemory_UnderflowCheck) {
        TotalSize = Size + 2*PageSize;
        BaseOffset = 2*PageSize;
        ProtectOffset = PageSize;
    }
    else if (Flags & PlatformMemory_OverflowCheck) {
        umm SizeRoundedUp = AlignPow2(Size, PageSize);
        TotalSize = SizeRoundedUp + 2*PageSize;
        BaseOffset = PageSize + SizeRoundedUp - Size;
        ProtectOffset = PageSize + SizeRoundedUp;
    }

    LinuxMemoryBlock *Block = (LinuxMemoryBlock *)mmap(0, TotalSize,
        PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    DEBUG_ASSERT(Block);
    Block->Block.Base = (u8 *)Block + BaseOffset;
    DEBUG_ASSERT(Block->Block.Used == 0);
    DEBUG_ASSERT(Block->Block.ArenaPrev == 0);

    if (Flags & (PlatformMemory_UnderflowCheck|PlatformMemory_OverflowCheck)) {
        int32 Error = mprotect((u8 *)Block + ProtectOffset, PageSize, PROT_NONE);
        DEBUG_ASSERT(Error == 0);
    }

    LinuxMemoryBlock *Sentinel = &GlobalLinuxState.MemorySentinel;
    Block->Next = Sentinel;
    Block->Block.Size = Size;
    Block->Block.Flags = Flags;
    Block->LoopingFlags = LinuxIsInLoop(&GlobalLinuxState) ? LinuxMem_AllocatedDuringLooping : 0;

    BeginTicketMutex(&GlobalLinuxState.MemoryMutex);
    Block->Prev = Sentinel->Prev;
    Block->Prev->Next = Block;
    Block->Next->Prev = Block;
    EndTicketMutex(&GlobalLinuxState.MemoryMutex);

    platform_memory_block *PlatBlock = &Block->Block;
    return(PlatBlock);
}

PLATFORM_DEALLOCATE_MEMORY(LinuxDeallocateMemory)
{
    if(Block)
    {
        LinuxMemoryBlock *LinuxBlock = ((LinuxMemoryBlock *)Block);
        if(LinuxIsInLoop(&GlobalLinuxState))
        {
            LinuxBlock->LoopingFlags = LinuxMem_FreedDuringLooping;
        }
        else
        {
            LinuxFreeMemoryBlock(LinuxBlock);
        }
    }
}

#endif

//
// Replays
//

#if 0
internal void LinuxBeginRecordingInput(
    LinuxState *state, int InputRecordingIndex)
{
    // TODO(michiel): mmap to file?
    char FileName[LINUX_STATE_FILE_NAME_COUNT];
    LinuxGetInputFileLocation(state, true, InputRecordingIndex, sizeof(FileName), FileName);
    state->RecordingHandle = open(FileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (state->RecordingHandle >= 0)
    {
        //ssize_t BytesWritten = write(FileHandle, Memory, MemorySize);
        ssize_t BytesWritten;

        state->InputRecordingIndex = InputRecordingIndex;
        LinuxMemoryBlock *Sentinel = state->MemorySentinel;

        BeginTicketMutex(state->MemoryMutex);
        for (LinuxMemoryBlock *SourceBlock = Sentinel->Next;
            SourceBlock != Sentinel;
            SourceBlock = SourceBlock->Next)
        {
            if (!(SourceBlock->Block.Flags & PlatformMemory_NotRestored))
            {
                linux_saved_memory_block DestBlock;
                void *BasePointer = SourceBlock->Block.Base;
                DestBlock.BasePointer = (u64)BasePointer;
                DestBlock.Size = SourceBlock->Block.Size;
                BytesWritten = write(state->RecordingHandle, &DestBlock, sizeof(DestBlock));
                DEBUG_ASSERT(DestBlock.Size <= U32Max);
                BytesWritten = write(state->RecordingHandle, BasePointer, (uint32)DestBlock.Size);
            }
        }
        EndTicketMutex(state->MemoryMutex);

        linux_saved_memory_block DestBlock = {};
        BytesWritten = write(state->RecordingHandle, &DestBlock, sizeof(DestBlock));
    }
}

internal void LinuxEndRecordingInput(LinuxState *state)
{
    fsync(state->RecordingHandle);
    close(state->RecordingHandle);
    state->InputRecordingIndex = 0;
}

internal void LinuxBeginInputPlayBack(
    LinuxState *state, int InputPlayingIndex)
{
    //LinuxClearBlocksByMask(state, LinuxMem_AllocatedDuringLooping);

    char FileName[LINUX_STATE_FILE_NAME_COUNT];
    LinuxGetInputFileLocation(state, true, InputPlayingIndex, sizeof(FileName), FileName);
    state->PlaybackHandle = open(FileName, O_RDONLY);
    if (state->PlaybackHandle >= 0) {
        state->InputPlayingIndex = InputPlayingIndex;

        for (;;) {
            linux_saved_memory_block Block = {};
            ssize_t BytesRead = read(state->PlaybackHandle, &Block, sizeof(Block));
            if (Block.BasePointer != 0) {
                void *BasePointer = (void *)Block.BasePointer;
                DEBUG_ASSERT(Block.Size <= UINT32_MAX);
                BytesRead = read(state->PlaybackHandle, BasePointer, (uint32)Block.Size);
            }
            else {
                break;
            }
        }
        // TODO(casey): Stream memory in from the file!
    }
}

internal void LinuxEndInputPlayBack(LinuxState *state)
{
    LinuxClearBlocksByMask(state, LinuxMem_FreedDuringLooping);
    close(state->PlaybackHandle);
    state->InputPlayingIndex = 0;
}

internal void LinuxRecordInput(LinuxState *state, game_input *NewInput)
{
    write(state->RecordingHandle, NewInput, sizeof(*NewInput));
}

internal void
LinuxPlayBackInput(linux_state *State, game_input *NewInput)
{
    ssize_t BytesRead = read(State->PlaybackHandle, NewInput, sizeof(*NewInput));
    if (BytesRead == 0) {
        // NOTE(casey): We've hit the end of the stream, go back to the beginning
        int32 PlayingIndex = State->InputPlayingIndex;
        LinuxEndInputPlayBack(State);
        LinuxBeginInputPlayBack(State, PlayingIndex);
        read(State->PlaybackHandle, NewInput, sizeof(*NewInput));
    }
}
#endif

//
// NOTE(michiel): Timing
//

#if 0
internal inline struct timespec LinuxGetWallClock()
{
    struct timespec clock;
    clock_gettime(CLOCK_MONOTONIC, &clock);
    return clock;
}

internal inline float32 LinuxGetSecondsElapsed(
    struct timespec start, struct timespec end)
{
    return (float32)(end.tv_sec - start.tv_sec)
        + ((float32)(end.tv_nsec - start.tv_nsec) * 1e-9f);
}

// NOTE(michiel): Reference
global unsigned int GlobalX11Map[] = {
    XK_Home, XK_End, XK_KP_Divide, XK_KP_Multiply, XK_KP_Add, XK_KP_Subtract, XK_BackSpace, XK_Tab,
    XK_Return, XK_Linefeed, XK_Page_Up, XK_Page_Down,
    XK_KP_0, XK_KP_1, XK_KP_2, XK_KP_3, XK_KP_4, XK_KP_5, XK_KP_6, XK_KP_7, XK_KP_8, XK_KP_9,
    XK_Insert, XK_Escape, XK_KP_Decimal, XK_KP_Enter, XK_space, XK_quotedbl, XK_comma, XK_minus, XK_period, XK_slash,
    XK_0, XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7, XK_8, XK_9,
    XK_semicolon, XK_equal,
    XK_A, XK_B, XK_C, XK_D, XK_E, XK_F, XK_G, XK_H, XK_I, XK_J, XK_K, XK_L, XK_M,
    XK_N, XK_O, XK_P, XK_Q, XK_R, XK_S, XK_T, XK_U, XK_V, XK_W, XK_X, XK_Y, XK_Z,
    XK_bracketleft, XK_backslash, XK_bracketright, XK_grave,
    XK_Control_L, XK_Control_R, XK_Alt_L, XK_Alt_R, XK_Shift_L, XK_Shift_R, XK_Super_L, XK_Super_R,
    XK_F1, XK_F2, XK_F3, XK_F4, XK_F5, XK_F6, XK_F7, XK_F8, XK_F9, XK_F10, XK_F11, XK_F12,
    XK_Num_Lock, XK_Scroll_Lock, XK_Print, XK_Break,
    XK_Up, XK_Down, XK_Left, XK_Right, XK_Delete,
};
#endif

#if 0
internal void LinuxProcessPendingMessages(
    LinuxState *State, Display *display, Window window,
    Atom WmDeleteWindow,
    /*game_controller_input *KeyboardController,*/ GameInput* Input, Vec2 *MouseP,
    LinuxWindowDimension* Dimension)
{
    while (GlobalRunning && XPending(display)) {
        XEvent Event;
        XNextEvent(display, &Event);

        // NOTE(michiel): Don't skip the scroll key Events
        if (Event.type == ButtonRelease) {
            if ((Event.xbutton.button != 4) &&
            (Event.xbutton.button != 5) &&
            XEventsQueued(display, QueuedAfterReading)) {
                // NOTE(michiel): Skip the auto repeat key
                XEvent NextEvent;
                XPeekEvent(display, &NextEvent);
                if ((NextEvent.type == ButtonPress) &&
                (NextEvent.xbutton.time == Event.xbutton.time) &&
                (NextEvent.xbutton.button == Event.xbutton.button)) {
                    continue;
                }
            }
        }
        // NOTE(michiel): Skip the Keyboard
        if (Event.type == KeyRelease
        && XEventsQueued(display, QueuedAfterReading)) {
            XEvent NextEvent;
            XPeekEvent(display, &NextEvent);
            if ((NextEvent.type == KeyPress) &&
            (NextEvent.xbutton.time == Event.xbutton.time) &&
            (NextEvent.xbutton.button == Event.xbutton.button)) {
                continue;
            }
        }

        switch (Event.type) {
            case ConfigureNotify: {
                int32 W = Event.xconfigure.width;
                int32 H = Event.xconfigure.height;
                if ((Dimension->Width != W) || (Dimension->Height != H)) {
                    Dimension->Width = W;
                    Dimension->Height = H;
                }
            } break;
            case DestroyNotify: {
                GlobalRunning = false;
            } break;
            case ClientMessage: {
                if ((Atom)Event.xclient.data.l[0] == WmDeleteWindow) {
                    GlobalRunning = false;
                }
            } break;
            case MotionNotify: {
                MouseP->x = (float32)Event.xmotion.x;
                MouseP->y = (float32)Event.xmotion.y;
            } break;

            case ButtonRelease:
            case ButtonPress: {
                if (Event.xbutton.button == 1) {
                    LinuxProcessKeyboardMessage(&Input->MouseButtons[PlatformMouseButton_Left],
                                                Event.type == ButtonPress);
                }
                else if (Event.xbutton.button == 2) {
                    LinuxProcessKeyboardMessage(&Input->MouseButtons[PlatformMouseButton_Middle],
                                                Event.type == ButtonPress);
                }
                else if (Event.xbutton.button == 3) {
                    LinuxProcessKeyboardMessage(&Input->MouseButtons[PlatformMouseButton_Right],
                                                Event.type == ButtonPress);
                }
                else if (Event.xbutton.button == 4) {
                    ++(Input->MouseZ);
                }
                else if (Event.xbutton.button == 5) {
                    --(Input->MouseZ);
                }
                else if (Event.xbutton.button == 8) {
                    LinuxProcessKeyboardMessage(&Input->MouseButtons[PlatformMouseButton_Extended0],
                                                Event.type == ButtonPress);
                }
                else if (Event.xbutton.button == 9) {
                    LinuxProcessKeyboardMessage(&Input->MouseButtons[PlatformMouseButton_Extended1],
                                                Event.type == ButtonPress);
                    // } else {
                    //     printf("Uncaught button: %u\n", Event.xbutton.button);
                }
            } break;

            case KeyRelease:
            case KeyPress: {
                bool32 AltKeyWasDown = Event.xkey.state & KEYCODE_ALT_MASK;
                bool32 ShiftKeyWasDown = Event.xkey.state & KEYCODE_SHIFT_MASK;
                
                if (!GlobalPause) {
                    if (Event.xkey.keycode == KEYCODE_W) {
                        LinuxProcessKeyboardMessage(
                            &KeyboardController->MoveUp,
                            Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_A) {
                        LinuxProcessKeyboardMessage(
                            &KeyboardController->MoveLeft,
                            Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_S) {
                        LinuxProcessKeyboardMessage(
                            &KeyboardController->MoveDown,
                            Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_D) {
                        LinuxProcessKeyboardMessage(
                            &KeyboardController->MoveRight,
                            Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_Q) {
                        LinuxProcessKeyboardMessage(
                            &KeyboardController->LeftShoulder,
                            Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_E) {
                        LinuxProcessKeyboardMessage(
                            &KeyboardController->RightShoulder,
                            Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_UP) {
                        LinuxProcessKeyboardMessage(
                            &KeyboardController->ActionUp,
                            Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_DOWN) {
                        LinuxProcessKeyboardMessage(
                            &KeyboardController->ActionDown,
                            Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_LEFT) {
                        LinuxProcessKeyboardMessage(
                            &KeyboardController->ActionLeft,
                            Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_RIGHT) {
                        LinuxProcessKeyboardMessage(
                            &KeyboardController->ActionRight,
                            Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_ESCAPE) {
                        LinuxProcessKeyboardMessage(
                            &KeyboardController->Back,
                            Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_SPACE) {
                        LinuxProcessKeyboardMessage(
                            &KeyboardController->Start,
                            Event.type == KeyPress);
                    }
                }
                
                if ((Event.xkey.keycode == KEYCODE_SHIFT_L)
                || (Event.xkey.keycode == KEYCODE_SHIFT_R)) {
                    Input->ShiftDown = (Event.type == KeyPress);
                }
                else if ((Event.xkey.keycode == KEYCODE_ALT_L)
                || (Event.xkey.keycode == KEYCODE_ALT_R)) {
                    Input->AltDown = (Event.type == KeyPress);
                }
                else if ((Event.xkey.keycode == KEYCODE_CTRL_L)
                || (Event.xkey.keycode == KEYCODE_CTRL_R)) {
                    Input->ControlDown = (Event.type == KeyPress);
                }

#if GAME_INTERNAL
                else if (Event.xkey.keycode == KEYCODE_P) {
                    if (Event.type == KeyPress) {
                        GlobalPause = !GlobalPause;
                    }
                }
                else if (Event.xkey.keycode == KEYCODE_L) {
                    if (Event.type == KeyPress) {
                        if (AltKeyWasDown) {
                            LinuxBeginInputPlayBack(State, 1);
                        }
                        else {
                            if (State->InputPlayingIndex == 0) {
                                if (State->InputRecordingIndex == 0) {
                                    LinuxBeginRecordingInput(State, 1);
                                }
                                else {
                                    LinuxEndRecordingInput(State);
                                    LinuxBeginInputPlayBack(State, 1);
                                }
                            }
                            else {
                                LinuxEndInputPlayBack(State);
                            }
                        }
                    }
                }
#endif
                if (Event.type == KeyPress) {
                    if (Event.xkey.keycode == KEYCODE_PLUS) {
                        if (ShiftKeyWasDown) {
                            OpenGL.DebugLightBufferIndex += 1;
                        }
                        else {
                            OpenGL.DebugLightBufferTexIndex += 1;
                        }
                    }
                    else if (Event.xkey.keycode == KEYCODE_MINUS) {
                        if (ShiftKeyWasDown) {
                            OpenGL.DebugLightBufferIndex -= 1;
                        }
                        else {
                            OpenGL.DebugLightBufferTexIndex -= 1;
                        }
                    }
                    else if ((Event.xkey.keycode == KEYCODE_ENTER)
                    && AltKeyWasDown) {
                        ToggleFullscreen(display, window);
                    }
                    else if ((Event.xkey.keycode >= KEYCODE_F1)
                    && (Event.xkey.keycode <= KEYCODE_F10)) {
                        Input->FKeyPressed[Event.xkey.keycode - KEYCODE_F1 + 1] = true;
                    }
                    else if ((Event.xkey.keycode >= KEYCODE_F11)
                    && (Event.xkey.keycode <= KEYCODE_F12)) {
                        // NOTE(michiel): Because of X11 mapping we get to do the function keys in 2 steps :)
                        Input->FKeyPressed[Event.xkey.keycode - KEYCODE_F11 + 1] = true;
                    }
                }
            } break;

            default: {
            } break;
        }
    }
}
#endif

//
// NOTE(michiel): Platform file handling
//

#if 0
struct LinuxFindFile
{
    DIR* Dir;
    struct dirent *FileData;
};

struct linux_platform_file_handle
{
    int32 LinuxHandle;
};

struct linux_platform_file_group
{
    char Wildcard[6];
    bool32 FileAvailable;
    LinuxFindFile FindData;
};

enum pattern_match_flag
{
    PatternMatchFlag_None      = 0x00,
    PatternMatchFlag_MaySkip   = 0x01,
    PatternMatchFlag_Restarted = 0x02,
};

internal bool32
MatchPattern(char *Pattern, char *String)
{
    bool32 Result = false;
    uint32 Flags = PatternMatchFlag_None;
    char *P = Pattern;
    char *S = String;
    while (*S)
    {
        if (*P == '*')
        {
            Flags = PatternMatchFlag_MaySkip;
            ++P;
        }
        else if (Flags == PatternMatchFlag_MaySkip)
        {
            Result = true;
            if (*S == *P)
            {
                Flags = PatternMatchFlag_None;
                ++P;
                ++S;
            }
            else
            {
                Flags = PatternMatchFlag_MaySkip;
                ++S;
            }
        }
        else if (*S != *P)
        {
            if (Flags == PatternMatchFlag_Restarted)
            {
                break;
            }
            Result = false;
            Flags = PatternMatchFlag_Restarted;
            P = Pattern;
        }
        else
        {
            Result = true;
            Flags = PatternMatchFlag_None;
            ++P;
            ++S;
        }
    }
    return Result && (*P == 0);
}

internal bool32
FindFileInFolder(char *Wildcard, LinuxFindFile *Finder)
{
    bool32 Result = false;
    if (Finder->Dir)
    {
        struct dirent *FileData = readdir(Finder->Dir);
        while (FileData)
        {
            if (MatchPattern(Wildcard, FileData->d_name))
            {
                Result = true;
                Finder->FileData = FileData;
                break;
            }
            FileData = readdir(Finder->Dir);
        }
        if (!Result)
        {
            Finder->FileData = 0;
        }
    }
    return Result;
}
#endif

#if 0
internal PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(LinuxGetAllFilesOfTypeBegin)
{
    platform_file_group Result = {};

    // TODO(casey): If we want, someday, make an actual arena used by Linux
    linux_platform_file_group *LinuxFileGroup = (linux_platform_file_group *)mmap(
        NULL, sizeof(linux_platform_file_group),
        PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    Result.Platform = LinuxFileGroup;

    LinuxFileGroup->Wildcard[0] = '*';
    LinuxFileGroup->Wildcard[1] = '.';
    LinuxFileGroup->Wildcard[2] = 'h';
    LinuxFileGroup->Wildcard[3] = 'h';
    switch(Type)
    {
        case PlatformFileType_AssetFile:
        {
            LinuxFileGroup->Wildcard[4] = 'a';
        } break;

        case PlatformFileType_SavedGameFile:
        {
            LinuxFileGroup->Wildcard[4] = 's';
        } break;

        InvalidDefaultCase;
    }

    Result.FileCount = 0;

    LinuxFindFile FindData;
    FindData.Dir = opendir(".");
    for (bool32 Found = FindFileInFolder(LinuxFileGroup->Wildcard, &FindData);
         Found;
         Found = FindFileInFolder(LinuxFileGroup->Wildcard, &FindData))
    {
        ++Result.FileCount;
    }
    closedir(FindData.Dir);

    LinuxFileGroup->FindData.Dir = opendir(".");
    LinuxFileGroup->FileAvailable = FindFileInFolder(LinuxFileGroup->Wildcard, &LinuxFileGroup->FindData);

    return(Result);
}

internal PLATFORM_GET_ALL_FILE_OF_TYPE_END(LinuxGetAllFilesOfTypeEnd)
{
    linux_platform_file_group *LinuxFileGroup = (linux_platform_file_group *)FileGroup->Platform;
    if(LinuxFileGroup)
    {
        if (LinuxFileGroup->FindData.Dir)
        {
            closedir(LinuxFileGroup->FindData.Dir);
            LinuxFileGroup->FindData.Dir = 0;
        }

        munmap(LinuxFileGroup, sizeof(linux_platform_file_group));
        LinuxFileGroup = 0;
    }
}

internal PLATFORM_OPEN_FILE(LinuxOpenNextFile)
{
    linux_platform_file_group *LinuxFileGroup = (linux_platform_file_group *)FileGroup->Platform;
    platform_file_handle Result = {};

    if(LinuxFileGroup->FileAvailable)
    {
        // TODO(casey): If we want, someday, make an actual arena used by Linux
        linux_platform_file_handle *LinuxHandle = (linux_platform_file_handle *)mmap(
            NULL, sizeof(linux_platform_file_handle),
            PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        Result.Platform = LinuxHandle;

        if(LinuxHandle)
        {
            char *FileName = LinuxFileGroup->FindData.FileData->d_name;
            LinuxHandle->LinuxHandle = open(FileName, O_RDONLY);
            Result.NoErrors = (LinuxHandle->LinuxHandle >= 0);
        }

        if(!FindFileInFolder(LinuxFileGroup->Wildcard, &LinuxFileGroup->FindData))
        {
            LinuxFileGroup->FileAvailable = false;
        }
    }

    return(Result);
}

internal PLATFORM_FILE_ERROR(LinuxFileError)
{
#if GAME_INTERNAL
    fprintf(stderr, "LINUX FILE ERROR: %s\n", Message);
#endif

    Handle->NoErrors = false;
}

internal PLATFORM_READ_DATA_FROM_FILE(LinuxReadDataFromFile)
{
    if(PlatformNoFileErrors(Source))
    {
        linux_platform_file_handle *Handle = (linux_platform_file_handle *)Source->Platform;

        s64 BytesRead = pread(Handle->LinuxHandle, Dest, Size, (s64)Offset);
        if ((s64)Size == BytesRead)
        {
            // NOTE(michiel): File read succeeded!
        }
        else
        {
            LinuxFileError(Source, "Read file failed.");
        }
    }
}
#endif

//
// NOTE(michiel): MAIN
//

#if 0
#if GAME_INTERNAL
global_variable debug_table GlobalDebugTable_;
debug_table *GlobalDebugTable = &GlobalDebugTable_;
#endif
#endif

#if 0
// TODO(michiel): Linuxify
internal void
LinuxFullRestart(char *SourceEXE, char *DestEXE, char *DeleteEXE)
{

    pid_t Pid = fork();

    if (Pid)
    {
        Assert(sizeof(Result.OSHandle) >= sizeof(pid_t));
        *(pid_t *)&Result.OSHandle = Pid;
    }
    else
    {
        // NOTE(michiel): This is the created child process
        chdir(Path);
        execl(RealCommand, CommandLine, (char *)0);
        exit(0);
    }

    DeleteFile(DeleteEXE);
    if(MoveFile(DestEXE, DeleteEXE))
    {
        if(MoveFile(SourceEXE, DestEXE))
        {
            STARTUPINFO StartupInfo = {};
            StartupInfo.cb = sizeof(StartupInfo);
            PROCESS_INFORMATION ProcessInfo = {};
            if(CreateProcess(DestEXE,
                    GetCommandLine(),
                    0,
                    0,
                    FALSE,
                    0,
                    0,
                    "w:\\handmade\\data\\",
                    &StartupInfo,
                    &ProcessInfo))
            {
                CloseHandle(ProcessInfo.hProcess);
            }
            else
            {
                // TODO(casey): Error!
            }

            ExitProcess(0);
        }
    }
}
#endif

internal bool LinuxInitAudio(LinuxAudio* audio, GameAudio* gameAudio,
    uint32 channels, uint32 sampleRate, uint32 bufSampleLength)
{
    int err;
    snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
    const char* pcmName = "plughw:0,0";
    err = snd_pcm_open(&audio->pcmHandle, pcmName, stream, 0);
    if (err < 0) {
        DEBUG_PRINT("Error opening PCM device %s\n", pcmName);
        return false;
    }

    snd_pcm_hw_params_t* hwParams;
    snd_pcm_hw_params_alloca(&hwParams);
    err = snd_pcm_hw_params_any(audio->pcmHandle, hwParams);
    if (err < 0) {
        DEBUG_PRINT("Failed to initialize hw params\n");
        return false;
    }

    // Set access
    err = snd_pcm_hw_params_set_access(audio->pcmHandle, hwParams,
        SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        DEBUG_PRINT("Error setting audio access\n");
        return false;
    }

    // Set sample format
    err = snd_pcm_hw_params_set_format(audio->pcmHandle, hwParams,
        SND_PCM_FORMAT_S16_LE);
    if (err < 0) {
        DEBUG_PRINT("Error setting audio format\n");
        return false;
    }

    // Set sample rate. If the exact rate is not supported by the hardware,
    // use nearest possible rate.
    uint32 rate = sampleRate;
    err = snd_pcm_hw_params_set_rate_near(audio->pcmHandle, hwParams,
        &rate, 0);
    if (err < 0) {
        DEBUG_PRINT("Error setting sample rate\n");
        return false;
    }
    if (rate != sampleRate) {
        DEBUG_PRINT("Sample rate %d Hz not supported by your hardware\n"
            "==> Using %d Hz instead.\n", sampleRate, rate);
    }
    gameAudio->sampleRate = rate;

    // Set number of channels
    err = snd_pcm_hw_params_set_channels(audio->pcmHandle, hwParams, channels);
    if (err < 0) {
        DEBUG_PRINT("Error setting number of channels\n");
        return false;
    }
    gameAudio->channels = channels;

    // Set number of periods. Periods used to be called fragments.
    uint32 periods = 2;
    err = snd_pcm_hw_params_set_periods(audio->pcmHandle, hwParams,
        periods, 0);
    if (err < 0) {
        DEBUG_PRINT("Error setting number of periods\n");
        return false;
    }

    // Set buffer size (in frames). The resulting latency is given by
    // latency = periodsize * periods / (rate * bytes_per_frame)
    uint32 periodSize = 8192;
    uint32 numFrames = periodSize * periods / 4;
    err = snd_pcm_hw_params_set_buffer_size(audio->pcmHandle, hwParams,
        numFrames);
    if (err < 0) {
        DEBUG_PRINT("Error setting buffer size\n");
        return false;
    }
    DEBUG_PRINT("Sample rate: %d\n", sampleRate);
    DEBUG_PRINT("Number of frames: %d\n", numFrames);
    DEBUG_PRINT("Almost done with sound. Press Enter to continue\n");
    getchar();

    // Apply HW parameter settings to PCM device and prepare device
    err = snd_pcm_hw_params(audio->pcmHandle, hwParams);
    if (err < 0) {
        DEBUG_PRINT("Error setting HW params\n");
        return false;
    }

    return true;
}

int main(int argc, char **argv)
{
    int width = 800;
    int height = 600;

    #if GAME_SLOW
    debugPrint_ = DEBUGPlatformPrint;
    #endif

    LinuxState state = {};
    LinuxGetEXEFileName(&state);

    RemoveFileNameFromPath(state.exeFilePath, pathToApp_, LINUX_STATE_FILE_NAME_COUNT);
    DEBUG_PRINT("Path to application: %s\n", pathToApp_);
    
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        DEBUG_PRINT("Failed to open display\n");
        return 1;
    }

    if (!LinuxLoadGLXExtensions()) {
        return 1;
    }

    GLXFBConfig* framebufferConfigs = LinuxGetOpenGLFramebufferConfig(display);
    GLXFBConfig framebufferConfig = framebufferConfigs[0];
    XFree(framebufferConfigs);
    XVisualInfo* visualInfo = glXGetVisualFromFBConfig(display, framebufferConfig);
    if (!visualInfo) {
        DEBUG_PRINT("Failed to get framebuffer visual info\n");
        return 1;
    }

    visualInfo->screen = DefaultScreen(display);

    XSetWindowAttributes windowAttribs = {};
    Window root = RootWindow(display, visualInfo->screen);
    windowAttribs.colormap = XCreateColormap(display, root, visualInfo->visual, AllocNone);
    windowAttribs.border_pixel = 0;
    // This in combination with CWOverrideRedirect produces a borderless window
    //windowAttribs.override_redirect = 1;
    windowAttribs.event_mask = (StructureNotifyMask | PropertyChangeMask |
                                PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                                KeyPressMask | KeyReleaseMask);

    Window glWindow = XCreateWindow(display, root,
        100, 100, width, height,
        0, visualInfo->depth, InputOutput, visualInfo->visual,
        CWBorderPixel | CWColormap | CWEventMask /*| CWOverrideRedirect*/,
        &windowAttribs);
    if (!glWindow) {
        DEBUG_PRINT("Failed to create X11 window\n");
        return 1;
    }
    
    XSizeHints sizeHints = {};
    sizeHints.x = 100;
    sizeHints.y = 100;
    sizeHints.width  = width;
    sizeHints.height = height;
    sizeHints.flags = USSize | USPosition; // US vs PS?

    XSetNormalHints(display, glWindow, &sizeHints);
    XSetStandardProperties(display, glWindow, "315K", "glsync text",
        None, NULL, 0, &sizeHints);

    Atom wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, glWindow, &wmDeleteWindow, 1);
    XMapRaised(display, glWindow);
    DEBUG_PRINT("Created X11 window\n");

    GLXContext glxContext = 0;
    LOAD_GLX_FUNCTION(glXCreateContextAttribsARB);
    if (glXCreateContextAttribsARB) {
        glxContext = glXCreateContextAttribsARB(display, framebufferConfig,
            0, true, linuxOpenGLAttribs);
    }
    if (!glxContext) {
        DEBUG_PRINT("Failed to create OpenGL 3+ context\n");
        return 1;
    }

    if (!glXMakeCurrent(display, glWindow, glxContext)) {
        DEBUG_PRINT("Failed to make GLX context current\n");
        return 1;
    }
    DEBUG_PRINT("Created GLX context\n");

	OpenGLFunctions glFuncs = {};
    if (!LinuxInitOpenGL(&glFuncs, display, glWindow, width, height)) {
        return 1;
    }
    DEBUG_PRINT("Initialized Linux OpenGL\n");

    LinuxAudio linuxAudio;
    GameAudio gameAudio;
    if (!LinuxInitAudio(&linuxAudio, &gameAudio, 2, SAMPLERATE, 0)) {
        return 1;
    }
    unsigned char* data;
    int pcmreturn;
    int frames;

    data = (unsigned char*)malloc(8192);
    frames = 8192 >> 2;
    for (uint32 l1 = 0; l1 < 100; l1++) {
        for (uint32 l2 = 0; l2 < (8192 * 2 / 4); l2++) {
            short s1 = (l2 % 128) * 100 - 5000;  
            short s2 = (l2 % 256) * 100 - 5000;  
            data[4*l2] = (unsigned char)s1;
            data[4*l2+1] = s1 >> 8;
            data[4*l2+2] = (unsigned char)s2;
            data[4*l2+3] = s2 >> 8;
        }
        while ((pcmreturn = snd_pcm_writei(linuxAudio.pcmHandle,
        data, frames)) < 0) {
            snd_pcm_prepare(linuxAudio.pcmHandle);
            DEBUG_PRINT("<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>\n");
        }
    }
    DEBUG_PRINT("Done playing? Press Enter to continue");
    getchar();
    DEBUG_PRINT("Initialized Linux audio\n");

#if GAME_INTERNAL
	void* baseAddress = (void*)TERABYTES((uint64)2);;
#else
	void* baseAddress = 0;
#endif
    
    GameMemory gameMemory = {};
	gameMemory.permanentStorageSize = MEGABYTES(64);
	gameMemory.transientStorageSize = GIGABYTES(1);

	gameMemory.DEBUGPlatformPrint = DEBUGPlatformPrint;
	gameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
	gameMemory.DEBUGPlatformReadFile = DEBUGPlatformReadFile;
	gameMemory.DEBUGPlatformWriteFile = DEBUGPlatformWriteFile;

    gameMemory.DEBUGShouldInitGlobals = true;

	// TODO Look into using large virtual pages for this
    // potentially big allocation
	uint64 totalSize = gameMemory.permanentStorageSize
        + gameMemory.transientStorageSize;
	// TODO check allocation fail?
	gameMemory.permanentStorage = mmap(baseAddress, (size_t)totalSize,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	gameMemory.transientStorage = ((uint8*)gameMemory.permanentStorage +
		gameMemory.permanentStorageSize);

	state.gameMemorySize = totalSize;
	state.gameMemoryBlock = gameMemory.permanentStorage;
	if (!gameMemory.permanentStorage || !gameMemory.transientStorage) {
		// TODO log
		return 1;
	}
	DEBUG_PRINT("Initialized game memory\n");

    /*char SourceGameCodeDLLFullPath[LINUX_STATE_FILE_NAME_COUNT];
    LinuxBuildEXEPathFileName(&state, "libHandmade.so",
        sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);*/

	GameInput input[2] = {};
	GameInput *newInput = &input[0];
	GameInput *oldInput = &input[1];

    running_ = true;
    while (running_) {
		GameInput *temp = newInput;
		newInput = oldInput;
		oldInput = temp;
        // TODO clear input
    }

#if 0
        // Platform = GameMemory.PlatformAPI;

        if (Samples != MAP_FAILED)
        {
            uint32 NrJoysticks = LinuxFindJoysticks();

            game_input Input[2] = {};
            game_input *NewInput = &Input[0];
            game_input *OldInput = &Input[1];

            struct timespec LastCounter = LinuxGetWallClock();
            struct timespec FlipWallClock = LinuxGetWallClock();

            int DebugTimeMarkerIndex = 0;
            linux_debug_time_marker DebugTimeMarkers[30] = {0};

            uint32 AudioLatencyBytes = 0;
            float32 AudioLatencySeconds = 0.0f;
            bool32 SoundIsValid = false;

            linux_game_code Game = {};
            LinuxLoadGameCode(&Game, SourceGameCodeDLLFullPath, LinuxFileId(SourceGameCodeDLLFullPath));

            DEBUGSetEventRecording(Game.IsValid);

            // NOTE(michiel): And display the window on the screen
            XMapRaised(display, GlWindow);
            uint32 ExpectedFramesPerUpdate = 1;
            float32 TargetSecondsPerFrame = (float32)ExpectedFramesPerUpdate / (float32)GameUpdateHz;
            
            // NOTE(michiel): Querying X11 every frame can cause hickups because of a sync of X11 state
            linux_window_dimension Dimension = LinuxGetWindowDimension(display, GlWindow);
            v2 MouseP = LinuxGetMousePosition(display, GlWindow);
            
            while (GlobalRunning)
            {
                {DEBUG_DATA_BLOCK("Platform");
                    DEBUG_VALUE(ExpectedFramesPerUpdate);
                }
                {DEBUG_DATA_BLOCK("Platform/Controls");
                    DEBUG_B32(GlobalPause);
                    DEBUG_B32(GlobalSoftwareRendering);
                }

                //
                //
                //

                NewInput->dtForFrame = TargetSecondsPerFrame;

                //
                //
                //

                BEGIN_BLOCK("Input Processing");

                // TODO(casey): Zeroing macro
                // TODO(casey): We can't zero everything because the up/down state will
                // be wrong!!!
                game_controller_input *OldKeyboardController = GetController(OldInput, 0);
                game_controller_input *NewKeyboardController = GetController(NewInput, 0);
                {
                    TIMED_BLOCK("Resetting Buttons");
                    *NewKeyboardController = {};
                    NewKeyboardController->IsConnected = true;
                    for(uint32 ButtonIndex = 0;
                        ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
                        ++ButtonIndex)
                    {
                        NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                            OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                    }
                    for(uint32 ButtonIndex = 0;
                        ButtonIndex < PlatformMouseButton_Count;
                        ++ButtonIndex)
                    {
                        NewInput->MouseButtons[ButtonIndex] = OldInput->MouseButtons[ButtonIndex];
                        NewInput->MouseButtons[ButtonIndex].HalfTransitionCount = 0;
                    }
                }
                
                {
                    TIMED_BLOCK("Linux Keyboard and Message Processing");
                    ZeroStruct(NewInput->FKeyPressed);
                    LinuxProcessPendingMessages(state, display, GlWindow, WmDeleteWindow,
                        /*NewKeyboardController,*/ NewInput, &MouseP, &Dimension);
                }
                rectangle2i DrawRegion;
                {
                    TIMED_BLOCK("Aspect fitting");
                    DrawRegion = AspectRatioFit(RenderCommands.Settings.Width, RenderCommands.Settings.Height,
                                                Dimension.Width, Dimension.Height);
                }
                
                {
                    TIMED_BLOCK("Mouse Position");
                    float32 MouseX = (float32)MouseP.x;
                    float32 MouseY = (float32)((Dimension.Height - 1) - MouseP.y);
                    
                    float32 MouseU = Clamp01MapToRange((float32)DrawRegion.MinX, MouseX, (float32)DrawRegion.MaxX);
                    float32 MouseV = Clamp01MapToRange((float32)DrawRegion.MinY, MouseY, (float32)DrawRegion.MaxY);
                    
                    NewInput->MouseX = (float32)RenderCommands.Settings.Width*MouseU;
                    NewInput->MouseY = (float32)RenderCommands.Settings.Height*MouseV;
                    NewInput->MouseZ = 0.0f;
                }
                

                if (!GlobalPause)
                {
                    TIMED_BLOCK("Joystick processing");
                    // set the third parameter to the start where to fill in joystick data
                    LinuxJoystickPopulateGameInput(NewInput, OldInput, 1, NrJoysticks);
                }
                END_BLOCK();

                //
                //
                //

                BEGIN_BLOCK("Game Update");
                
                RenderCommands.LightPointIndex = 1;
                
                game_offscreen_buffer Buffer = {};
                Buffer.Memory = GlobalBackbuffer.Memory;
                Buffer.Width = GlobalBackbuffer.Width;
                Buffer.Height = GlobalBackbuffer.Height;
                Buffer.Pitch = GlobalBackbuffer.Pitch;
                if(!GlobalPause)
                {
                    if(state->InputRecordingIndex)
                    {
                        LinuxRecordInput(state, NewInput);
                    }

                    if(state->InputPlayingIndex)
                    {
                        game_input Temp = *NewInput;
                        LinuxPlayBackInput(state, NewInput);
                        for (uint32 MouseButtonIndex = 0;
                            MouseButtonIndex < PlatformMouseButton_Count;
                            ++MouseButtonIndex)
                        {
                            NewInput->MouseButtons[MouseButtonIndex] = Temp.MouseButtons[MouseButtonIndex];
                        }
                        NewInput->MouseX = Temp.MouseX;
                        NewInput->MouseY = Temp.MouseY;
                        NewInput->MouseZ = Temp.MouseZ;
                    }
                    if(Game.UpdateAndRender)
                    {
                        Game.UpdateAndRender(&GameMemory, NewInput, &RenderCommands);
                        if(NewInput->QuitRequested)
                        {
                            GlobalRunning = false;
                        }
                    }
                }

                END_BLOCK();

                //
                //
                //

                BEGIN_BLOCK("Audio Update");

                if(!GlobalPause)
                {
                    /* NOTE(casey):
                        Here is how sound output computation works.
                        We define a safety value that is the number
                        of samples we think our game update loop
                        may vary by (let's say up to 2ms)
                        When we wake up to write audio, we will look
                        and see what the play cursor position is and we
                        will forecast ahead where we think the play
                        cursor will be on the next frame boundary.
                        We will then look to see if the write cursor is
                        before that by at least our safety value.  If
                        it is, the target fill position is that frame
                        boundary plus one frame.  This gives us perfect
                        audio sync in the case of a card that has low
                        enough latency.
                        If the write cursor is _after_ that safety
                        margin, then we assume we can never sync the
                        audio perfectly, so we will write one frame's
                        worth of audio plus the safety margin's worth
                        of guard samples.
                    */
                    uint32 PlayCursor = SoundOutput.Buffer.ReadIndex;
                    uint32 WriteCursor = PlayCursor + AUDIO_WRITE_SAFE_SAMPLES * SoundOutput.BytesPerSample;
                    if (!SoundIsValid)
                    {
                        SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
                        SoundIsValid = true;
                    }

                    uint32 ByteToLock = ((SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) %
                                        SoundOutput.Buffer.Size);

                    uint32 ExpectedSoundBytesPerFrame =
                        (uint32)((float32)(SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample) /
                                GameUpdateHz);

                    uint32 ExpectedFrameBoundaryByte = PlayCursor + ExpectedSoundBytesPerFrame;

                    uint32 SafeWriteCursor = WriteCursor + SoundOutput.SafetyBytes;
                    bool32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);

                    uint32 TargetCursor = 0;
                    if(AudioCardIsLowLatency)
                    {
                        TargetCursor = (ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame);
                    }
                    else
                    {
                        TargetCursor = (WriteCursor + ExpectedSoundBytesPerFrame +
                                        SoundOutput.SafetyBytes);
                    }
                    TargetCursor = (TargetCursor % SoundOutput.Buffer.Size);

                    uint32 BytesToWrite = 0;
                    if(ByteToLock > TargetCursor)
                    {
                        BytesToWrite = (SoundOutput.Buffer.Size - ByteToLock);
                        BytesToWrite += TargetCursor;
                    }
                    else
                    {
                        BytesToWrite = TargetCursor - ByteToLock;
                    }

                    game_sound_output_buffer SoundBuffer = {};
                    SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                    SoundBuffer.SampleCount = Align8(BytesToWrite / SoundOutput.BytesPerSample);
                    BytesToWrite = SoundBuffer.SampleCount*SoundOutput.BytesPerSample;
                    SoundBuffer.Samples = Samples;
                    if(Game.GetSoundSamples)
                    {
                        Game.GetSoundSamples(&GameMemory, &SoundBuffer);
                    }

#if GAME_INTERNAL
                    linux_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
                    Marker->OutputPlayCursor = PlayCursor;
                    Marker->OutputWriteCursor = WriteCursor;
                    Marker->OutputLocation = ByteToLock;
                    Marker->OutputByteCount = BytesToWrite;
                    Marker->ExpectedFlipPlayCursor = ExpectedFrameBoundaryByte;

                    uint32 UnwrappedWriteCursor = WriteCursor;
                    if(UnwrappedWriteCursor < PlayCursor)
                    {
                        UnwrappedWriteCursor += SoundOutput.Buffer.Size;
                    }
                    AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
                    AudioLatencySeconds =
                        (((float32)AudioLatencyBytes / (float32)SoundOutput.BytesPerSample) / (float32)SoundOutput.SamplesPerSecond);
#endif
                    LinuxFillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
                }

                END_BLOCK();

                //
                //
                //

#if GAME_INTERNAL
                BEGIN_BLOCK("Debug Collation");

                // Reload code if necessary
                ino_t GameLibId = LinuxFileId(SourceGameCodeDLLFullPath);
                bool32 ExecutableNeedsToBeReloaded = (GameLibId != Game.GameLibID);

                GameMemory.ExecutableReloaded = false;
                if(ExecutableNeedsToBeReloaded)
                {
                    LinuxCompleteAllWork(&HighPriorityQueue);
                    LinuxCompleteAllWork(&LowPriorityQueue);
                    DEBUGSetEventRecording(false);
                }

                if(Game.DEBUGFrameEnd)
                {
                    Game.DEBUGFrameEnd(&GameMemory, NewInput, &RenderCommands);
                }

                if(ExecutableNeedsToBeReloaded)
                {
                    bool32 IsValid = false;
                    for(uint32 LoadTryIndex = 0;
                        !IsValid && (LoadTryIndex < 100);
                        ++LoadTryIndex)
                    {
                        IsValid = LinuxLoadGameCode(&Game, SourceGameCodeDLLFullPath, GameLibId);
                        usleep(100000);
                    }

                    GameMemory.ExecutableReloaded = true;
                    DEBUGSetEventRecording(Game.IsValid);
                }

                END_BLOCK();
#endif

                //
                //
                //

#if 0
                BEGIN_BLOCK("Framerate Wait");

                if (!GlobalPause)
                {
                    struct timespec WorkCounter = LinuxGetWallClock();
                    float32 WorkSecondsElapsed = LinuxGetSecondsElapsed(LastCounter, WorkCounter);

                    float32 SecondsElapsedForFrame = WorkSecondsElapsed;
                    if (SecondsElapsedForFrame < TargetSecondsPerFrame)
                    {
                        uint32 SleepUs = (uint32)(0.99e6f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
                        usleep(SleepUs);
                        while (SecondsElapsedForFrame < TargetSecondsPerFrame)
                        {
                            SecondsElapsedForFrame = LinuxGetSecondsElapsed(LastCounter, LinuxGetWallClock());
                        }
                    }
                    else
                    {
                        // Missed frame rate
                    }
                }
                END_BLOCK();
#endif

                //
                //
                //

                BEGIN_BLOCK("Frame Display");

                BeginTicketMutex(&TextureOpQueue->Mutex);
                texture_op *FirstTextureOp = TextureOpQueue->First;
                texture_op *LastTextureOp = TextureOpQueue->Last;
                TextureOpQueue->Last = TextureOpQueue->First = 0;
                EndTicketMutex(&TextureOpQueue->Mutex);

                if(FirstTextureOp)
                {
                    Assert(LastTextureOp);
                    OpenGLManageTextures(FirstTextureOp);
                    BeginTicketMutex(&TextureOpQueue->Mutex);
                    LastTextureOp->Next = TextureOpQueue->FirstFree;
                    TextureOpQueue->FirstFree = FirstTextureOp;
                    EndTicketMutex(&TextureOpQueue->Mutex);
                }
                
                glXSwapBuffers(display, GlWindow);

                /*LinuxDisplayBufferInWindow(&HighPriorityQueue, &RenderCommands, display, GlWindow,
                                            DrawRegion, Dimension.Width, Dimension.Height,
                                            &FrameTempArena);*/

                RenderCommands.PushBufferDataAt = RenderCommands.PushBufferBase;
                RenderCommands.VertexCount = 0;
                RenderCommands.LightBoxCount = 0;

                FlipWallClock = LinuxGetWallClock();

                game_input *Temp = NewInput;
                NewInput = OldInput;
                OldInput = Temp;
                // TODO(casey): Should I clear these here?

                END_BLOCK();

                struct timespec EndCounter = LinuxGetWallClock();
                float32 MeasuredSecondsPerFrame = LinuxGetSecondsElapsed(LastCounter, EndCounter);
                float32 ExactTargetFramesPerUpdate = MeasuredSecondsPerFrame*(float32)MonitorRefreshHz;
                uint32 NewExpectedFramesPerUpdate = RoundReal32ToInt32(ExactTargetFramesPerUpdate);
                ExpectedFramesPerUpdate = NewExpectedFramesPerUpdate;

                TargetSecondsPerFrame = MeasuredSecondsPerFrame;

                FRAME_MARKER(MeasuredSecondsPerFrame);
                LastCounter = EndCounter;
            }

            LinuxCompleteAllWork(&HighPriorityQueue);
            LinuxCompleteAllWork(&LowPriorityQueue);

            LinuxUnloadGameCode(&Game);
            LinuxStopPlayingSound();
        }
        else
        {
            LinuxStopPlayingSound();
            fprintf(stderr, "Unable to allocate space for the sound\n");
        }
#endif

    return 0;
}