#include "linux_main.h"
//#include "handmade_shared.h"

//#include "handmade_random.h"
//#include "handmade_lighting.h"

// #include <sys/types.h>
#include <sys/mman.h>     // PROT_*, MAP_*, munmap
#include <sys/stat.h>     // stat
#include <sys/sysinfo.h>  // get_nprocs
#include <sys/wait.h>     // waitpid
#include <unistd.h>       // usleep
#include <pthread.h>      // threading
#include <dlfcn.h>        // dlopen, dlsym, dlclose
#include <time.h>         // CLOCK_MONOTONIC, clock_gettime
#include <semaphore.h>    // sem_init, sem_wait, sem_post
#include <alloca.h>       // alloca

// X11 Windowing
#include <X11/Xlib.h>
#include <X11/Xatom.h>

// OpenGL and OpenGL for X11
#include <GL/gl.h>
#include <GL/glx.h>

// TODO(michiel): What to do with logging
#include <stdio.h>        // fprintf, stderr

//#include "linux_sound.h"
//#include "linux_joystick.h"

// TODO(michiel): write to console:
// write(1, string, size); for stdout
// write(2, string, size); for stderr
// read(0, buffer, size) to read from stdin

#if 0
platform_api Platform;

global_variable linux_state GlobalLinuxState;
global_variable b32 GlobalSoftwareRendering;
global_variable b32 GlobalRunning;
global_variable b32 GlobalPause;
global_variable b32 GlobalFullscreen;
global_variable linux_offscreen_buffer GlobalBackbuffer;
global_variable b32 DEBUGGlobalShowCursor;
global_variable u32 GlobalWindowPositionX;
global_variable u32 GlobalWindowPositionY;
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
OpenGLGlobalFunction(glXSwapIntervalEXT);
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

#if 0
// NOTE(michiel): Include other source files
#include "linux_sound.cpp"
#include "linux_joystick.cpp"

#include "handmade_render.h"
#include "handmade_opengl.h"
global_variable open_gl OpenGL;

#include "handmade_sort.cpp"
#include "handmade_opengl.cpp"
#include "handmade_render.cpp"
#endif

internal void
CatStrings(size_t SourceACount, const char *SourceA,
    size_t SourceBCount, const char *SourceB,
    size_t DestCount, char *Dest)
{
    // TODO(casey): Dest bounds checking!

    for(int index = 0; index < SourceACount; index++) {
        *Dest++ = *SourceA++;
    }

    for(int index = 0; index < SourceBCount; index++) {
        *Dest++ = *SourceB++;
    }

    *Dest++ = 0;
}

//
// NOTE(michiel): File and process handling
//

internal void
LinuxGetEXEFileName(LinuxState *state)
{
    // NOTE(casey): Never use MAX_PATH in code that is user-facing, because it
    // can be dangerous and lead to bad results.
    ssize_t NumRead = readlink("/proc/self/exe",
        state->EXEFileName, ARRAY_COUNT(state->EXEFileName) - 1);
    if (NumRead > 0) {
        state->OnePastLastEXEFileNameSlash = state->EXEFileName;
        for (char *scan = state->EXEFileName; *scan; scan++) {
            if (*scan == '/') {
                state->OnePastLastEXEFileNameSlash = scan + 1;
            }
        }
    }
}

internal void
LinuxBuildEXEPathFileName(linux_state *State, char *FileName,
    int DestCount, char *Dest)
{
    CatStrings(State->OnePastLastEXEFileNameSlash - State->EXEFileName,
        State->EXEFileName, StringLength(FileName), FileName,
        DestCount, Dest);
}

internal inline ino_t
LinuxFileId(char *FileName)
{
    struct stat Attr = {};
    if (stat(FileName, &Attr)) {
        Attr.st_ino = 0;
    }

    return Attr.st_ino;
}

