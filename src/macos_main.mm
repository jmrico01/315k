#include "macos_main.h"

#undef internal
#include <Cocoa/Cocoa.h>
#define internal static

#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

#include <mach/mach_time.h>
#include <sys/mman.h>       // memory functions
#include <sys/stat.h>       // file stat functions
#include <fcntl.h>          // file open/close functions
#include <dlfcn.h>          // dynamic linking functions

#include "km_defines.h"
#include "km_debug.h"

// Let the command line override
//#ifndef HANDMADE_USE_VSYNC
//#define HANDMADE_USE_VSYNC 1
//#endif

global_var bool32 running_;
global_var NSOpenGLContext* glContext_;
global_var char pathToApp_[MACOS_STATE_FILE_NAME_COUNT];

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

internal inline uint32 SafeTruncateUInt64(uint64 value)
{
	// TODO defines for maximum values
	DEBUG_ASSERT(value <= 0xFFFFFFFF);
	uint32 result = (uint32)value;
	return result;
}

// TODO maybe factor out all unix-style shared functions
// (e.g. library loading, file i/o)

// NOTE Explicit wrappers around dlsym, dlopen and dlclose
internal void* MacOSLoadFunction(void* libHandle, const char* name)
{
    void* symbol = dlsym(libHandle, name);
    if (!symbol) {
        DEBUG_PRINT("dlsym failed: %s\n", dlerror());
    }
    // TODO(michiel): Check if lib with underscore exists?!
    return symbol;
}

internal void* MacOSLoadLibrary(const char* libName)
{
    void* handle = dlopen(libName, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        DEBUG_PRINT("dlopen failed: %s\n", dlerror());
    }
    return handle;
}

/*internal void MacOSUnloadLibrary(void* handle)
{
    if (handle != NULL) {
        dlclose(handle);
        handle = NULL;
    }
}*/

#if GAME_INTERNAL

