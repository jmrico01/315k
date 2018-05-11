#include "macos_main.h"

#include <Cocoa/Cocoa.h>

#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>

#include <mach/mach_time.h>

#include "km_defines.h"
#include "km_debug.h"

//#define Maximum(A, B) ((A > B) ? (A) : (B))

//#import "handmade_platform.h"
//#import "handmade_memory.h"
//#import "osx_handmade.h"

//#import "handmade_intrinsics.h"

// Let the command line override
//#ifndef HANDMADE_USE_VSYNC
//#define HANDMADE_USE_VSYNC 1
//#endif

global_var bool32 running_;
global_var NSOpenGLContext* glContext_;
global_var NSString* pathToApp_;

///////////////////////////////////////////////////////////////////////
// Application Delegate

@interface KMAppDelegate : NSObject<NSApplicationDelegate, NSWindowDelegate>
@end

@implementation KMAppDelegate


///////////////////////////////////////////////////////////////////////
// Application delegate methods
//
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

	printf("applicationWillTerminate\n");
}



///////////////////////////////////////////////////////////////////////
// Window delegate methods
//
- (void)windowWillClose:(id)sender
{
	printf("Main window will close. Stopping game.\n");

	//OSXStopGame();
}


- (NSSize)windowWillResize:(NSWindow*)window
                    toSize:(NSSize)frameSize
{
	// Maintain the proper aspect ratio...
	//frameSize.height = frameSize.width / GlobalAspectRatio;

	NSRect windowRect = [window frame];
	NSRect contentRect = [window contentRectForFrameRect:windowRect];

	float32 widthAdd = (windowRect.size.width - contentRect.size.width);
	float32 heightAdd = (windowRect.size.height - contentRect.size.height);

	float32 renderWidth = 1920.0f;
	float32 renderHeight = 1080.0f;
	//r32 RenderWidth = 2560;
	//r32 RenderHeight = 1440;

	float32 newCy = (renderHeight * (frameSize.width - widthAdd))
		/ renderWidth;

	frameSize.height = newCy + heightAdd;

	return frameSize;
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
//
// Application Delegate
///////////////////////////////////////////////////////////////////////


void OSXCreateMainMenu()
{
	// Create the Menu. Two options right now:
	//   Toggle Full Screen
	//   Quit
    NSMenu* menubar = [NSMenu new];

	NSMenuItem* appMenuItem = [NSMenuItem new];
	[menubar addItem:appMenuItem];

	[NSApp setMainMenu:menubar];

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

void OSXKeyProcessing(bool32 IsDown, uint32 Key,
	int CommandKeyFlag, int ControlKeyFlag, int AlternateKeyFlag,
	game_input* Input, osx_game_data* GameData)
{
	game_controller_input* Controller = GetController(Input, 0);

	switch (Key)
	{
		case 'w':
			OSXProcessKeyboardMessage(&Controller->MoveUp, IsDown);
			break;

		case 'a':
			OSXProcessKeyboardMessage(&Controller->MoveLeft, IsDown);
			break;

		case 's':
			OSXProcessKeyboardMessage(&Controller->MoveDown, IsDown);
			break;

		case 'd':
			OSXProcessKeyboardMessage(&Controller->MoveRight, IsDown);
			break;

		case 'q':
			if (IsDown && CommandKeyFlag)
			{
				OSXStopGame();
			}
			else
			{
				OSXProcessKeyboardMessage(&Controller->LeftShoulder, IsDown);
			}
			break;

		case 'e':
			OSXProcessKeyboardMessage(&Controller->RightShoulder, IsDown);
			break;

		case ' ':
			OSXProcessKeyboardMessage(&Controller->Start, IsDown);
			break;

		case 27:
			OSXProcessKeyboardMessage(&Controller->Back, IsDown);
			break;

		case 0xF700: //NSUpArrowFunctionKey
			OSXProcessKeyboardMessage(&Controller->ActionUp, IsDown);
			break;

		case 0xF702: //NSLeftArrowFunctionKey
			OSXProcessKeyboardMessage(&Controller->ActionLeft, IsDown);
			break;

		case 0xF701: //NSDownArrowFunctionKey
			OSXProcessKeyboardMessage(&Controller->ActionDown, IsDown);
			break;

		case 0xF703: //NSRightArrowFunctionKey
			OSXProcessKeyboardMessage(&Controller->ActionRight, IsDown);
			break;

#if HANDMADE_INTERNAL
		case 'p':
			if (IsDown)
			{
				OSXToggleGlobalPause();
			}
			break;

		case 'l':
#if 1
			if (IsDown)
			{
				osx_state* OSXState = &GlobalOSXState;

				if (CommandKeyFlag)
				{
					OSXBeginInputPlayback(OSXState, 1);
				}
				else
				{
					if (OSXState->InputPlayingIndex == 0)
					{
						if (OSXState->InputRecordingIndex == 0)
						{
							OSXBeginRecordingInput(OSXState, 1);
						}
						else
						{
							OSXEndRecordingInput(OSXState);
							OSXBeginInputPlayback(OSXState, 1);
						}
					}
					else
					{
						OSXEndInputPlayback(OSXState);
					}
				}
			}
#endif
			break;
#endif
		default:
			return;
			break;
	}
}

void OSXProcessPendingMessages(GameInput* input)
{
	NSEvent* event;

	do {
		event = [NSApp nextEventMatchingMask:NSAnyEventMask
			untilDate:nil
			inMode:NSDefaultRunLoopMode
			dequeue:YES];

		switch ([event type]) {
			case NSKeyDown: {
				unichar c = [[event charactersIgnoringModifiers] characterAtIndex:0];
				int modifierFlags = [event modifierFlags];
				int commandKeyFlag = ModifierFlags & NSCommandKeyMask;
				int ctrlKeyFlag = ModifierFlags & NSControlKeyMask;
				int altKeyFlag = ModifierFlags & NSAlternateKeyMask;

				int KeyDownFlag = 1;

				OSXKeyProcessing(KeyDownFlag, C,
					commandKeyFlag, ctrlKeyFlag, altKeyFlag,
					input);
			} break;

			case NSKeyUp: {
				unichar c = [[event charactersIgnoringModifiers] characterAtIndex:0];
				int modifierFlags = [event modifierFlags];
				int commandKeyFlag = ModifierFlags & NSCommandKeyMask;
				int ctrlKeyFlag = ModifierFlags & NSControlKeyMask;
				int altKeyFlag = ModifierFlags & NSAlternateKeyMask;

				int KeyDownFlag = 0;

				OSXKeyProcessing(KeyDownFlag, C,
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

	OSXCreateMainMenu();

	NSString *dir = [[NSFileManager defaultManager] currentDirectoryPath];
	NSLog(@"Path to executable: %@", dir);

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

    NSOpenGLPixelFormatAttribute openGLAttributes[] =
    {
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


	///////////////////////////////////////////////////////////////////
	// Run loop
	//
	//uint tickCounter = 0;
	//uint64 currentTime = mach_absolute_time();
	//GameData.LastCounter = currentTime;
	//float32 frameTime = 0.0f;
	running_ = true;

	while (running_)
	{
		//OSXProcessPendingMessages(&GameData);
		OSXProcessPendingMessages();

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

		if (mouseInWindowFlag)
		{
			// NOTE(jeff): Use this instead of convertRectFromScreen: if you want to support Snow Leopard
			// NSPoint PointInWindow = [[self window] convertScreenToBase:[NSEvent mouseLocation]];

			// We don't actually care what the mouse screen coordinates are, we just want the
			// coordinates relative to the content view
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
		glClear(GL_COLOR_BUFFER_BIT);
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