#if GAME_INTERNAL
DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if(File->Contents) {
        munmap(File->Contents, File->ContentsSize);
        File->Contents = 0;
    }
    File->ContentsSize = 0;
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_read_file_result Result = {};

    s32 FileHandle = open(Filename, O_RDONLY);
    if (FileHandle >= 0) {
        off_t FileSize64 = lseek(FileHandle, 0, SEEK_END);
        lseek(FileHandle, 0, SEEK_SET);

        if (FileSize64 > 0) {
            u32 FileSize32 = SafeTruncateUInt64(FileSize64);
            Result.Contents = mmap(NULL, FileSize32, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (Result.Contents) {
                ssize_t BytesRead = read(FileHandle, Result.Contents, Result.ContentsSize);
                if ((ssize_t)FileSize32 == BytesRead) {
                    // NOTE(casey): File read successfully
                    Result.ContentsSize = FileSize32;
                }
                else {
                    // TODO(michiel): Logging
                    DEBUGPlatformFreeFileMemory(&Result);
                }
            }
        }
        else {
            // TODO(michiel): Logging
        }

        close(FileHandle);
    }
    else {
        // TODO(casey): Logging
    }

    return Result;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    b32 Result = false;

    s32 FileHandle = open(Filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (FileHandle >= 0) {
        ssize_t BytesWritten = write(FileHandle, Memory, MemorySize);
        if (fsync(FileHandle) >= 0) {
            Result = (BytesWritten == (ssize_t)MemorySize);
        }
        else {
            // TODO(casey): Logging
        }

        close(FileHandle);
    }
    else {
        // TODO(casey): Logging
    }

    return Result;
}
#endif

// Dynamic code loading
internal bool32 LinuxLoadGameCode(LinuxGameCode *GameCode,
    char *DLLName, ino_t FileID)
{
    if (GameCode->GameLibID != FileID) {
        LinuxUnloadLibrary(GameCode->GameLibHandle);
        GameCode->GameLibID = FileID;
        GameCode->IsValid = false;

        GameCode->GameLibHandle = LinuxLoadLibrary(DLLName);
        if (GameCode->GameLibHandle) {
            *(void **)(&GameCode->UpdateAndRender) = LinuxLoadFunction(
                GameCode->GameLibHandle, "GameUpdateAndRender");
        }
    }

    if (!GameCode->IsValid) {
        LinuxUnloadLibrary(GameCode->GameLibHandle);
        GameCode->GameLibID = 0;
        GameCode->UpdateAndRender = 0;
        GameCode->GetSoundSamples = 0;
        GameCode->DEBUGFrameEnd = 0;
    }

    return GameCode->IsValid;
}

internal void
LinuxUnloadGameCode(linux_game_code *GameCode)
{
    LinuxUnloadLibrary(GameCode->GameLibHandle);
    GameCode->GameLibID = 0;
    GameCode->IsValid = false;
    GameCode->UpdateAndRender = 0;
    GameCode->GetSoundSamples = 0;
    GameCode->DEBUGFrameEnd = 0;
}

//
// NOTE(michiel): OpenGL
//

/*
global_variable int LinuxOpenGLAttribs[] =
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

#define LinuxGetOpenGLFunction(Name) Name = (type_##Name *)glXGetProcAddress((GLubyte *) #Name)
*/

internal void
LinuxLoadGlxExtensions(void)
{
    /* Open Xlib Display */
    Display *TempDisplay = XOpenDisplay(NULL);
    if (TempDisplay) {
        int Dummy;
        if (glXQueryExtension(TempDisplay, &Dummy, &Dummy)) {
            int DisplayBufferAttribs[] = {
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
            XVisualInfo *Visuals = 0;
            Visuals = glXChooseVisual(TempDisplay,
                DefaultScreen(TempDisplay), DisplayBufferAttribs);

            if (Visuals) {
                Visuals->screen = DefaultScreen(TempDisplay);

                XSetWindowAttributes Attribs = {};
                Window Root = RootWindow(TempDisplay, Visuals->screen);
                Attribs.colormap = XCreateColormap(TempDisplay, Root,
                    Visuals->visual, AllocNone);

                Window GLWindow;
                GLWindow = XCreateWindow(TempDisplay, Root,
                    0, 0,
                    10, 10,
                    0, Visuals->depth, InputOutput, Visuals->visual,
                    CWColormap, &Attribs);
                if (GLWindow) {
                    GLXContext Context = glXCreateContext(TempDisplay, Visuals, NULL, true);

                    LinuxGetOpenGLFunction(glXCreateContextAttribsARB);

                    if (glXMakeCurrent(TempDisplay, GLWindow, Context)) {
                        char *Extensions = (char *)glXQueryExtensionsString(TempDisplay, Visuals->screen);
                        char *At = Extensions;
                        while (*At) {
                            while (IsWhitespace(*At)) {
                                At++;
                            }
                            char *End = At;
                            while (*End && !IsWhitespace(*End)) {
                                End++;
                            }

                            umm Count = End - At;

                            if (0) {}
                            else if (StringsAreEqual(Count, At, "GLX_EXT_framebuffer_sRGB")) {OpenGL.SupportsSRGBFramebuffer = true;}
                            else if (StringsAreEqual(Count, At, "GLX_ARB_framebuffer_sRGB")) {OpenGL.SupportsSRGBFramebuffer = true;}

                            At = End;
                        }
                    }

                    glXMakeCurrent(0, 0, 0);

                    glXDestroyContext(TempDisplay, Context);
                    XDestroyWindow(TempDisplay, GLWindow);
                }

                XFreeColormap(TempDisplay, Attribs.colormap);
                XFree(Visuals);
            }
        }
        XCloseDisplay(TempDisplay);
    }
}

internal GLXFBConfig *
LinuxGetOpenGLFramebufferConfig(Display *display)
{
    int VisualAttribs[] = {
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

    if (!OpenGL.SupportsSRGBFramebuffer) {
        VisualAttribs[22] = None;
    }

    int FramebufferCount;
    GLXFBConfig *FramebufferConfig = glXChooseFBConfig(display, DefaultScreen(display), VisualAttribs, &FramebufferCount);
    DEBUG_ASSERT(FramebufferCount >= 1);

    return FramebufferConfig;
}

internal GLXContext
LinuxInitOpenGL(Display *display, GLXDrawable GlWindow, GLXFBConfig Config)
{
    b32 ModernContext = true;

    GLXContext OpenGlContext = 0;
    if (glXCreateContextAttribsARB) {
        OpenGlContext = glXCreateContextAttribsARB(display, Config, 0, true, LinuxOpenGLAttribs);
    }

    if (!OpenGlContext) {
        ModernContext = false;

        XVisualInfo *VisualInfo = glXGetVisualFromFBConfig(display, Config);
        OpenGlContext = glXCreateContext(display, VisualInfo, 0, true);
        XFree(VisualInfo);
    }

#if 0
    if (glXMakeCurrent(display, GlWindow, OpenGlContext)) {
        LinuxGetOpenGLFunction(glDebugMessageCallbackARB);

        LinuxGetOpenGLFunction(glXSwapIntervalEXT);
        LinuxGetOpenGLFunction(glGetStringi);

        LinuxGetOpenGLFunction(glGenBuffers);
        LinuxGetOpenGLFunction(glBindBuffer);
        LinuxGetOpenGLFunction(glBufferData);
        LinuxGetOpenGLFunction(glDrawBuffers);

        LinuxGetOpenGLFunction(glCreateShader);
        LinuxGetOpenGLFunction(glShaderSource);
        LinuxGetOpenGLFunction(glCompileShader);
        LinuxGetOpenGLFunction(glAttachShader);
        LinuxGetOpenGLFunction(glGetShaderInfoLog);
        LinuxGetOpenGLFunction(glDeleteShader);

        LinuxGetOpenGLFunction(glCreateProgram);
        LinuxGetOpenGLFunction(glLinkProgram);
        LinuxGetOpenGLFunction(glValidateProgram);
        LinuxGetOpenGLFunction(glGetProgramiv);
        LinuxGetOpenGLFunction(glGetProgramInfoLog);
        LinuxGetOpenGLFunction(glUseProgram);
        LinuxGetOpenGLFunction(glDeleteProgram);

        LinuxGetOpenGLFunction(glGetUniformLocation);
        LinuxGetOpenGLFunction(glUniform1i);
        LinuxGetOpenGLFunction(glUniform1f);
        LinuxGetOpenGLFunction(glUniform2fv);
        LinuxGetOpenGLFunction(glUniform3fv);
        LinuxGetOpenGLFunction(glUniform4fv);
        LinuxGetOpenGLFunction(glUniformMatrix4fv);

        LinuxGetOpenGLFunction(glGenVertexArrays);
        LinuxGetOpenGLFunction(glBindVertexArray);

        LinuxGetOpenGLFunction(glGetAttribLocation);
        LinuxGetOpenGLFunction(glEnableVertexAttribArray);
        LinuxGetOpenGLFunction(glVertexAttribPointer);
        LinuxGetOpenGLFunction(glVertexAttribIPointer);
        LinuxGetOpenGLFunction(glDisableVertexAttribArray);

        LinuxGetOpenGLFunction(glTexImage2DMultisample);

        opengl_info Info = OpenGLGetInfo(ModernContext);
        if(Info.HasGL_ARB_framebuffer_object)
        {
            LinuxGetOpenGLFunction(glGenFramebuffers);
            LinuxGetOpenGLFunction(glCheckFramebufferStatus);
            LinuxGetOpenGLFunction(glBindFramebuffer);
            LinuxGetOpenGLFunction(glFramebufferTexture2D);
            LinuxGetOpenGLFunction(glBlitFramebuffer);
            LinuxGetOpenGLFunction(glDeleteFramebuffers);
        }

        if (glXSwapIntervalEXT)
        {
            glXSwapIntervalEXT(display, GlWindow, 1);
        }

        OpenGLInit(Info, OpenGL.SupportsSRGBFramebuffer);
    }
#endif

    return OpenGlContext;
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

internal LinuxOffscreenBuffer LinuxCreateOffscreenBuffer(u32 Width, u32 Height)
{
    LinuxOffscreenBuffer OffscreenBuffer = {};
    OffscreenBuffer.Width = Width;
    OffscreenBuffer.Height = Height;
    OffscreenBuffer.Pitch = Align16(OffscreenBuffer.Width * BYTES_PER_PIXEL);
    u32 Size = OffscreenBuffer.Pitch * OffscreenBuffer.Height;

    OffscreenBuffer.Memory = (u8 *)mmap(NULL, Size, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (OffscreenBuffer.Memory == MAP_FAILED) {
        // TODO(michiel): Logging
        OffscreenBuffer.Width = 0;
        OffscreenBuffer.Height = 0;
        return OffscreenBuffer;
    }

    return OffscreenBuffer;
}

internal void LinuxResizeOffscreenBuffer(
    LinuxOffscreenBuffer *Buffer, u32 Width, u32 Height)
{
    if (Buffer->Memory) {
        munmap(Buffer->Memory, Buffer->Pitch * Buffer->Height);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->Pitch = Buffer->Width * BYTES_PER_PIXEL;

    u32 NewSize = Buffer->Pitch * Buffer->Height;
    Buffer->Memory = (u8 *)mmap(NULL, NewSize, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (Buffer->Memory == MAP_FAILED) {
        // TODO(michiel): Logging
        Buffer->Width = 0;
        Buffer->Height = 0;
        return;
    }
}

internal void LinuxDisplayBufferInWindow(
    platform_work_queue *RenderQueue, game_render_commands *Commands,
    Display *display, GLXDrawable GlWindow,
    rectangle2i DrawRegion, u32 WindowWidth, u32 WindowHeight,
    memory_arena *TempArena)
{
    temporary_memory TempMem = BeginTemporaryMemory(TempArena);

    /*  TODO(casey): Do we want to check for resources like before?  Probably?
        if(AllResourcesPresent(RenderGroup))
        {
            RenderToOutput(TranState->HighPriorityQueue, RenderGroup, &DrawBuffer, &TranState->TranArena);
        }
    */

    if (GlobalSoftwareRendering) {
        loaded_bitmap OutputTarget;
        OutputTarget.Memory = GlobalBackbuffer.Memory;
        OutputTarget.Width = GlobalBackbuffer.Width;
        OutputTarget.Height = GlobalBackbuffer.Height;
        OutputTarget.Pitch = GlobalBackbuffer.Pitch;

        SoftwareRenderCommands(RenderQueue, Commands, &OutputTarget, TempArena);

        // NOTE(michiel): Draw with opengl because i allready got that one
        // TODO(casey): Track clears so we clear the backbuffer to the right color?
        v4 ClearColor = {};
        OpenGLDisplayBitmap(GlobalBackbuffer.Width, GlobalBackbuffer.Height,
            GlobalBackbuffer.Memory, GlobalBackbuffer.Pitch, DrawRegion,
            ClearColor, OpenGL.ReservedBlitTexture);
    }
    else {
        TIMED_BLOCK("OpenGLRenderCommands");
        OpenGLRenderCommands(Commands, DrawRegion, WindowWidth, WindowHeight);
    }

    BEGIN_BLOCK("Swap buffers");
    glXSwapBuffers(display, GlWindow);
    END_BLOCK();

    EndTemporaryMemory(TempMem);
}

internal void
LinuxProcessKeyboardMessage(game_button_state *NewState, b32 IsDown)
{
    if (NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

internal void
LinuxGetInputFileLocation(linux_state *State, b32 InputStream,
                          s32 SlotIndex, s32 DestCount, char *Dest)
{
    char Temp[64];
    snprintf(Temp, sizeof(Temp), "loop_edit_%d_%s.hmi", SlotIndex, InputStream ? "input" : "state");
    LinuxBuildEXEPathFileName(State, Temp, DestCount, Dest);
}

internal void
LinuxCreateHiddenCursor(Display *display, Window window)
{
    Pixmap Blank;
    XColor Dummy;
    char BlankBytes[1] = {0x00};
    Blank = XCreateBitmapFromData(display, window, BlankBytes, 1, 1);
    GlobalHiddenCursor = XCreatePixmapCursor(display, Blank, Blank, &Dummy, &Dummy, 0, 0);
    XFreePixmap(display, Blank);
}

internal void
LinuxHideCursor(Display *display, Window window)
{
    if (DEBUGGlobalShowCursor)
    {
        XDefineCursor(display, window, GlobalHiddenCursor);
        DEBUGGlobalShowCursor = false;
    }
}

internal void
LinuxShowCursor(Display *display, Window window)
{
    if (!DEBUGGlobalShowCursor)
    {
        XUndefineCursor(display, window);
        DEBUGGlobalShowCursor = true;
    }
}

internal linux_window_dimension
LinuxGetWindowDimension(Display *display, Window window)
{
    XWindowAttributes WindowAttribs = {};
    XGetWindowAttributes(display, window, &WindowAttribs);

    linux_window_dimension Result = {};
    Result.Width = WindowAttribs.width;
    Result.Height = WindowAttribs.height;
    return Result;
}

internal void
ToggleFullscreen(Display *display, Window window)
{
    GlobalFullscreen = !GlobalFullscreen;
    Atom FullscreenAtom = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    Atom WindowState = XInternAtom(display, "_NET_WM_STATE", False);
    s32 Mask = SubstructureNotifyMask | SubstructureRedirectMask;
    // s32 Mask = StructureNotifyMask | ResizeRedirectMask;
    XEvent event = {};
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    // event.xclient.display = display;
    event.xclient.window = window;
    event.xclient.message_type = WindowState;
    event.xclient.format = 32;
    event.xclient.data.l[0] = (GlobalFullscreen ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE);  /* set (2 is toggle) */
    event.xclient.data.l[1] = (long)FullscreenAtom;
    event.xclient.data.l[2] = 0;

    XSendEvent(display, DefaultRootWindow(display), False, Mask, &event);
    // XFlush(display);
}

internal inline v2
LinuxGetMousePosition(Display *display, Window window)
{
    Window RetRoot, RetWin;
    s32 RootX, RootY;
    s32 WinX, WinY;
    u32 Mask;
    b32 QuerySuccess = XQueryPointer(display, window,
                                     &RetRoot, &RetWin,
                                     &RootX, &RootY, &WinX, &WinY, &Mask);

    v2 Result = {};
    if (QuerySuccess)
    {
        Result.x = (f32)WinX;
        Result.y = (f32)WinY;
    }
    return Result;
}

//
// NOTE(michiel): Memory
//

#if GAME_INTERNAL
internal
DEBUG_PLATFORM_GET_MEMORY_STATS(LinuxGetMemoryStats)
{
    debug_platform_memory_stats Stats = {};

    BeginTicketMutex(&GlobalLinuxState.MemoryMutex);
    linux_memory_block *Sentinel = &GlobalLinuxState.MemorySentinel;
    for(linux_memory_block *SourceBlock = Sentinel->Next;
        SourceBlock != Sentinel;
        SourceBlock = SourceBlock->Next)
    {
        Assert(SourceBlock->Block.Size <= U32Max);

        ++Stats.BlockCount;
        Stats.TotalSize += SourceBlock->Block.Size;
        Stats.TotalUsed += SourceBlock->Block.Used;
    }
    EndTicketMutex(&GlobalLinuxState.MemoryMutex);

    return(Stats);
}
#endif

internal void
LinuxVerifyMemoryListIntegrity(void)
{
    BeginTicketMutex(&GlobalLinuxState.MemoryMutex);
    local_persist u32 FailCounter;
    linux_memory_block *Sentinel = &GlobalLinuxState.MemorySentinel;
    for(linux_memory_block *SourceBlock = Sentinel->Next;
        SourceBlock != Sentinel;
        SourceBlock = SourceBlock->Next)
    {
        Assert(SourceBlock->Block.Size <= U32Max);
    }
    ++FailCounter;
    EndTicketMutex(&GlobalLinuxState.MemoryMutex);
}

internal void
LinuxFreeMemoryBlock(linux_memory_block *Block)
{
    u32 Size = Block->Block.Size;
    u64 Flags = Block->Block.Flags;
    umm PageSize = sysconf(_SC_PAGESIZE);
    umm TotalSize = Size + sizeof(linux_memory_block);
    if(Flags & PlatformMemory_UnderflowCheck)
    {
        TotalSize = Size + 2*PageSize;
    }
    else if(Flags & PlatformMemory_OverflowCheck)
    {
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
    for(linux_memory_block *BlockIter = State->MemorySentinel.Next;
        BlockIter != &State->MemorySentinel;
        )
    {
        linux_memory_block *Block = BlockIter;
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

inline b32x
LinuxIsInLoop(linux_state *State)
{
    b32x Result = ((State->InputRecordingIndex != 0) ||
                   (State->InputPlayingIndex));
    return(Result);
}

PLATFORM_ALLOCATE_MEMORY(LinuxAllocateMemory)
{
    // NOTE(casey): We require memory block headers not to change the cache
    // line alignment of an allocation
    Assert(sizeof(linux_memory_block) == 64);

    umm PageSize = sysconf(_SC_PAGESIZE);
    umm TotalSize = Size + sizeof(linux_memory_block);
    umm BaseOffset = sizeof(linux_memory_block);
    umm ProtectOffset = 0;
    if(Flags & PlatformMemory_UnderflowCheck)
    {
        TotalSize = Size + 2*PageSize;
        BaseOffset = 2*PageSize;
        ProtectOffset = PageSize;
    }
    else if(Flags & PlatformMemory_OverflowCheck)
    {
        umm SizeRoundedUp = AlignPow2(Size, PageSize);
        TotalSize = SizeRoundedUp + 2*PageSize;
        BaseOffset = PageSize + SizeRoundedUp - Size;
        ProtectOffset = PageSize + SizeRoundedUp;
    }

    linux_memory_block *Block = (linux_memory_block *)
        mmap(0, TotalSize, PROT_READ|PROT_WRITE,
             MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    Assert(Block);
    Block->Block.Base = (u8 *)Block + BaseOffset;
    Assert(Block->Block.Used == 0);
    Assert(Block->Block.ArenaPrev == 0);

    if(Flags & (PlatformMemory_UnderflowCheck|PlatformMemory_OverflowCheck))
    {
        s32 Error = mprotect((u8 *)Block + ProtectOffset, PageSize, PROT_NONE);
        Assert(Error == 0);
    }

    linux_memory_block *Sentinel = &GlobalLinuxState.MemorySentinel;
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
        linux_memory_block *LinuxBlock = ((linux_memory_block *)Block);
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

//
// Replays
//

internal void
LinuxBeginRecordingInput(linux_state *State, int InputRecordingIndex)
{
    // TODO(michiel): mmap to file?
    char FileName[LINUX_STATE_FILE_NAME_COUNT];
    LinuxGetInputFileLocation(State, true, InputRecordingIndex, sizeof(FileName), FileName);
    State->RecordingHandle = open(FileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if(State->RecordingHandle >= 0)
    {
        //ssize_t BytesWritten = write(FileHandle, Memory, MemorySize);
        ssize_t BytesWritten;

        State->InputRecordingIndex = InputRecordingIndex;
        linux_memory_block *Sentinel = &GlobalLinuxState.MemorySentinel;

        BeginTicketMutex(&GlobalLinuxState.MemoryMutex);
        for(linux_memory_block *SourceBlock = Sentinel->Next;
            SourceBlock != Sentinel;
            SourceBlock = SourceBlock->Next)
        {
            if(!(SourceBlock->Block.Flags & PlatformMemory_NotRestored))
            {
                linux_saved_memory_block DestBlock;
                void *BasePointer = SourceBlock->Block.Base;
                DestBlock.BasePointer = (u64)BasePointer;
                DestBlock.Size = SourceBlock->Block.Size;
                BytesWritten = write(State->RecordingHandle, &DestBlock, sizeof(DestBlock));
                Assert(DestBlock.Size <= U32Max);
                BytesWritten = write(State->RecordingHandle, BasePointer, (u32)DestBlock.Size);
            }
        }
        EndTicketMutex(&GlobalLinuxState.MemoryMutex);

        linux_saved_memory_block DestBlock = {};
        BytesWritten = write(State->RecordingHandle, &DestBlock, sizeof(DestBlock));
    }
}

internal void
LinuxEndRecordingInput(linux_state *State)
{
    fsync(State->RecordingHandle);
    close(State->RecordingHandle);
    State->InputRecordingIndex = 0;
}

internal void
LinuxBeginInputPlayBack(linux_state *State, int InputPlayingIndex)
{
    LinuxClearBlocksByMask(State, LinuxMem_AllocatedDuringLooping);

    char FileName[LINUX_STATE_FILE_NAME_COUNT];
    LinuxGetInputFileLocation(State, true, InputPlayingIndex, sizeof(FileName), FileName);
    State->PlaybackHandle = open(FileName, O_RDONLY);
    if(State->PlaybackHandle >= 0)
    {
        State->InputPlayingIndex = InputPlayingIndex;

        for(;;)
        {
            linux_saved_memory_block Block = {};
            ssize_t BytesRead = read(State->PlaybackHandle, &Block, sizeof(Block));
            if(Block.BasePointer != 0)
            {
                void *BasePointer = (void *)Block.BasePointer;
                Assert(Block.Size <= U32Max);
                BytesRead = read(State->PlaybackHandle, BasePointer, (u32)Block.Size);
            }
            else
            {
                break;
            }
        }
        // TODO(casey): Stream memory in from the file!
    }
}

internal void
LinuxEndInputPlayBack(linux_state *State)
{
    LinuxClearBlocksByMask(State, LinuxMem_FreedDuringLooping);
    close(State->PlaybackHandle);
    State->InputPlayingIndex = 0;
}

internal void
LinuxRecordInput(linux_state *State, game_input *NewInput)
{
    write(State->RecordingHandle, NewInput, sizeof(*NewInput));
}

internal void
LinuxPlayBackInput(linux_state *State, game_input *NewInput)
{
    ssize_t BytesRead = read(State->PlaybackHandle, NewInput, sizeof(*NewInput));
    if(BytesRead == 0)
    {
        // NOTE(casey): We've hit the end of the stream, go back to the beginning
        s32 PlayingIndex = State->InputPlayingIndex;
        LinuxEndInputPlayBack(State);
        LinuxBeginInputPlayBack(State, PlayingIndex);
        read(State->PlaybackHandle, NewInput, sizeof(*NewInput));
    }
}


//
// NOTE(michiel): Timing
//

internal inline struct timespec
LinuxGetWallClock()
{
    struct timespec Clock;
    clock_gettime(CLOCK_MONOTONIC, &Clock);
    return Clock;
}

internal inline f32
LinuxGetSecondsElapsed(struct timespec Start, struct timespec End)
{
    return ((f32)(End.tv_sec - Start.tv_sec)
            + ((f32)(End.tv_nsec - Start.tv_nsec) * 1e-9f));
}

//
// NOTE(michiel): Threading
//

internal void LinuxAddEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
{
    u32 NewNextEntryToWrite = (Queue->NextEntryToWrite + 1) % ArrayCount(Queue->Entries);
    Assert(NewNextEntryToWrite != Queue->NextEntryToRead);
    platform_work_queue_entry *Entry = Queue->Entries + Queue->NextEntryToWrite;
    Entry->Callback = Callback;
    Entry->Data = Data;
    ++Queue->CompletionGoal;

    asm volatile("" ::: "memory");

    Queue->NextEntryToWrite = NewNextEntryToWrite;
    sem_post(&Queue->SemaphoreHandle);
}

internal b32
LinuxDoNextWorkQueueEntry(platform_work_queue *Queue)
{
    bool32 WeShouldSleep = false;

    uint32 OriginalNextEntryToRead = Queue->NextEntryToRead;
    uint32 NewNextEntryToRead = (OriginalNextEntryToRead + 1) % ArrayCount(Queue->Entries);
    if(OriginalNextEntryToRead != Queue->NextEntryToWrite)
    {
        uint32 Index = __sync_val_compare_and_swap(&Queue->NextEntryToRead,
                                                   OriginalNextEntryToRead,
                                                   NewNextEntryToRead);
        if(Index == OriginalNextEntryToRead)
        {
            platform_work_queue_entry Entry = Queue->Entries[Index];
            Entry.Callback(Queue, Entry.Data);
            __sync_fetch_and_add(&Queue->CompletionCount, 1);
        }
    }
    else
    {
        WeShouldSleep = true;
    }

    return(WeShouldSleep);
}

internal void
LinuxCompleteAllWork(platform_work_queue *Queue)
{
    while(Queue->CompletionGoal != Queue->CompletionCount)
    {
        LinuxDoNextWorkQueueEntry(Queue);
    }

    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}

void *
ThreadProc(void *Parameter)
{
    linux_thread_startup *Thread = (linux_thread_startup *)Parameter;
    platform_work_queue *Queue = Thread->Queue;

    for(;;)
    {
        if(LinuxDoNextWorkQueueEntry(Queue))
        {
            sem_wait(&Queue->SemaphoreHandle);
        }
    }

    //    return(0);
}

internal void
LinuxMakeQueue(platform_work_queue *Queue, u32 ThreadCount, linux_thread_startup *Startups)
{
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;

    Queue->NextEntryToWrite = 0;
    Queue->NextEntryToRead = 0;

    uint32 InitialCount = 0;
    sem_init(&Queue->SemaphoreHandle, 0, InitialCount);

    for(uint32 ThreadIndex = 0;
        ThreadIndex < ThreadCount;
        ++ThreadIndex)
    {
        linux_thread_startup *Startup = Startups + ThreadIndex;
        Startup->Queue = Queue;

        pthread_attr_t Attr;
        pthread_t ThreadID;
        pthread_attr_init(&Attr);
        pthread_attr_setdetachstate(&Attr, PTHREAD_CREATE_DETACHED);
        int result = pthread_create(&ThreadID, &Attr, ThreadProc, Startup);
        pthread_attr_destroy(&Attr);
    }
}

//
// NOTE(michiel): Keyboard mapping
//

#if 0
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

internal void
LinuxProcessPendingMessages(linux_state *State, Display *display, Window window,
                            Atom WmDeleteWindow,
                            game_controller_input *KeyboardController, game_input *Input, v2 *MouseP,
                            linux_window_dimension *Dimension)
{
    while (GlobalRunning && XPending(display))
    {
        XEvent Event;
        XNextEvent(display, &Event);

        // NOTE(michiel): Don't skip the scroll key Events
        if (Event.type == ButtonRelease)
        {
            if ((Event.xbutton.button != 4) &&
                (Event.xbutton.button != 5) &&
                XEventsQueued(display, QueuedAfterReading))
            {
                // NOTE(michiel): Skip the auto repeat key
                XEvent NextEvent;
                XPeekEvent(display, &NextEvent);
                if ((NextEvent.type == ButtonPress) &&
                    (NextEvent.xbutton.time == Event.xbutton.time) &&
                    (NextEvent.xbutton.button == Event.xbutton.button))
                {
                    continue;
                }
            }
        }
        // NOTE(michiel): Skip the Keyboard
        if (Event.type == KeyRelease && XEventsQueued(display, QueuedAfterReading))
        {
            XEvent NextEvent;
            XPeekEvent(display, &NextEvent);
            if ((NextEvent.type == KeyPress) &&
                (NextEvent.xbutton.time == Event.xbutton.time) &&
                (NextEvent.xbutton.button == Event.xbutton.button))
            {
                continue;
            }
        }

        switch (Event.type)
        {
            case ConfigureNotify:
            {
                s32 W = Event.xconfigure.width;
                s32 H = Event.xconfigure.height;
                if((Dimension->Width != W) || (Dimension->Height != H))
                {
                    Dimension->Width = W;
                    Dimension->Height = H;
                }
            } break;

            case DestroyNotify:
            {
                GlobalRunning = false;
            } break;

            case ClientMessage:
            {
                if ((Atom)Event.xclient.data.l[0] == WmDeleteWindow)
                {
                    GlobalRunning = false;
                }
            } break;
            
            case MotionNotify:
            {
                MouseP->x = (f32)Event.xmotion.x;
                MouseP->y = (f32)Event.xmotion.y;
            } break;

            case ButtonRelease:
            case ButtonPress:
            {
                if (Event.xbutton.button == 1)
                {
                    LinuxProcessKeyboardMessage(&Input->MouseButtons[PlatformMouseButton_Left],
                                                Event.type == ButtonPress);
                }
                else if (Event.xbutton.button == 2)
                {
                    LinuxProcessKeyboardMessage(&Input->MouseButtons[PlatformMouseButton_Middle],
                                                Event.type == ButtonPress);
                }
                else if (Event.xbutton.button == 3)
                {
                    LinuxProcessKeyboardMessage(&Input->MouseButtons[PlatformMouseButton_Right],
                                                Event.type == ButtonPress);
                }
                else if (Event.xbutton.button == 4)
                {
                    ++(Input->MouseZ);
                }
                else if (Event.xbutton.button == 5)
                {
                    --(Input->MouseZ);
                }
                else if (Event.xbutton.button == 8)
                {
                    LinuxProcessKeyboardMessage(&Input->MouseButtons[PlatformMouseButton_Extended0],
                                                Event.type == ButtonPress);
                }
                else if (Event.xbutton.button == 9)
                {
                    LinuxProcessKeyboardMessage(&Input->MouseButtons[PlatformMouseButton_Extended1],
                                                Event.type == ButtonPress);
                    // } else {
                    //     printf("Uncaught button: %u\n", Event.xbutton.button);
                }
            } break;

            case KeyRelease:
            case KeyPress:
            {
                b32 AltKeyWasDown = Event.xkey.state & KEYCODE_ALT_MASK;
                b32 ShiftKeyWasDown = Event.xkey.state & KEYCODE_SHIFT_MASK;
                
                if (!GlobalPause)
                {
                    if (Event.xkey.keycode == KEYCODE_W)
                    {
                        LinuxProcessKeyboardMessage(&KeyboardController->MoveUp, Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_A)
                    {
                        LinuxProcessKeyboardMessage(&KeyboardController->MoveLeft, Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_S)
                    {
                        LinuxProcessKeyboardMessage(&KeyboardController->MoveDown, Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_D)
                    {
                        LinuxProcessKeyboardMessage(&KeyboardController->MoveRight, Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_Q)
                    {
                        LinuxProcessKeyboardMessage(&KeyboardController->LeftShoulder, Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_E)
                    {
                        LinuxProcessKeyboardMessage(&KeyboardController->RightShoulder, Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_UP)
                    {
                        LinuxProcessKeyboardMessage(&KeyboardController->ActionUp, Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_DOWN)
                    {
                        LinuxProcessKeyboardMessage(&KeyboardController->ActionDown, Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_LEFT)
                    {
                        LinuxProcessKeyboardMessage(&KeyboardController->ActionLeft, Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_RIGHT)
                    {
                        LinuxProcessKeyboardMessage(&KeyboardController->ActionRight, Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_ESCAPE)
                    {
                        LinuxProcessKeyboardMessage(&KeyboardController->Back, Event.type == KeyPress);
                    }
                    else if (Event.xkey.keycode == KEYCODE_SPACE)
                    {
                        LinuxProcessKeyboardMessage(&KeyboardController->Start, Event.type == KeyPress);
                    }
                }
                
                if ((Event.xkey.keycode == KEYCODE_SHIFT_L) || (Event.xkey.keycode == KEYCODE_SHIFT_R))
                {
                    Input->ShiftDown = (Event.type == KeyPress);
                }
                else if ((Event.xkey.keycode == KEYCODE_ALT_L) || (Event.xkey.keycode == KEYCODE_ALT_R))
                {
                    Input->AltDown = (Event.type == KeyPress);
                }
                else if ((Event.xkey.keycode == KEYCODE_CTRL_L) || (Event.xkey.keycode == KEYCODE_CTRL_R))
                {
                    Input->ControlDown = (Event.type == KeyPress);
                }

#if GAME_INTERNAL
                else if (Event.xkey.keycode == KEYCODE_P)
                {
                    if (Event.type == KeyPress)
                    {
                        GlobalPause = !GlobalPause;
                    }
                }
                else if (Event.xkey.keycode == KEYCODE_L)
                {
                    if (Event.type == KeyPress)
                    {
                        if (AltKeyWasDown)
                        {
                            LinuxBeginInputPlayBack(State, 1);
                        }
                        else
                        {
                            if(State->InputPlayingIndex == 0)
                            {
                                if(State->InputRecordingIndex == 0)
                                {
                                    LinuxBeginRecordingInput(State, 1);
                                }
                                else
                                {
                                    LinuxEndRecordingInput(State);
                                    LinuxBeginInputPlayBack(State, 1);
                                }
                            }
                            else
                            {
                                LinuxEndInputPlayBack(State);
                            }
                        }
                    }
                }
#endif
                if (Event.type == KeyPress)
                {
                    if (Event.xkey.keycode == KEYCODE_PLUS)
                    {
                        if (ShiftKeyWasDown)
                        {
                            OpenGL.DebugLightBufferIndex += 1;
                        }
                        else
                        {
                            OpenGL.DebugLightBufferTexIndex += 1;
                        }
                    }
                    else if (Event.xkey.keycode == KEYCODE_MINUS)
                    {
                        if (ShiftKeyWasDown)
                        {
                            OpenGL.DebugLightBufferIndex -= 1;
                        }
                        else
                        {
                            OpenGL.DebugLightBufferTexIndex -= 1;
                        }
                    }
                    else if ((Event.xkey.keycode == KEYCODE_ENTER) && AltKeyWasDown)
                    {
                        ToggleFullscreen(display, window);
                    }
                    else if ((Event.xkey.keycode >= KEYCODE_F1) && (Event.xkey.keycode <= KEYCODE_F10))
                    {
                        Input->FKeyPressed[Event.xkey.keycode - KEYCODE_F1 + 1] = true;
                    }
                    else if ((Event.xkey.keycode >= KEYCODE_F11) && (Event.xkey.keycode <= KEYCODE_F12))
                    {
                        // NOTE(michiel): Because of X11 mapping we get to do the function keys in 2 steps :)
                        Input->FKeyPressed[Event.xkey.keycode - KEYCODE_F11 + 1] = true;
                    }
                }
            } break;

            default:
            break;
        }
    }
}

//
// NOTE(michiel): Platform file handling
//

struct linux_find_file
{
    DIR *Dir;
    struct dirent *FileData;
};

struct linux_platform_file_handle
{
    s32 LinuxHandle;
};

struct linux_platform_file_group
{
    char Wildcard[6];
    b32 FileAvailable;
    linux_find_file FindData;
};

enum pattern_match_flag
{
    PatternMatchFlag_None      = 0x00,
    PatternMatchFlag_MaySkip   = 0x01,
    PatternMatchFlag_Restarted = 0x02,
};

internal b32
MatchPattern(char *Pattern, char *String)
{
    b32 Result = false;
    u32 Flags = PatternMatchFlag_None;
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

internal b32
FindFileInFolder(char *Wildcard, linux_find_file *Finder)
{
    b32 Result = false;
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

    linux_find_file FindData;
    FindData.Dir = opendir(".");
    for (b32 Found = FindFileInFolder(LinuxFileGroup->Wildcard, &FindData);
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

//
// NOTE(michiel): MAIN
//

#if GAME_INTERNAL
global_variable debug_table GlobalDebugTable_;
debug_table *GlobalDebugTable = &GlobalDebugTable_;
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

int main(int argc, char **argv)
{
    DEBUGSetEventRecording(true);

    linux_state *State = &GlobalLinuxState;
    State->MemorySentinel.Prev = &State->MemorySentinel;
    State->MemorySentinel.Next = &State->MemorySentinel;

    LinuxGetEXEFileName(State);

    char LinuxEXEFullPath[LINUX_STATE_FILE_NAME_COUNT];
    LinuxBuildEXEPathFileName(State, "linux_handmade",
        sizeof(LinuxEXEFullPath), LinuxEXEFullPath);

    char SourceGameCodeDLLFullPath[LINUX_STATE_FILE_NAME_COUNT];
    LinuxBuildEXEPathFileName(State, "libHandmade.so",
                              sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);

    XInitThreads();

    GlobalWindowPositionX = 0;
    GlobalWindowPositionY = 0;

#if GAME_INTERNAL
    DEBUGGlobalShowCursor = true;
#endif

    /* NOTE(casey): 1080p display mode is 1920x1080 -> Half of that is 960x540
       1920 -> 2048 = 2048-1920 -> 128 pixels
       1080 -> 2048 = 2048-1080 -> pixels 968
       1024 + 128 = 1152
    */
    //GlobalBackbuffer = LinuxCreateOffscreenBuffer(192, 108);
    //GlobalBackbuffer = LinuxCreateOffscreenBuffer(480, 270);
    //GlobalBackbuffer = LinuxCreateOffscreenBuffer(960, 540);
    //GlobalBackbuffer = LinuxCreateOffscreenBuffer(1280, 720);
    GlobalBackbuffer = LinuxCreateOffscreenBuffer(1920, 1080);
    //GlobalBackbuffer = LinuxCreateOffscreenBuffer(1279, 719);

    /* Open Xlib Display */
    Display *display = XOpenDisplay(NULL);
    if (display)
    {
        LinuxLoadGlxExtensions();

        GLXFBConfig *FramebufferConfigs = LinuxGetOpenGLFramebufferConfig(display);
        GLXFBConfig FramebufferConfig = FramebufferConfigs[0];
        XFree(FramebufferConfigs);
        XVisualInfo *VisualInfo = glXGetVisualFromFBConfig(display, FramebufferConfig);

        if (VisualInfo)
        {
            VisualInfo->screen = DefaultScreen(display);

            XSetWindowAttributes WindowAttribs = {};
            Window Root = RootWindow(display, VisualInfo->screen);
            WindowAttribs.colormap = XCreateColormap(display, Root, VisualInfo->visual, AllocNone);
            WindowAttribs.border_pixel = 0;
            //WindowAttribs.override_redirect = 1;       // This in combination with CWOverrideRedirect produces a borderless window

            WindowAttribs.event_mask = (StructureNotifyMask | PropertyChangeMask |
                                        PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                                        KeyPressMask | KeyReleaseMask);

            Window GlWindow;
            GlWindow = XCreateWindow(display, Root,
                                     GlobalWindowPositionX, GlobalWindowPositionY,
                                     GlobalBackbuffer.Width, GlobalBackbuffer.Height,
                                     0, VisualInfo->depth, InputOutput, VisualInfo->visual,
                                     CWBorderPixel|CWColormap|CWEventMask/*|CWOverrideRedirect*/, &WindowAttribs);
            if (GlWindow)
            {
                XSizeHints SizeHints = {};
                SizeHints.x = GlobalWindowPositionX;
                SizeHints.y = GlobalWindowPositionY;
                SizeHints.width  = GlobalBackbuffer.Width;
                SizeHints.height = GlobalBackbuffer.Height;
                SizeHints.flags = USSize | USPosition; //US vs PS?

                XSetNormalHints(display, GlWindow, &SizeHints);
                XSetStandardProperties(display, GlWindow, "Handmade Hero", "glsync text", None, NULL, 0, &SizeHints);

                GLXContext OpenGLContext = LinuxInitOpenGL(display, GlWindow, FramebufferConfig);

                Atom WmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
                XSetWMProtocols(display, GlWindow, &WmDeleteWindow, 1);

                // NOTE(michiel): Hide cursor
                LinuxCreateHiddenCursor(display, GlWindow);
#if GAME_INTERNAL
                LinuxShowCursor(display, GlWindow);
#else
                LinuxHideCursor(display, GlWindow);
                ToggleFullscreen(display, GlWindow);
#endif

                s32 NrProcessors = get_nprocs();
                // NOTE(michiel): Minus the main thread of course
                --NrProcessors;

                // NOTE(michiel): Get number of extra threads to use
                s32 HighPrioTasks = 1;
                s32 LowPrioTasks = 1;
                if (NrProcessors > 0)
                {
                    HighPrioTasks = Maximum(1, (s32)((f32)NrProcessors * 0.8));
                    LowPrioTasks = Maximum(1, NrProcessors - HighPrioTasks);
                }

                linux_thread_startup *HighPriStartups = (linux_thread_startup *)alloca(HighPrioTasks * sizeof(linux_thread_startup));
                for (u32 HighStartupIndex = 0;
                     HighStartupIndex < HighPrioTasks;
                     ++HighStartupIndex)
                {
                    linux_thread_startup *ThreadStartup = HighPriStartups + HighStartupIndex;
                    ZeroStruct(*ThreadStartup);
                }
                platform_work_queue HighPriorityQueue = {};
                LinuxMakeQueue(&HighPriorityQueue, HighPrioTasks, HighPriStartups);

                linux_thread_startup *LowPriStartups = (linux_thread_startup *)alloca(LowPrioTasks * sizeof(linux_thread_startup));
                for (u32 LowStartupIndex = 0;
                     LowStartupIndex < LowPrioTasks;
                     ++LowStartupIndex)
                {
                    linux_thread_startup *ThreadStartup = LowPriStartups + LowStartupIndex;
                    ZeroStruct(*ThreadStartup);
                }
                platform_work_queue LowPriorityQueue = {};
                LinuxMakeQueue(&LowPriorityQueue, LowPrioTasks, LowPriStartups);

                linux_sound_output SoundOutput = {};

                // TODO(michiel): Use XRandR only for this query?
                int MonitorRefreshHz = 60;
                f32 GameUpdateHz = (f32)(MonitorRefreshHz);

                if (LinuxInitializeSound("default", 48000, 4, 512, &SoundOutput))
                {
                    LinuxStartPlayingSound();

                    GlobalRunning = true;

                    // TODO(casey): Let's make this our first growable arena!
                    memory_arena FrameTempArena = {};

                    // TODO(casey): Decide what our pushbuffer size is!
                    u32 PushBufferSize = Megabytes(64);
                    platform_memory_block *PushBufferBlock = LinuxAllocateMemory(PushBufferSize, PlatformMemory_NotRestored);
                    u8 *PushBuffer = PushBufferBlock->Base;

                    u32 MaxVertexCount = 65536;
                    platform_memory_block *VertexArrayBlock = LinuxAllocateMemory(MaxVertexCount*sizeof(textured_vertex), PlatformMemory_NotRestored);
                    textured_vertex *VertexArray = (textured_vertex *)VertexArrayBlock->Base;
                    platform_memory_block *BitmapArrayBlock = LinuxAllocateMemory(MaxVertexCount*sizeof(loaded_bitmap *), PlatformMemory_NotRestored);
                    loaded_bitmap **BitmapArray = (loaded_bitmap **)BitmapArrayBlock->Base;
                    lighting_box *LightBoxes = (lighting_box *)
                        LinuxAllocateMemory(LIGHT_DATA_WIDTH*sizeof(lighting_box),
                                            PlatformMemory_NotRestored)->Base;

                    game_render_commands RenderCommands = DefaultRenderCommands(
                        PushBufferSize, PushBuffer,
                        (u32)GlobalBackbuffer.Width,
                        (u32)GlobalBackbuffer.Height,
                        MaxVertexCount, VertexArray, BitmapArray,
                        &OpenGL.WhiteBitmap,
                        LightBoxes);

                    // TODO(michiel): Pool with bitmap mmap
                    // TODO(casey): Remove MaxPossibleOverrun?
                    size_t NrSamples = SoundOutput.SamplesPerSecond * AUDIO_CHANNELS;     // 1 second
                    size_t SampleSize = NrSamples * sizeof(s16);
                    u32 MaxPossibleOverrun = AUDIO_CHANNELS * 8 * sizeof(u16);
                    s16 *Samples = (s16 *)mmap(NULL, SampleSize + MaxPossibleOverrun,
                                               PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
                    SoundOutput.SafetyBytes = (SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample / GameUpdateHz) / 3;

#if GAME_INTERNAL
                    void *BaseAddress = (void *)Terabytes((u64)2);
#else
                    void *BaseAddress = 0;
#endif

                    game_memory GameMemory = {};
#if GAME_INTERNAL
                    GameMemory.DebugTable = GlobalDebugTable;
#endif
                    GameMemory.HighPriorityQueue = &HighPriorityQueue;
                    GameMemory.LowPriorityQueue = &LowPriorityQueue;
                    GameMemory.PlatformAPI.AddEntry = LinuxAddEntry;
                    GameMemory.PlatformAPI.CompleteAllWork = LinuxCompleteAllWork;

                    GameMemory.PlatformAPI.GetAllFilesOfTypeBegin = LinuxGetAllFilesOfTypeBegin;
                    GameMemory.PlatformAPI.GetAllFilesOfTypeEnd = LinuxGetAllFilesOfTypeEnd;
                    GameMemory.PlatformAPI.OpenNextFile = LinuxOpenNextFile;
                    GameMemory.PlatformAPI.ReadDataFromFile = LinuxReadDataFromFile;
                    GameMemory.PlatformAPI.FileError = LinuxFileError;

                    GameMemory.PlatformAPI.AllocateMemory = LinuxAllocateMemory;
                    GameMemory.PlatformAPI.DeallocateMemory = LinuxDeallocateMemory;

#if GAME_INTERNAL
                    GameMemory.PlatformAPI.DEBUGFreeFileMemory = DEBUGPlatformFreeFileMemory;
                    GameMemory.PlatformAPI.DEBUGReadEntireFile = DEBUGPlatformReadEntireFile;
                    GameMemory.PlatformAPI.DEBUGWriteEntireFile = DEBUGPlatformWriteEntireFile;
                    GameMemory.PlatformAPI.DEBUGExecuteSystemCommand = DEBUGExecuteSystemCommand;
                    GameMemory.PlatformAPI.DEBUGGetProcessState = DEBUGGetProcessState;
                    GameMemory.PlatformAPI.DEBUGGetMemoryStats = LinuxGetMemoryStats;
#endif
                    u32 TextureOpCount = 1024;
                    platform_texture_op_queue *TextureOpQueue = &GameMemory.TextureOpQueue;
                    TextureOpQueue->FirstFree =
                        (texture_op *)mmap(0, sizeof(texture_op)*TextureOpCount,
                                           PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
                    for(u32 TextureOpIndex = 0;
                        TextureOpIndex < (TextureOpCount - 1);
                        ++TextureOpIndex)
                    {
                        texture_op *Op = TextureOpQueue->FirstFree + TextureOpIndex;
                        Op->Next = TextureOpQueue->FirstFree + TextureOpIndex + 1;
                    }

                    // Platform = GameMemory.PlatformAPI;

                    if (Samples != MAP_FAILED)
                    {
                        u32 NrJoysticks = LinuxFindJoysticks();

                        game_input Input[2] = {};
                        game_input *NewInput = &Input[0];
                        game_input *OldInput = &Input[1];

                        struct timespec LastCounter = LinuxGetWallClock();
                        struct timespec FlipWallClock = LinuxGetWallClock();

                        int DebugTimeMarkerIndex = 0;
                        linux_debug_time_marker DebugTimeMarkers[30] = {0};

                        u32 AudioLatencyBytes = 0;
                        f32 AudioLatencySeconds = 0.0f;
                        b32 SoundIsValid = false;

                        linux_game_code Game = {};
                        LinuxLoadGameCode(&Game, SourceGameCodeDLLFullPath, LinuxFileId(SourceGameCodeDLLFullPath));

                        DEBUGSetEventRecording(Game.IsValid);

                        // NOTE(michiel): And display the window on the screen
                        XMapRaised(display, GlWindow);
                        u32 ExpectedFramesPerUpdate = 1;
                        f32 TargetSecondsPerFrame = (f32)ExpectedFramesPerUpdate / (f32)GameUpdateHz;
                        
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
                                for(u32 ButtonIndex = 0;
                                    ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
                                    ++ButtonIndex)
                                {
                                    NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                                        OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                                }
                                for(u32 ButtonIndex = 0;
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
                                LinuxProcessPendingMessages(State, display, GlWindow, WmDeleteWindow,
                                                            NewKeyboardController, NewInput, &MouseP, &Dimension);
                            }
                            rectangle2i DrawRegion;
                            {
                                TIMED_BLOCK("Aspect fitting");
                                DrawRegion = AspectRatioFit(RenderCommands.Settings.Width, RenderCommands.Settings.Height,
                                                            Dimension.Width, Dimension.Height);
                            }
                            
                            {
                                TIMED_BLOCK("Mouse Position");
                                f32 MouseX = (f32)MouseP.x;
                                f32 MouseY = (f32)((Dimension.Height - 1) - MouseP.y);
                                
                                f32 MouseU = Clamp01MapToRange((f32)DrawRegion.MinX, MouseX, (f32)DrawRegion.MaxX);
                                f32 MouseV = Clamp01MapToRange((f32)DrawRegion.MinY, MouseY, (f32)DrawRegion.MaxY);
                                
                                NewInput->MouseX = (f32)RenderCommands.Settings.Width*MouseU;
                                NewInput->MouseY = (f32)RenderCommands.Settings.Height*MouseV;
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
                                if(State->InputRecordingIndex)
                                {
                                    LinuxRecordInput(State, NewInput);
                                }

                                if(State->InputPlayingIndex)
                                {
                                    game_input Temp = *NewInput;
                                    LinuxPlayBackInput(State, NewInput);
                                    for(u32 MouseButtonIndex = 0;
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
                                u32 PlayCursor = SoundOutput.Buffer.ReadIndex;
                                u32 WriteCursor = PlayCursor + AUDIO_WRITE_SAFE_SAMPLES * SoundOutput.BytesPerSample;
                                if (!SoundIsValid)
                                {
                                    SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
                                    SoundIsValid = true;
                                }

                                u32 ByteToLock = ((SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) %
                                                  SoundOutput.Buffer.Size);

                                u32 ExpectedSoundBytesPerFrame =
                                    (u32)((f32)(SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample) /
                                          GameUpdateHz);

                                u32 ExpectedFrameBoundaryByte = PlayCursor + ExpectedSoundBytesPerFrame;

                                u32 SafeWriteCursor = WriteCursor + SoundOutput.SafetyBytes;
                                b32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);

                                u32 TargetCursor = 0;
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

                                u32 BytesToWrite = 0;
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

                                u32 UnwrappedWriteCursor = WriteCursor;
                                if(UnwrappedWriteCursor < PlayCursor)
                                {
                                    UnwrappedWriteCursor += SoundOutput.Buffer.Size;
                                }
                                AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
                                AudioLatencySeconds =
                                    (((f32)AudioLatencyBytes / (f32)SoundOutput.BytesPerSample) / (f32)SoundOutput.SamplesPerSecond);
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
                            b32 ExecutableNeedsToBeReloaded = (GameLibId != Game.GameLibID);

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
                                b32 IsValid = false;
                                for(u32 LoadTryIndex = 0;
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
                                f32 WorkSecondsElapsed = LinuxGetSecondsElapsed(LastCounter, WorkCounter);

                                f32 SecondsElapsedForFrame = WorkSecondsElapsed;
                                if (SecondsElapsedForFrame < TargetSecondsPerFrame)
                                {
                                    u32 SleepUs = (u32)(0.99e6f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
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

                            LinuxDisplayBufferInWindow(&HighPriorityQueue, &RenderCommands, display, GlWindow,
                                                       DrawRegion, Dimension.Width, Dimension.Height,
                                                       &FrameTempArena);

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
                            f32 MeasuredSecondsPerFrame = LinuxGetSecondsElapsed(LastCounter, EndCounter);
                            f32 ExactTargetFramesPerUpdate = MeasuredSecondsPerFrame*(f32)MonitorRefreshHz;
                            u32 NewExpectedFramesPerUpdate = RoundReal32ToInt32(ExactTargetFramesPerUpdate);
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
                }
                else
                {
                    fprintf(stderr, "Unable to initialize sound!\n");
                }
            }
            else
            {
                fprintf(stderr, "window creation failed\n");
            }
        }
        else
        {
            fprintf(stderr, "failed to choose visual, exiting\n");
        }
    }
    else
    {
        fprintf(stderr, "Can't open display\n");
    }

    return 0;
}