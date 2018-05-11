#include "macos_main.h"

#undef internal
#include <Cocoa/Cocoa.h>
#define internal static

#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>

#include <mach/mach_time.h>

#include "km_defines.h"
#include "km_debug.h"

// Let the command line override
//#ifndef HANDMADE_USE_VSYNC
//#define HANDMADE_USE_VSYNC 1
//#endif

global_var bool32 running_;
global_var NSOpenGLContext* glContext_;
global_var NSString* pathToApp_;

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

				int KeyDownFlag = 1;

				MacOSKeyProcessing(KeyDownFlag, ch,
					commandKeyFlag, ctrlKeyFlag, altKeyFlag,
					input);
			} break;

			case NSKeyUp: {
				unichar ch = [[event charactersIgnoringModifiers] characterAtIndex:0];
				int modifierFlags = [event modifierFlags];
				int commandKeyFlag = modifierFlags & NSCommandKeyMask;
				int ctrlKeyFlag = modifierFlags & NSControlKeyMask;
				int altKeyFlag = modifierFlags & NSAlternateKeyMask;

				int KeyDownFlag = 0;

				MacOSKeyProcessing(KeyDownFlag, ch,
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
	NSLog(@"glViewport called on resize");

#if 0
	printf("KMOpenGLView reshape: [%.0f, %.0f] [%.0f, %.0f]\n",
			bounds.origin.x, bounds.origin.y,
			bounds.size.width, bounds.size.height);

	glLoadIdentity();
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
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

	NSApplication* app = [NSApplication sharedApplication];
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

	MacOSCreateMainMenu();

	pathToApp_ = [[NSFileManager defaultManager] currentDirectoryPath];
	NSLog(@"Path to executable: %@", pathToApp_);

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

	// game_data holds the OS X platform layer non-Cocoa data structures
	//osx_game_data GameData = {};

	//OSXSetupGameData(&GameData, [glContext_ CGLContextObj]);

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

	//OSXSetupGameRenderBuffer(&GameData, Width, Height, BytesPerPixel);

    GLint swapInt = 1;
    //GLint swapInt = 0;
	[glContext_ setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	[glContext_ setView:[window contentView]];

	[glContext_ makeCurrentContext];

	///////////////////////////////////////////////////////////////////
	// Non-Cocoa OpenGL
	//
	//OSXSetupOpenGL(&GameData);

#if 0
	// Default to full screen mode at startup...
    NSDictionary* fullScreenOptions =
		[NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES] forKey:NSFullScreenModeSetting];

	[[window contentView] enterFullScreenMode:[NSScreen mainScreen] withOptions:fullScreenOptions];
#endif

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
		printf("WindowFrame: [%.0f, %.0f]",
			windowFrame.size.width, windowFrame.size.height);
		printf("    ContentFrame: [%.0f, %.0f]\n",
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