DEBUG_PLATFORM_PRINT_FUNC(DEBUGPlatformPrint)
{
	NSString* formatStr = [NSString stringWithUTF8String:format];
    va_list args;
    va_start(args, format);
    NSLogv(formatStr, args);
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

    char fullPath[MACOS_STATE_FILE_NAME_COUNT];
    CatStrings(StringLength(pathToApp_), pathToApp_,
        StringLength(fileName), fileName,
        MACOS_STATE_FILE_NAME_COUNT, fullPath);
    int32 fileHandle = open(fullPath, O_RDONLY);
    if (fileHandle >= 0) {
        off_t fileSize64 = lseek(fileHandle, 0, SEEK_END);
        lseek(fileHandle, 0, SEEK_SET);

        if (fileSize64 > 0) {
            uint32 fileSize32 = SafeTruncateUInt64(fileSize64);
            result.data = mmap(NULL, fileSize32, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (result.data) {
                ssize_t bytesRead = read(fileHandle,
                    result.data, fileSize64);
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

    char fullPath[MACOS_STATE_FILE_NAME_COUNT];
    CatStrings(StringLength(pathToApp_), pathToApp_,
    	StringLength(fileName), fileName,
    	MACOS_STATE_FILE_NAME_COUNT, fullPath);
    int32 fileHandle = open(fullPath, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
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

#if 0

osx_game_code OSXLoadGameCode(const char* SourceDLName)
{
	osx_game_code Result = {};

	// TODO(casey): Need to get the proper path here!
	// TODO(casey): Automatic determination of when updates are necessary

	Result.DLLastWriteTime = OSXGetLastWriteTime(SourceDLName);

	Result.GameCodeDL = dlopen(SourceDLName, RTLD_LAZY|RTLD_GLOBAL);
	if (Result.GameCodeDL)
	{
		Result.UpdateAndRender = (game_update_and_render*)
			dlsym(Result.GameCodeDL, "GameUpdateAndRender");

		Result.GetSoundSamples = (game_get_sound_samples*)
			dlsym(Result.GameCodeDL, "GameGetSoundSamples");

		Result.DEBUGFrameEnd = (debug_game_frame_end*)
			dlsym(Result.GameCodeDL, "DEBUGGameFrameEnd");

		Result.IsValid = Result.UpdateAndRender && Result.GetSoundSamples;
	}

	if (!Result.IsValid)
	{
		Result.UpdateAndRender = 0;
		Result.GetSoundSamples = 0;
	}

	return Result;
}


void OSXUnloadGameCode(osx_game_code* GameCode)
{
	if (GameCode->GameCodeDL)
	{
		dlclose(GameCode->GameCodeDL);
		GameCode->GameCodeDL = 0;
	}

	GameCode->IsValid = false;
	GameCode->UpdateAndRender = 0;
	GameCode->GetSoundSamples = 0;
}

#endif

#define LOAD_GL_FUNCTION(name) \
    glFuncs->name = (name##Func*)MacOSLoadFunction(glLib, #name); \
    if (!glFuncs->name) { \
        DEBUG_PANIC("OpenGL function load failed: %s", #name); \
    }

internal bool32 MacOSInitOpenGL(
    OpenGLFunctions* glFuncs,
    int width, int height)
{
	void* glLib = MacOSLoadLibrary("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL");
	if (!glLib) {
		DEBUG_PRINT("Failed to load OpenGL library\n");
		return false;
	}

	// Generate function loading code
#define FUNC(returntype, name, ...) LOAD_GL_FUNCTION(name);
	GL_FUNCTIONS_BASE
#undef FUNC

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

	// Generate function loading code
#define FUNC(returntype, name, ...) LOAD_GL_FUNCTION(name);
	GL_FUNCTIONS_ALL
#undef FUNC

	const GLubyte* glslString =
        glFuncs->glGetString(GL_SHADING_LANGUAGE_VERSION);
    DEBUG_PRINT("GL_SHADING_LANGUAGE_VERSION: %s\n", glslString);

    return true;
}

@interface KMAppDelegate : NSObject<NSApplicationDelegate, NSWindowDelegate>
@end

@implementation KMAppDelegate

- (void)applicationDidFinishLaunching:(id)sender
{
	#pragma unused(sender)
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
	#pragma unused(sender)
	return YES;
}

- (void)applicationWillTerminate:(NSApplication*)sender
{
	#pragma unused(sender)
}

- (void)windowWillClose:(id)sender
{
	running_ = false;
}

#if 0
- (void)windowDidResize:(NSNotification*)notification
{
	NSWindow* window = [notification object];

	//Assert(GlobalGLContext == [NSOpenGLContext currentContext]);
	//[GlobalGLContext update];
	[GlobalGLContext makeCurrentContext];
	[GlobalGLContext update];

	// OpenGL reshape. Typically done in the view
	//glLoadIdentity();

	NSRect ContentViewFrame = [[window contentView] frame];
	glViewport(0, 0, ContentViewFrame.size.width, ContentViewFrame.size.height);
	[GlobalGLContext update];
}
#endif

@end

void MacOSCreateMainMenu()
{
	// Create the Menu. Two options right now:
	//	Toggle Full Screen
	//  Quit
    NSMenu* menu = [NSMenu new];

	NSMenuItem* appMenuItem = [NSMenuItem new];
	[menu addItem:appMenuItem];

	[NSApp setMainMenu:menu];

	NSMenu* appMenu = [NSMenu new];

    //NSString* appName = [[NSProcessInfo processInfo] processName];
    NSString* appName = @"315k";

    NSString* toggleFullScreenTitle = @"Toggle Full Screen";
    NSMenuItem* toggleFullScreenMenuItem = [[NSMenuItem alloc]
    	initWithTitle:toggleFullScreenTitle
    	action:@selector(toggleFullScreen:)
    	keyEquivalent:@"f"];
    [appMenu addItem:toggleFullScreenMenuItem];

    NSString* quitTitle = [@"Quit " stringByAppendingString:appName];
    NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle
    	action:@selector(terminate:)
    	keyEquivalent:@"q"];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
}

void MacOSKeyProcessing(bool32 isDown, uint32 key,
	int commandKeyFlag, int ctrlKeyFlag, int altKeyFlag,
	GameInput* input)
{
	if (key == 'q' && isDown && commandKeyFlag) {
		running_ = false;
	}
}

void MacOSProcessPendingMessages(GameInput* input)
{
	NSEvent* event;

	do {
		event = [NSApp nextEventMatchingMask:NSAnyEventMask
			untilDate:nil
			inMode:NSDefaultRunLoopMode
			dequeue:YES];

		switch ([event type]) {
			case NSKeyDown: {
				unichar ch = [[event charactersIgnoringModifiers] characterAtIndex:0];
				int modifierFlags = [event modifierFlags];
				int commandKeyFlag = modifierFlags & NSCommandKeyMask;
				int ctrlKeyFlag = modifierFlags & NSControlKeyMask;
				int altKeyFlag = modifierFlags & NSAlternateKeyMask;

				int keyDownFlag = 1;

				MacOSKeyProcessing(keyDownFlag, ch,
					commandKeyFlag, ctrlKeyFlag, altKeyFlag,
					input);
			} break;

			case NSKeyUp: {
				unichar ch = [[event charactersIgnoringModifiers] characterAtIndex:0];
				int modifierFlags = [event modifierFlags];
				int commandKeyFlag = modifierFlags & NSCommandKeyMask;
				int ctrlKeyFlag = modifierFlags & NSControlKeyMask;
				int altKeyFlag = modifierFlags & NSAlternateKeyMask;

				int keyDownFlag = 0;

				MacOSKeyProcessing(keyDownFlag, ch,
					commandKeyFlag, ctrlKeyFlag, altKeyFlag,
					input);
			} break;

			default: {
				[NSApp sendEvent:event];
			} break;
		}
	} while (event != nil);
}

///////////////////////////////////////////////////////////////////////
@interface KMOpenGLView : NSOpenGLView
{
}
@end

@implementation KMOpenGLView

- (id)init
{
	self = [super init];
	return self;
}


- (void)prepareOpenGL
{
	[super prepareOpenGL];
	[[self openGLContext] makeCurrentContext];
}

- (void)reshape
{
	[super reshape];

	NSRect bounds = [self bounds];
	//[[self openGLContext] makeCurrentContext];
	[glContext_ makeCurrentContext];
	[glContext_ update];
	glViewport(0, 0, bounds.size.width, bounds.size.height);
}


@end


///////////////////////////////////////////////////////////////////////
// Startup

int main(int argc, const char* argv[])
{
	#pragma unused(argc)
	#pragma unused(argv)

	//return NSApplicationMain(argc, argv); // ??
	@autoreleasepool
	{

	debugPrint_ = DEBUGPlatformPrint;

	NSApplication* app = [NSApplication sharedApplication];
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

	MacOSCreateMainMenu();

	NSString* path = [[NSFileManager defaultManager] currentDirectoryPath];
	const char* pathStr = [path UTF8String];
	strncpy(pathToApp_, pathStr, [path length]);
	DEBUG_PRINT("Path to application: %s", pathToApp_);

	NSFileManager* fileManager = [NSFileManager defaultManager];
	NSString* appPath = [NSString stringWithFormat:@"%@/Contents/Resources",
		[[NSBundle mainBundle] bundlePath]];
	if ([fileManager changeCurrentDirectoryPath:appPath] == NO) {
		// TODO fix this, it's crashing
		//DEBUG_PANIC("Failed to change application data path\n");
	}

	KMAppDelegate* appDelegate = [[KMAppDelegate alloc] init];
	[app setDelegate:appDelegate];
    [NSApp finishLaunching];
    DEBUG_PRINT("Finished launching NSApp\n");

    NSOpenGLPixelFormatAttribute openGLAttributes[] = {
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer, // Enable for vsync
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        0
    };
	NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc]
		initWithAttributes:openGLAttributes];
    glContext_ = [[NSOpenGLContext alloc] initWithFormat:pixelFormat
    	shareContext:NULL];
    DEBUG_PRINT("Initialized GL context\n");

	///////////////////////////////////////////////////////////////////
	// NSWindow and NSOpenGLView
	//
	// Create the main window and the content view
	NSRect screenRect = [[NSScreen mainScreen] frame];
	float width = 1280.0f;
	float height = 800.0f;

	NSRect InitialFrame = NSMakeRect(
		(screenRect.size.width - width) * 0.5,
		(screenRect.size.height - height) * 0.5,
		width, height
	);

	NSWindow* window = [[NSWindow alloc] initWithContentRect:InitialFrame
		styleMask:NSTitledWindowMask
			| NSClosableWindowMask
			| NSMiniaturizableWindowMask
			| NSResizableWindowMask
		backing:NSBackingStoreBuffered
		defer:NO];

	[window setBackgroundColor: NSColor.blackColor];
	[window setDelegate:appDelegate];

	NSView* cv = [window contentView];
	[cv setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	[cv setAutoresizesSubviews:YES];

#if 1
	KMOpenGLView* GLView = [[KMOpenGLView alloc] init];
	[GLView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	[GLView setPixelFormat:pixelFormat];
	[GLView setOpenGLContext:glContext_];
	[GLView setFrame:[cv bounds]];

	[cv addSubview:GLView];
#endif

    [pixelFormat release];

	[window setMinSize:NSMakeSize(160, 90)];
	[window setTitle:@"315k"];
	[window makeKeyAndOrderFront:nil];
    DEBUG_PRINT("Initialized window and OpenGL view\n");

    GLint swapInt = 1;
    //GLint swapInt = 0;
	[glContext_ setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	[glContext_ setView:[window contentView]];
	[glContext_ makeCurrentContext];

    PlatformFunctions platformFuncs = {};
	platformFuncs.DEBUGPlatformPrint = DEBUGPlatformPrint;
	platformFuncs.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
	platformFuncs.DEBUGPlatformReadFile = DEBUGPlatformReadFile;
	platformFuncs.DEBUGPlatformWriteFile = DEBUGPlatformWriteFile;
    if (!MacOSInitOpenGL(&platformFuncs.glFunctions,
    (int)width, (int)height)) {
        return 1;
    }
    DEBUG_PRINT("Initialized MacOS OpenGL\n");

#if 0
	// Default to full screen mode at startup...
    NSDictionary* fullScreenOptions =
		[NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES] forKey:NSFullScreenModeSetting];

	[[window contentView] enterFullScreenMode:[NSScreen mainScreen] withOptions:fullScreenOptions];
#endif

#if GAME_INTERNAL
	void* baseAddress = (void*)TERABYTES((uint64)2);;
#else
	void* baseAddress = 0;
#endif
    
    GameMemory gameMemory = {};
    gameMemory.DEBUGShouldInitGlobalFuncs = true;
	gameMemory.permanentStorageSize = MEGABYTES(64);
	gameMemory.transientStorageSize = GIGABYTES(1);

	// TODO Look into using large virtual pages for this
    // potentially big allocation
	uint64 totalSize = gameMemory.permanentStorageSize
        + gameMemory.transientStorageSize;
	// TODO check allocation fail?
	gameMemory.permanentStorage = mmap(baseAddress, (size_t)totalSize,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	gameMemory.transientStorage = ((uint8*)gameMemory.permanentStorage +
		gameMemory.permanentStorageSize);
	DEBUG_PRINT("Initialized game memory\n");

	GameInput input[2];
	GameInput* newInput = &input[0];
	GameInput* oldInput = &input[1];

	///////////////////////////////////////////////////////////////////
	// Run loop
	//
	//uint tickCounter = 0;
	//uint64 currentTime = mach_absolute_time();
	//GameData.LastCounter = currentTime;
	//float32 frameTime = 0.0f;
	running_ = true;

	while (running_) {
		MacOSProcessPendingMessages(newInput);

		[glContext_ makeCurrentContext];

		CGRect windowFrame = [window frame];
		//CGRect contentViewFrame = [[window contentView] frame];

#if 0
		DEBUG_PRINT("WindowFrame: [%.0f, %.0f]",
			windowFrame.size.width, windowFrame.size.height);
		DEBUG_PRINT("    ContentFrame: [%.0f, %.0f]\n",
			contentViewFrame.size.width, contentViewFrame.size.height);
#endif

		CGPoint mousePosInScreen = [NSEvent mouseLocation];
		BOOL mouseInWindowFlag = NSPointInRect(mousePosInScreen, windowFrame);
		CGPoint mousePosInView = {};

		if (mouseInWindowFlag) {
			// NOTE(jeff): Use this instead of convertRectFromScreen: if you want to support Snow Leopard
			// NSPoint PointInWindow = [[self window] convertScreenToBase:[NSEvent mouseLocation]];

			// We don't actually care what the mouse screen coordinates are,
			// we just want the coordinates relative to the content view
			NSRect rectInWindow = [window convertRectFromScreen:NSMakeRect(mousePosInScreen.x, mousePosInScreen.y, 1, 1)];
			NSPoint pointInWindow = rectInWindow.origin;
			mousePosInView = [[window contentView]
				convertPoint:pointInWindow fromView:nil];
		}

		//uint32 mouseButtonMask = [NSEvent pressedMouseButtons];

		/*OSXProcessFrameAndRunGameLogic(&GameData, ContentViewFrame,
										MouseInWindowFlag, mousePosInView,
										MouseButtonMask);*/

		// flushes and forces vsync
		glClearColor(0.0f, 0.0f, 0.1f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		[glContext_ flushBuffer];
		//glFlush(); // no vsync

		//uint32 endCounter = mach_absolute_time();

		/*float32 MeasuredSecondsPerFrame = OSXGetSecondsElapsed(GameData.LastCounter, EndCounter);
		float32 ExactTargetFramesPerUpdate = MeasuredSecondsPerFrame * (f32)GameData.MonitorRefreshHz;
		uint32 NewExpectedFramesPerUpdate = RoundReal32ToInt32(ExactTargetFramesPerUpdate);
		GameData.ExpectedFramesPerUpdate = NewExpectedFramesPerUpdate;

		GameData.TargetSecondsPerFrame = MeasuredSecondsPerFrame;*/

		//frameTime += MeasuredSecondsPerFrame;
		//GameData.LastCounter = EndCounter;

		GameInput* temp = newInput;
		newInput = oldInput;
		oldInput = temp;
		ClearInput(newInput, oldInput);

#if 0
		++tickCounter;
		if (tickCounter == 60)
		{
			float avgFrameTime = frameTime / 60.0f;
			tickCounter = 0;
			frameTime = 0.0f;
			//printf("frame time = %f\n", avgFrameTime);
		}
#endif
	}


	//OSXStopCoreAudio(&GameData.SoundOutput);

	} // @autoreleasepool
}

#include "km_input.cpp"