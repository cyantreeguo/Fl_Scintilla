#include "fltk_config.h"

#if __FLTK_IPHONEOS__

#include "Fl.H"
#include "x.H"
#include "Fl_Window.H"
#include "Fl_Tooltip.H"
#include "Fl_Printer.H"
#include "Fl_Input_.H"
#include "Fl_Text_Display.H"
#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"
#include <unistd.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>

//#include <sys/time.h>

#include "Fl_Device.H"

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

// ======================================================

static unsigned make_current_counts = 0; // if > 0, then Fl_Window::make_current() can be called only once
static Fl_X *fl_x_to_redraw = NULL;
static BOOL through_drawRect = NO;

/*
static Fl_Quartz_Graphics_Driver fl_quartz_driver;
static Fl_Display_Device fl_quartz_display(&fl_quartz_driver);
Fl_Display_Device *Fl_Display_Device::_display = &fl_quartz_display; // the platform display
*/

// these pointers are set by the Fl::lock() function:
static void nothing() { }
void (*fl_lock_function)() = nothing;
void (*fl_unlock_function)() = nothing;

static int device_w, device_h, work_y=0;
static int real_device_w, real_device_h;

static void handleUpdateEvent(Fl_Window *window);

static NSDate *endDate = [NSDate dateWithTimeIntervalSinceNow:-0.001];

static Fl_Window *resize_from_system;

// ************************* main begin ****************************************
static int forward_argc;
static char **forward_argv;
static int exit_status;

static unsigned char EventPumpEnabled_ = 0;
static void SetEventPump(unsigned char enabled)
{
	EventPumpEnabled_ = enabled;
}

#define FLTK_min(x, y) (((x) < (y)) ? (x) : (y))
#define FLTK_max(x, y) (((x) > (y)) ? (x) : (y))
@interface FLTK_splashviewcontroller : UIViewController {
    UIImageView *splash;
    UIImage *splashPortrait;
    UIImage *splashLandscape;
}

- (void)updateSplashImage:(UIInterfaceOrientation)interfaceOrientation;
@end

@implementation FLTK_splashviewcontroller

- (id)init
{
    self = [super init];
    if (self == nil) {
        return nil;
    }
    
    [self setWantsFullScreenLayout:YES];

    self->splash = [[UIImageView alloc] init];
    [self setView:self->splash];

    CGSize size = [UIScreen mainScreen].bounds.size;
    float height = FLTK_max(size.width, size.height);
    self->splashPortrait = [UIImage imageNamed:[NSString stringWithFormat:@"Default-%dh.png", (int)height]];
    if (!self->splashPortrait) {
        self->splashPortrait = [UIImage imageNamed:@"Default.png"];
    }
    self->splashLandscape = [UIImage imageNamed:@"Default-Landscape.png"];
    if (!self->splashLandscape && self->splashPortrait) {
        self->splashLandscape = [[UIImage alloc] initWithCGImage: self->splashPortrait.CGImage
                                                           scale: 1.0
                                                     orientation: UIImageOrientationRight];
    }
    if (self->splashPortrait) {
        [self->splashPortrait retain];
    }
    if (self->splashLandscape) {
        [self->splashLandscape retain];
    }

    [self updateSplashImage:[[UIApplication sharedApplication] statusBarOrientation]];

    return self;
}

- (NSUInteger)supportedInterfaceOrientations
{
    NSUInteger orientationMask = UIInterfaceOrientationMaskAll;

    // Don't allow upside-down orientation on the phone, so answering calls is in the natural orientation
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        orientationMask &= ~UIInterfaceOrientationMaskPortraitUpsideDown;
    }
    
    return orientationMask;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)orient
{
    NSUInteger orientationMask = [self supportedInterfaceOrientations];
    return (orientationMask & (1 << orient));
}

- (void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration
{
    /*
    if ( interfaceOrientation == UIInterfaceOrientationLandscapeLeft || interfaceOrientation == UIInterfaceOrientationLandscapeRight ) {
        device_h = real_device_w; device_w = real_device_h;
    } else {
        device_h = real_device_h; device_w = real_device_w;
    }
    */
    
    [self updateSplashImage:interfaceOrientation];
}

- (void)updateSplashImage:(UIInterfaceOrientation)interfaceOrientation
{
    UIImage *image;

    if (UIInterfaceOrientationIsLandscape(interfaceOrientation)) {
        image = self->splashLandscape;
    } else {
        image = self->splashPortrait;
    }
    if (image)
    {
        splash.image = image;
    }
}

@end

static UIWindow *launch_window=nil;
@interface FLTKUIKitDelegate : NSObject<UIApplicationDelegate> {	
}
+ (NSString *)getAppDelegateClassName;

- (void) keyboardWillShow:(NSNotification *)notification;
- (void) keyboardWillHide:(NSNotification *)notification;
@end

@implementation FLTKUIKitDelegate

+ (NSString *)getAppDelegateClassName
{
    return @"FLTKUIKitDelegate";
}

- (id)init
{
    self = [super init];
    return self;
}

- (void)postFinishLaunch
{
	CGSize size = [UIScreen mainScreen].bounds.size;
    real_device_w = (int)size.width; real_device_h = (int)size.height;
    device_w = real_device_w; device_h = real_device_h;
    Fl_Window::DisplayOrientation ori = Fl_Window::getCurrentOrientation();
    if ( ori == Fl_Window::rotatedAntiClockwise || ori == Fl_Window::rotatedClockwise ) {
        device_h = real_device_w; device_w = real_device_h;
    }
    //printf("decive_h=%d\n", device_h);
    
    [[UIApplication sharedApplication] setStatusBarHidden:NO];
    //[[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleLightContent animated:NO];
    /*
    [launch_window.rootViewController.navigationController.navigationBar setOpaque:YES];
    [launch_window.rootViewController.navigationController.navigationBar setTranslucent:NO];
    [launch_window.rootViewController.tabBarController.tabBar setOpaque: YES];
    [launch_window.rootViewController.tabBarController.tabBar setTranslucent: NO];
     */
    
    CGRect bounds = [[UIScreen mainScreen] applicationFrame];
    work_y = bounds.origin.y;
    printf("work_y=%d\n", work_y);
	
    /* run the user's application, passing argc and argv */
    SetEventPump(1);
    exit_status = IOS_main(forward_argc, forward_argv);
    SetEventPump(0);
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    /* If we showed a splash image, clean it up */
    if (launch_window) {
        [launch_window release];
        launch_window = NULL;
    }

    /* exit, passing the return status from the user's application */
    /* We don't actually exit to support applications that do setup in
     * their main function and then allow the Cocoa event loop to run.
     */
    /* exit(exit_status); */
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    /* Keep the launch image up until we set a video mode */
    CGRect crect =[[UIScreen mainScreen] bounds];
	launch_window = [[UIWindow alloc] initWithFrame:crect];
    FLTK_splashviewcontroller *splashViewController = [[FLTK_splashviewcontroller alloc] init];
    launch_window.rootViewController = splashViewController;
    [launch_window addSubview:splashViewController.view];
    [launch_window makeKeyAndVisible];

    /* Set working directory to resource path */
    [[NSFileManager defaultManager] changeCurrentDirectoryPath: [[NSBundle mainBundle] resourcePath]];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];
    if ( SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(@"5.0") ) {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShow:) name:UIKeyboardWillChangeFrameNotification object:nil];
    }

    [self performSelector:@selector(postFinishLaunch) withObject:nil afterDelay:0.0];

    return YES;
}

- (void) keyboardWillShow:(NSNotification *)notification
{
    printf("keyboard show\n");
    
    /*
     Reduce the size of the text view so that it's not obscured by the keyboard.
     Animate the resize so that it's in sync with the appearance of the keyboard.
     */
    
    NSDictionary *userInfo = [notification userInfo];
    
    // Get the origin of the keyboard when it's displayed.
    NSValue* aValue = [userInfo objectForKey:UIKeyboardFrameEndUserInfoKey];
    
    // Get the top of the keyboard as the y coordinate of its origin in self's view's coordinate system. The bottom of the text view's frame should align with the top of the keyboard's final position.
    CGRect keyboardRect = [aValue CGRectValue];
    
    // Get the duration of the animation.
    NSValue *animationDurationValue = [userInfo objectForKey:UIKeyboardAnimationDurationUserInfoKey];
    NSTimeInterval animationDuration;
    [animationDurationValue getValue:&animationDuration];
    
    // Animate the resize of the text view's frame in sync with the keyboard's appearance.
    //[self moveInputBarWithKeyboardHeight:keyboardRect.size.height withDuration:animationDuration];
    
    printf("height=%d\n", (int)keyboardRect.size.height);
}

- (void) keyboardWillHide:(NSNotification *)notification
{
    printf("keyboard hide\n");
    
    NSDictionary* userInfo = [notification userInfo];
    
    /*
     Restore the size of the text view (fill self's view).
     Animate the resize so that it's in sync with the disappearance of the keyboard.
     */
    NSValue *animationDurationValue = [userInfo objectForKey:UIKeyboardAnimationDurationUserInfoKey];
    NSTimeInterval animationDuration;
    [animationDurationValue getValue:&animationDuration];
    
    //[self moveInputBarWithKeyboardHeight:0.0 withDuration:animationDuration];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	fl_lock_function();
	while (Fl_X::first) {
		Fl_X *x = Fl_X::first;
		Fl::handle(FL_CLOSE, x->w);
		Fl::do_widget_deletion();
		if (Fl_X::first == x) {
			// FLTK has not closed all windows, so we return to the main program now
			break;
		}
	}
	fl_unlock_function();
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
    //SDL_SendAppEvent(SDL_APP_LOWMEMORY);
	// Do something
}

// http://justcoding.iteye.com/blog/1473350
/*
首次运行：
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
- (void)applicationDidBecomeActive:(UIApplication *)application

首次关闭（home）：
- (void)applicationWillResignActive:(UIApplication *)application
- (void)applicationDidEnterBackground:(UIApplication *)application

再次运行：
- (void)applicationWillEnterForeground:(UIApplication *)application
- (void)applicationDidBecomeActive:(UIApplication *)application

再次关闭：
- (void)applicationWillResignActive:(UIApplication *)application
- (void)applicationDidEnterBackground:(UIApplication *)application
*/
- (void) applicationWillResignActive:(UIApplication*)application
{
	// FIXIT: send event
}

- (void) applicationDidEnterBackground:(UIApplication*)application
{
    /*
     FIXIT:
	fl_lock_function();
	Fl_X *x;
	for (x = Fl_X::first; x; x = x->next) {
		Fl_Window *window = x->w;
		if (!window->parent()) Fl::handle(FL_HIDE, window);
	}
	fl_unlock_function();
     */
}

- (void) applicationWillEnterForeground:(UIApplication*)application
{
    /*
     FIXIT:
	fl_lock_function();
	Fl_X *x;
	for (x = Fl_X::first; x; x = x->next) {
		Fl_Window *w = x->w;
		if (!w->parent()) {
			Fl::handle(FL_SHOW, w);
		}
	}
	fl_unlock_function();
     */
}

- (void) applicationDidBecomeActive:(UIApplication*)application
{
	// FIXIT: send event
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
/*
    NSURL *fileURL = [url filePathURL];
    if (fileURL != nil) {
        SDL_SendDropFile([[fileURL path] UTF8String]);
    } else {
        SDL_SendDropFile([[url absoluteString] UTF8String]);
    }
*/	
	// FIXIT:
	
    return YES;
}

@end

#ifdef main
#undef main
#endif
int main(int argc, char **argv)
{
    int i;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    /* store arguments */
    forward_argc = argc;
    forward_argv = (char **)malloc((argc+1) * sizeof(char *));
    for (i = 0; i < argc; i++) {
        forward_argv[i] = (char*)malloc( (strlen(argv[i])+1) * sizeof(char));
        strcpy(forward_argv[i], argv[i]);
    }
    forward_argv[i] = NULL;
    
    /* Give over control to run loop, FLTKUIKitDelegate will handle most things from here */
    UIApplicationMain(argc, argv, NULL, [FLTKUIKitDelegate getAppDelegateClassName]);

    /* free the memory we used to hold copies of argc and argv */
    for (i = 0; i < forward_argc; i++) {
        free(forward_argv[i]);
    }
    free(forward_argv);

    [pool release];
    return exit_status;
}
// ************************* main end ****************************************

// type:0-begin,1-move,2-end
static void iosMouseHandler(NSSet *touches, UIEvent *event, UIView *view, int type);

//==============================================================================
@interface FLView : UIView <UITextViewDelegate>
{
	BOOL in_key_event; // YES means keypress is being processed by handleEvent
	BOOL need_handle; // YES means Fl::handle(FL_KEYBOARD,) is needed after handleEvent processing
	NSInteger identifier;
	NSRange selectedRange;
	
@public
    UITextView* hiddenTextView;
	Fl_Window *flwindow;
}
- (FLView*) initWithFlWindow: (Fl_Window*)win contentRect: (CGRect) rect;
- (Fl_Window *)getFl_Window;
- (void) dealloc;

- (void) drawRect: (CGRect) r;

- (void) touchesBegan: (NSSet*) touches withEvent: (UIEvent*) event;
- (void) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) event;
- (void) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) event;
- (void) touchesCancelled: (NSSet*) touches withEvent: (UIEvent*) event;

- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;
- (BOOL) canBecomeFirstResponder;

+ (void)prepareEtext: (NSString *)aString;
+ (void)concatEtext: (NSString *)aString;
- (BOOL) textView: (UITextView*) textView shouldChangeTextInRange: (NSRange) range replacementText: (NSString*) text;

//- (void) keyboardWillShow:(NSNotification *)notification;
//- (void) keyboardWillHide:(NSNotification *)notification;
@end

//==============================================================================
@interface FLViewController : UIViewController
{
}
- (NSUInteger) supportedInterfaceOrientations;
- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation;
- (void) willRotateToInterfaceOrientation: (UIInterfaceOrientation) toInterfaceOrientation duration: (NSTimeInterval) duration;
- (void) didRotateFromInterfaceOrientation: (UIInterfaceOrientation) fromInterfaceOrientation;
- (BOOL)prefersStatusBarHidden;
@end

//==============================================================================
@interface FLWindow : UIWindow {
    Fl_Window *w;
}
- (FLWindow *)initWithFlWindow: (Fl_Window *)flw contentRect: (CGRect)rect;
- (Fl_Window *)getFl_Window;
- (void) becomeKeyWindow;
@end

static void iosMouseHandler(NSSet *touches, UIEvent *event, UIView *view, int type)
{
    static int keysym[] = { 0, FL_Button + 1, FL_Button + 3, FL_Button + 2 };
    static int px, py;
    static char suppressed = 0;
    
    fl_lock_function();
    
    Fl_Window *window = (Fl_Window *)[(FLWindow *)[view window] getFl_Window];
    if (!window->shown()) {
        fl_unlock_function();
        return;
    }
    
    Fl_Window *first = Fl::first_window();
    if (first != window && !(first->modal() || first->non_modal())) Fl::first_window(window);
    
    UITouch *touch = [touches anyObject];
    CGPoint pos = [touch locationInView:view];
    //pos.x = pos.x * view.contentScaleFactor;
    //pos.y = pos.y * view.contentScaleFactor;
    NSUInteger taps = [touch tapCount];
    //pos.y = window->h() - pos.y;
    NSInteger btn = 1;//[theEvent buttonNumber]  + 1;
    //NSUInteger mods = [theEvent modifierFlags];
    int sendEvent = 0;
    
    /*
     NSEventType etype = [theEvent type];
     if (etype == NSLeftMouseDown || etype == NSRightMouseDown || etype == NSOtherMouseDown) {
     if (btn == 1) Fl::e_state |= FL_BUTTON1;
     else if (btn == 3) Fl::e_state |= FL_BUTTON2;
     else if (btn == 2) Fl::e_state |= FL_BUTTON3;
     } else if (etype == NSLeftMouseUp || etype == NSRightMouseUp || etype == NSOtherMouseUp) {
     if (btn == 1) Fl::e_state &= ~FL_BUTTON1;
     else if (btn == 3) Fl::e_state &= ~FL_BUTTON2;
     else if (btn == 2) Fl::e_state &= ~FL_BUTTON3;
     }
     */
    if ( type == 0 ) Fl::e_state |= FL_BUTTON1;
    else if ( type == 2 ) Fl::e_state &= ~FL_BUTTON1;
    
    switch (type) {
        case 0:
            suppressed = 0;
            sendEvent = FL_PUSH;
            Fl::e_is_click = 1;
            px = (int)pos.x; py = (int)pos.y;
            if (taps > 1) Fl::e_clicks++;
            else Fl::e_clicks = 0;
            // fall through
        case 2:
            if (suppressed) {
                suppressed = 0;
                break;
            }
            //if (!window) break;
            if (!sendEvent) {
                sendEvent = FL_RELEASE;
            }
            Fl::e_keysym = keysym[btn];
            // fall through
        case 1:
            suppressed = 0;
            if (!sendEvent) {
                sendEvent = FL_MOVE;
            }
            // fall through
            /*
             case NSLeftMouseDragged:
             case NSRightMouseDragged:
             case NSOtherMouseDragged:
             */
        {
            if (suppressed) break;
            if (!sendEvent) {
                sendEvent = FL_MOVE; // Fl::handle will convert into FL_DRAG
                if (fabs(pos.x - px) > 5 || fabs(pos.y - py) > 5) Fl::e_is_click = 0;
            }
            //            mods_to_e_state(mods);
            
            //update_e_xy_and_e_xy_root([view window]);
            Fl::e_x = int(pos.x);
            Fl::e_y = int(pos.y);
            Fl::e_x_root = int(pos.x);
            Fl::e_y_root = int(pos.y);
            
            Fl::handle(sendEvent, window);
        }
            break;
        default:
            break;
    }
    
    fl_unlock_function();
    
    return;
}

static void cocoaMouseWheelHandler(NSSet *touches, UIEvent *event, UIView *view, int type)
{
    // Handle the new "MightyMouse" mouse wheel events. Please, someone explain
    // to me why Apple changed the API on this even though the current API
    // supports two wheels just fine. Matthias,
    fl_lock_function();
    
    Fl_Window *window = (Fl_Window *)[(FLWindow *)[view window] getFl_Window];
    if ( !window->shown() ) {
        fl_unlock_function();
        return;
    }
    Fl::first_window(window);
    
    
    UITouch *touch = [touches anyObject];
    CGPoint oldpos = [touch previousLocationInView:view];
    CGPoint pos = [touch locationInView:view];
    NSUInteger taps = [touch tapCount];
    
    // Under OSX, single mousewheel increments are 0.1,
    // so make sure they show up as at least 1..
    //
    float dx = pos.x - oldpos.x;
    //float dx = [theEvent deltaX];
    //if ( fabs(dx) < 1.0 ) dx = (dx > 0) ? 1.0 : -1.0;
    float dy = pos.y - oldpos.y;
    //float dy = [theEvent deltaY];
    //if ( fabs(dy) < 1.0 ) dy = (dy > 0) ? 1.0 : -1.0;
    /*
    //if ([theEvent deltaX] != 0) {
    if (dx != 0) {
        Fl::e_dx = (int)-dx;
        Fl::e_dy = 0;
        if ( Fl::e_dx) Fl::handle( FL_MOUSEWHEEL, window );
    //} else if ([theEvent deltaY] != 0) {
    } else*/ if (dy != 0) {
        Fl::e_dx = 0;
        Fl::e_dy = (int)-dy;
        if ( Fl::e_dy) Fl::handle( FL_MOUSEWHEEL, window );
    } else {
        fl_unlock_function();
        return;
    }
    
    fl_unlock_function();
    
    //  return noErr;
}

//******************* spot **********************************

// public variables
CGContextRef fl_gc = 0;
void *fl_capture = 0;           // (NSWindow*) we need this to compensate for a missing(?) mouse capture
bool fl_show_iconic;                    // true if called from iconize() - shows the next created window in collapsed state
Window fl_window;
Fl_Window *Fl_Window::current_;
Fl_Fontdesc *fl_fonts = Fl_X::calc_fl_fonts();

static Fl_Window *spot_win_=0;

void fl_reset_spot()
{
    if ( Fl_X::first == NULL ) return;
    FLView *view = (FLView*)[[Fl_X::first->xid rootViewController] view];
    [view->hiddenTextView resignFirstResponder];
    printf("reset_spot\n");
}

void fl_set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win)
{
    FLView *view = (FLView*)[[Fl_X::first->xid rootViewController] view];
    [view->hiddenTextView becomeFirstResponder];
    view->hiddenTextView.text = @"";
    view->hiddenTextView.text = @"1";
    
    spot_win_ = win;
    printf("set_spot\n");
}

void fl_set_status(int x, int y, int w, int h)
{
    //printf("set_status\n");
}

void fl_open_display()
{
	static char beenHereDoneThat = 0;

	if (beenHereDoneThat) return;
	beenHereDoneThat = 1;

	// FIXIT: do some init thing
}

// so a CGRect matches exactly what is denoted x,y,w,h for clipping purposes
CGRect fl_cgrectmake_cocoa(int x, int y, int w, int h)
{
	return CGRectMake(x, y, w > 0 ? w - 0.9 : 0, h > 0 ? h - 0.9 : 0);
}

double fl_ios_flush_and_wait(double time_to_wait)  //ok
{
	if ( 0 == EventPumpEnabled_ ) return 0.0;
	
    Fl::flush();
    
    //printf("start\n");
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:endDate];
    [pool release];
    //printf("runloop end\n");
    
    return 0.0;
}

void fl_clipboard_notify_change()
{
	// No need to do anything here...
}

/*
 * Check if there is actually a message pending
 */
int fl_ready() // ok
{
	return 1;
}

void Fl::enable_im()
{
}

void Fl::disable_im()
{
}

/*
 * smallest x coordinate in screen space of work area of menubar-containing display
 */
int Fl::x() // ok
{
	return 0;
}

/*
 * smallest y coordinate in screen space of work area of menubar-containing display
 */
int Fl::y() // ok
{
	return work_y;
}

/*
 * width of work area of menubar-containing display
 */
int Fl::w() // ok
{
	return device_w;
}

/*
 * height of work area of menubar-containing display
 */
int Fl::h() // ok
{
	return device_h-work_y;
}

// computes the work area of the nth screen (screen #0 has the menubar)
void Fl_X::screen_work_area(int &X, int &Y, int &W, int &H, int n) // ok
{
	X = 0;
	Y = 0;
	W = device_w;
	H = device_h;
}

/*
 * get the current mouse pointer world coordinates
 */
void Fl::get_mouse(int &x, int &y)
{
/*
	fl_open_display();
	NSPoint pt = [NSEvent mouseLocation];
	x = int(pt.x);
	y = int(main_screen_height - pt.y);
*/	
}


/*
 * Gets called when a window is created, resized, or deminiaturized
 */
static void handleUpdateEvent(Fl_Window *window)
{
	if (!window) return;
	Fl_X *i = Fl_X::i(window);
	i->wait_for_expose = 0;

	if (i->region) {
		XDestroyRegion(i->region);
		i->region = 0;
	}

	for (Fl_X *cx = i->xidChildren; cx; cx = cx->xidNext) {
		if (cx->region) {
			XDestroyRegion(cx->region);
			cx->region = 0;
		}
		cx->w->clear_damage(FL_DAMAGE_ALL);
        CGContextRef gc = (CGContextRef)UIGraphicsGetCurrentContext();//[[UIGraphicsPopContext currentContext] graphicsPort];
		CGContextSaveGState(gc); // save original context
		cx->flush();
		CGContextRestoreGState(gc); // restore original context
		cx->w->clear_damage();
	}
	window->clear_damage(FL_DAMAGE_ALL);
	i->flush();
	window->clear_damage();
}

void Fl_Window::fullscreen_x()
{
	_set_fullscreen();
	/* On OS X < 10.6, it is necessary to recreate the window. This is done with hide+show. */
	hide();
	show();
	Fl::handle(FL_FULLSCREEN, this);
}

void Fl_Window::fullscreen_off_x(int X, int Y, int W, int H)
{
	_clear_fullscreen();
	hide();
	resize(X, Y, W, H);
	show();
	Fl::handle(FL_FULLSCREEN, this);
}

/*
 * Initialize the given port for redraw and call the window's flush() to actually draw the content
 */
void Fl_X::flush()
{
    //*
    if (through_drawRect ) { //|| w->as_gl_window()) {
        make_current_counts = 1;
        w->flush();
        make_current_counts = 0;
        Fl_X::q_release_context();
        return;
    }
    //*/
    // have Cocoa immediately redraw the window's view
    FLView *view = (FLView *)[[fl_xid(w) rootViewController] view];
    fl_x_to_redraw = this;
    [view setNeedsDisplay];//: YES];
    // will send the drawRect: message to the window's view after having prepared the adequate NSGraphicsContext
    //[view displayIfNeededIgnoringOpacity];
    fl_x_to_redraw = NULL;
}


/*
 * go ahead, create that (sub)window
 */
void Fl_X::make(Fl_Window *w)
{
	if (w->parent()) {      // create a subwindow
		Fl_Group::current(0);
		// our subwindow needs this structure to know about its clipping.
		Fl_X *x = new Fl_X;
		x->subwindow = true;
		x->other_xid = 0;
		x->region = 0;
		x->subRegion = 0;
		x->gc = 0;          // stay 0 for Quickdraw; fill with CGContext for Quartz
		w->set_visible();
		Fl_Window *win = w->window();
		Fl_X *xo = Fl_X::i(win);
		if (xo) {
			x->xidNext = xo->xidChildren;
			x->xidChildren = 0L;
			xo->xidChildren = x;
			x->xid = win->i->xid;
			x->w = w; w->i = x;
			x->wait_for_expose = 0;
			{
				Fl_X *z = xo->next; // we don't want a subwindow in Fl_X::first
				xo->next = x;
				x->next = z;
			}
			int old_event = Fl::e_number;
			w->handle(Fl::e_number = FL_SHOW);
			Fl::e_number = old_event;
			w->redraw();      // force draw to happen
		}
		/*
		if (w->as_gl_window()) { // if creating a sub-GL-window
			while (win->window()) win = win->window();
			[Fl_X::i(win)->xid containsGLsubwindow: YES];
		}
		*/
	} else {            // create a desktop window
        //printf("make\n");
		Fl_Group::current(0);
		fl_open_display();

		if (w->non_modal() && Fl_X::first /*&& !fl_disable_transient_for*/) {
			// find some other window to be "transient for":
			Fl_Window *w = Fl_X::first->w;
			while (w->parent()) w = w->window(); // todo: this code does not make any sense! (w!=w??)
		}

		Fl_X *x = new Fl_X();
		x->subwindow = false;
		x->other_xid = 0; // room for doublebuffering image map. On OS X this is only used by overlay windows
		x->region = 0;
		x->subRegion = 0;
		x->xidChildren = 0;
		x->xidNext = 0;
		x->gc = 0;

		CGRect crect;
		if (w->fullscreen_active()) {
            [[UIApplication sharedApplication] setStatusBarHidden: YES];
            int sx, sy, sw, sh;
            Fl::screen_work_area(sx, sy, sw, sh);
			w->x(sx);
            w->y(sy);
			w->w(sw);
            w->h(sh);

			//w->resize(X, Y, W, H);
        } else {
            if (SYSTEM_VERSION_LESS_THAN(@"7.0")) {
                int y_ios6 = w->y();
                if (y_ios6 <= work_y) y_ios6 = 0;
                //else y_ios6 -= work_y;
                w->y(y_ios6);
            }
        }
        //printf("make(), x=%d, y=%d, w=%d, h=%d\n", w->x(), w->y(), w->w(), w->h());
		
        crect.origin.x = w->x();
        crect.origin.y = w->y();
		crect.size.width = w->w();
		crect.size.height = w->h();
		FLWindow *cw = [[FLWindow alloc] initWithFlWindow: w contentRect: crect];
        //if (w->fullscreen_active()) cw.windowLevel = UIWindowLevelAlert;
        cw.autoresizesSubviews = NO;
        cw.opaque = YES;
        cw.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent:0];
        
        crect.size.width = w->w();
        crect.size.height = w->h();
        crect.origin.x = 0.0;
        crect.origin.y = 0.0;
		FLView *myview = [[FLView alloc] initWithFlWindow: w contentRect: crect];
        myview.multipleTouchEnabled = YES;
        myview.opaque = YES;
        myview.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent:0];
		
		FLViewController* controller;
		controller = [[FLViewController alloc] init];
        controller.view = myview;

		x->xid = cw;
		x->w = w; w->i = x;
		x->wait_for_expose = 1;
		x->next = Fl_X::first;
		Fl_X::first = x;
        
        cw.rootViewController = controller;
		[cw addSubview: myview];
		//[myview release];
        
        /*
        if ([controller respondsToSelector:@selector(setNeedsStatusBarAppearanceUpdate)]) {
            // ios 7
            [controller prefersStatusBarHidden];
            [controller performSelector:@selector(setNeedsStatusBarAppearanceUpdate)];
        }
        */

		/*
		q_set_window_title(cw, w->label(), w->iconlabel());
		if (!w->force_position()) {
			if (w->modal()) {
				[cw center];
			} else if (w->non_modal()) {
				[cw center];
			} else {
				static NSPoint delta = NSZeroPoint;
				delta = [cw cascadeTopLeftFromPoint: delta];
			}
		}
		if (w->menu_window()) { // make menu windows slightly transparent
			[cw setAlphaValue: 0.97];
		}
		*/
		
		// Install DnD handlers
		//[myview registerForDraggedTypes: [NSArray arrayWithObjects: utf8_format,  NSFilenamesPboardType, nil]];
		/*
		if (!Fl_X::first->next) {
			// if this is the first window, we need to bring the application to the front
			[NSApp activateIgnoringOtherApps:YES];
		}
		*/

		if (w->size_range_set) w->size_range_();

		if (w->border() || (!w->modal() && !w->tooltip_window())) {
			Fl_Tooltip::enter(0);
		}

		if (w->modal()) Fl::modal_ = w;

		w->set_visible();
		if (w->border() || (!w->modal() && !w->tooltip_window())) Fl::handle(FL_FOCUS, w);
		Fl::first_window(w);
        [cw makeKeyAndVisible];
        
        /*
        if ( launch_window != nil ) {
            [[[launch_window rootViewController] view] release];
            [[launch_window rootViewController] release];
            [launch_window release];
            launch_window = nil;
            printf("release\n");
        }
        //*/

		int old_event = Fl::e_number;
		w->handle(Fl::e_number = FL_SHOW);
		Fl::e_number = old_event;

		// if (w->modal()) { Fl::modal_ = w; fl_fix_focus(); }
	}
}


/*
 * Tell the OS what window sizes we want to allow
 */
void Fl_Window::size_range_()
{
}


/*
 * returns pointer to the filename, or null if name ends with ':'
 */
const char* fl_filename_name(const char *name)
{
	const char *p, *q;
	if (!name) return (0);
	for (p = q = name; *p;) {
		if ((p[0] == ':') && (p[1] == ':')) {
			q = p + 2;
			p++;
		} else if (p[0] == '/') {
			q = p + 1;
		}
		p++;
	}
	return q;
}


/*
 * set the window title bar name
 */
void Fl_Window::label(const char *name, const char *mininame)
{
}


/*
 * make a window visible
 */
void Fl_Window::show()
{
	image(Fl::scheme_bg_);
	if (Fl::scheme_bg_) {
		labeltype(FL_NORMAL_LABEL);
		align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	} else {
		labeltype(FL_NO_LABEL);
	}
	Fl_Tooltip::exit(this);
	if (!shown()) {
		fl_open_display();
		//if (can_boxcheat(box())) fl_background_pixel = int(fl_xpixel(color()));
		Fl_X::make(this);
	} else {
		//printf("Fl_Window::show 2\n");
		// Once again, we would lose the capture if we activated the window.
		//if (IsIconic(i->xid)) OpenIcon(i->xid);
		//if (!fl_capture) BringWindowToTop(i->xid);
		//ShowWindow(i->xid,fl_capture?SW_SHOWNOACTIVATE:SW_RESTORE);
	}
}

/*
 * resize a window
 */
void Fl_Window::resize(int X, int Y, int W, int H)
{
    CGRect r = [[UIScreen mainScreen] bounds];
    //printf("resize: %d %d, %d %d\n", (int)r.size.width, (int)r.size.height, device_w, device_h);
    
	if ((!parent()) && shown()) {
        //size_range(W, H, W, H);
		//int bx, by, bt;
		//if (!this->border()) bt = 0;
		//else get_window_frame_sizes(bx, by, bt);
        x(X); y(Y);w(W);h(H);
        
        printf("resize(), x=%d, y=%d, w=%d, h=%d\n", X, Y, W, H);
		CGRect dim;
		dim.origin.x = 0;//X;
		dim.origin.y = 0;//Y;//main_screen_height - (Y + H);
		dim.size.width = W;
		dim.size.height = H;// + bt;
		//[i->xid frame: dim display: YES]; // calls windowDidResize
        i->xid.frame = dim;
        //[i->xid setNeedsDisplay];
        i->xid.rootViewController.view.frame = dim;
        [i->xid.rootViewController.view setNeedsDisplay];
		return;
	}
	resize_from_system = 0;
	//if (is_a_resize) {
		Fl_Group::resize(X, Y, W, H);
		if (shown()) {
			redraw();
		}
	//} else {
	//	x(X); y(Y);
	//}
}

// removes x,y,w,h rectangle from region r and returns result as a new Fl_Region
static Fl_Region MacRegionMinusRect(Fl_Region r, int x, int y, int w, int h)
{
    Fl_Region outr = (Fl_Region)malloc(sizeof(*outr));
    outr->rects = (CGRect *)malloc(4 * r->count * sizeof(CGRect));
    outr->count = 0;
    CGRect rect = fl_cgrectmake_cocoa(x, y, w, h);
    for (int i = 0; i < r->count; i++) {
        CGRect A = r->rects[i];
        CGRect test = CGRectIntersection(A, rect);
        if (CGRectIsEmpty(test)) {
            outr->rects[(outr->count)++] = A;
        } else {
            const CGFloat verylarge = 100000.;
            CGRect side = CGRectMake(0, 0, rect.origin.x, verylarge); // W side
            test = CGRectIntersection(A, side);
            if (!CGRectIsEmpty(test)) {
                outr->rects[(outr->count)++] = test;
            }
            side = CGRectMake(0, rect.origin.y + rect.size.height, verylarge, verylarge); // N side
            test = CGRectIntersection(A, side);
            if (!CGRectIsEmpty(test)) {
                outr->rects[(outr->count)++] = test;
            }
            side = CGRectMake(rect.origin.x + rect.size.width, 0, verylarge, verylarge); // E side
            test = CGRectIntersection(A, side);
            if (!CGRectIsEmpty(test)) {
                outr->rects[(outr->count)++] = test;
            }
            side = CGRectMake(0, 0, verylarge, rect.origin.y); // S side
            test = CGRectIntersection(A, side);
            if (!CGRectIsEmpty(test)) {
                outr->rects[(outr->count)++] = test;
            }
        }
    }
    if (outr->count == 0) {
        free(outr->rects);
        free(outr);
        outr = XRectangleRegion(0, 0, 0, 0);
    } else outr->rects = (CGRect *)realloc(outr->rects, outr->count * sizeof(CGRect));
    return outr;
}

void Fl_Window::make_current()
{
    if (make_current_counts > 1) return;
    if (make_current_counts) make_current_counts++;
    Fl_X::q_release_context();
    fl_window = i->xid;
    current_ = this;
    
    int xp = 0, yp = 0;
    Fl_Window *win = this;
    while (win) {
        if (!win->window()) break;
        xp += win->x();
        yp += win->y();
        win = (Fl_Window *)win->window();
    }
    i->gc = (CGContextRef)UIGraphicsGetCurrentContext();
    fl_gc = i->gc;
    Fl_Region fl_window_region = XRectangleRegion(0, 0, w(), h());
    if (!this->window()) {
        for (Fl_X *cx = i->xidChildren; cx; cx = cx->xidNext) {   // clip-out all sub-windows
            Fl_Window *cw = cx->w;
            Fl_Region from = fl_window_region;
            fl_window_region = MacRegionMinusRect(from, cw->x(), cw->y(), cw->w(), cw->h());
            XDestroyRegion(from);
        }
    }
    
    // antialiasing must be deactivated because it applies to rectangles too
    // and escapes even clipping!!!
    // it gets activated when needed (e.g., draw text)
    CGContextSetShouldAntialias(fl_gc, false);
    //CGFloat hgt = [[[fl_window rootViewController] view] frame].size.height;
    //CGContextTranslateCTM(fl_gc, 0.5, hgt - 0.5f);
    CGContextTranslateCTM(fl_gc, 0.5, 0.5f);
    //CGContextScaleCTM(fl_gc, 1.0f, -1.0f); // now 0,0 is top-left point of the window
    win = this;
    while (win && win->window()) { // translate to subwindow origin if this is a subwindow context
        CGContextTranslateCTM(fl_gc, win->x(), win->y());
        win = win->window();
    }
    //apply window's clip
    CGContextClipToRects(fl_gc, fl_window_region->rects, fl_window_region->count);
    XDestroyRegion(fl_window_region);
    // this is the context with origin at top left of (sub)window clipped out of its subwindows if any
    CGContextSaveGState(fl_gc);
}

// helper function to manage the current CGContext fl_gc
extern void fl_quartz_restore_line_style_();

// FLTK has only one global graphics state. This function copies the FLTK state into the
// current Quartz context
void Fl_X::q_fill_context()
{
    if (!fl_gc) return;
    if (!fl_window) { // a bitmap context
        size_t hgt = CGBitmapContextGetHeight(fl_gc);
        CGContextTranslateCTM(fl_gc, 0.5, hgt - 0.5f);
        CGContextScaleCTM(fl_gc, 1.0f, -1.0f); // now 0,0 is top-left point of the context
    }
    fl_color(fl_graphics_driver->color());
    fl_quartz_restore_line_style_();
}

// The only way to reset clipping to its original state is to pop the current graphics
// state and restore the global state.
void Fl_X::q_clear_clipping()
{
    if (!fl_gc) return;
    CGContextRestoreGState(fl_gc);
    CGContextSaveGState(fl_gc);
}

// Give the Quartz context back to the system
void Fl_X::q_release_context(Fl_X *x)
{
    if (x && x->gc != fl_gc) return;
    if (!fl_gc) return;
    CGContextRestoreGState(fl_gc); // KEEP IT: matches the CGContextSaveGState of make_current
    CGContextFlush(fl_gc);
    fl_gc = 0;
}

void Fl_X::q_begin_image(CGRect &rect, int cx, int cy, int w, int h)
{
    CGContextSaveGState(fl_gc);
    CGRect r2 = rect;
    r2.origin.x -= 0.5f;
    r2.origin.y -= 0.5f;
    CGContextClipToRect(fl_gc, r2);
    // move graphics context to origin of vertically reversed image
    CGContextTranslateCTM(fl_gc, rect.origin.x - cx - 0.5, rect.origin.y - cy + h - 0.5);
    CGContextScaleCTM(fl_gc, 1, -1);
    rect.origin.x = rect.origin.y = 0;
    rect.size.width = w;
    rect.size.height = h;
}

void Fl_X::q_end_image()
{
    CGContextRestoreGState(fl_gc);
}

////////////////////////////////////////////////////////////////
// Copy & Paste fltk implementation.
////////////////////////////////////////////////////////////////
/*
static void convert_crlf(char *s, size_t len)
{
    // turn all \r characters into \n:
    for (size_t x = 0; x < len; x++) if (s[x] == '\r') s[x] = '\n';
}

// fltk 1.3 clipboard support constant definitions:
static NSString* calc_utf8_format(void)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_6
#define NSPasteboardTypeString @"public.utf8-plain-text"
#endif
    if (fl_mac_os_version >= 100600) return NSPasteboardTypeString;
    return NSStringPboardType;
}

// clipboard variables definitions :
char *fl_selection_buffer[2] = { NULL, NULL };
int fl_selection_length[2] = { 0, 0 };
static int fl_selection_buffer_length[2];

static PasteboardRef allocatePasteboard(void)
{
    PasteboardRef clip;
    PasteboardCreate(kPasteboardClipboard, &clip); // requires Mac OS 10.3
    return clip;
}
static PasteboardRef myPasteboard = allocatePasteboard();

extern void fl_trigger_clipboard_notify(int source);

void fl_clipboard_notify_change()
{
    // No need to do anything here...
}

static void clipboard_check(void)
{
    PasteboardSyncFlags flags;
    
    flags = PasteboardSynchronize(myPasteboard); // requires Mac OS 10.3
    
    if (!(flags & kPasteboardModified)) return;
    if (flags & kPasteboardClientIsOwner) return;
    
    fl_trigger_clipboard_notify(1);
}
*/

/*
 * create a selection
 * stuff: pointer to selected data
 * len: size of selected data
 * type: always "plain/text" for now
 */
void Fl::copy(const char *stuff, int len, int clipboard, const char *type)
{
    /*
    if (!stuff || len < 0) return;
    if (len + 1 > fl_selection_buffer_length[clipboard]) {
        delete[]fl_selection_buffer[clipboard];
        fl_selection_buffer[clipboard] = new char[len + 100];
        fl_selection_buffer_length[clipboard] = len + 100;
    }
    memcpy(fl_selection_buffer[clipboard], stuff, len);
    fl_selection_buffer[clipboard][len] = 0; // needed for direct paste
    fl_selection_length[clipboard] = len;
    if (clipboard) {
        CFDataRef text = CFDataCreate(kCFAllocatorDefault, (UInt8 *)fl_selection_buffer[1], len);
        if (text == NULL) return; // there was a pb creating the object, abort.
        NSPasteboard *clip = [NSPasteboard generalPasteboard];
        [clip declareTypes: [NSArray arrayWithObject: utf8_format] owner: nil];
        [clip setData: (NSData *)text forType: utf8_format];
        CFRelease(text);
    }
     */
}

// Call this when a "paste" operation happens:
void Fl::paste(Fl_Widget &receiver, int clipboard, const char *type)
{
}

int Fl::clipboard_contains(const char *type)
{
    printf("clipboard_contains\n");
	return 0;
}

int Fl_X::unlink(Fl_X *start)
{
    if (start) {
        Fl_X *pc = start;
        while (pc) {
            if (pc->xidNext == this) {
                pc->xidNext = xidNext;
                return 1;
            }
            if (pc->xidChildren) {
                if (pc->xidChildren == this) {
                    pc->xidChildren = xidNext;
                    return 1;
                }
                if (unlink(pc->xidChildren)) return 1;
            }
            pc = pc->xidNext;
        }
    } else {
        for (Fl_X *pc = Fl_X::first; pc; pc = pc->next) {
            if (unlink(pc)) return 1;
        }
    }
    return 0;
}

void Fl_X::relink(Fl_Window *w, Fl_Window *wp)
{
    Fl_X *x = Fl_X::i(w);
    Fl_X *p = Fl_X::i(wp);
    if (!x || !p) return;
    // first, check if 'x' is already registered as a child of 'p'
    for (Fl_X *i = p->xidChildren; i; i = i->xidNext) {
        if (i == x) return;
    }
    // now add 'x' as the first child of 'p'
    x->xidNext = p->xidChildren;
    p->xidChildren = x;
}

void Fl_X::destroy()
{
    // subwindows share their xid with their parent window, so should not close it
    if (!subwindow && w && !w->parent() && xid) {
        [xid resignKeyWindow];
        [xid release];
    }
}

void Fl_X::map()
{
    if (w && xid) {
        [xid orderFront: nil];
    }
    //+ link to window list
    if (w && w->parent()) {
        Fl_X::relink(w, w->window());
        w->redraw();
    }
}

void Fl_X::unmap()
{
    if (w && !w->parent() && xid) {
        [xid orderOut: nil];
    }
    if (w && Fl_X::i(w)) Fl_X::i(w)->unlink();
}

// intersects current and x,y,w,h rectangle and returns result as a new Fl_Region
Fl_Region Fl_X::intersect_region_and_rect(Fl_Region current, int x, int y, int w, int h)
{
    if (current == NULL) return XRectangleRegion(x, y, w, h);
    CGRect r = fl_cgrectmake_cocoa(x, y, w, h);
    Fl_Region outr = (Fl_Region)malloc(sizeof(*outr));
    outr->count = current->count;
    outr->rects = (CGRect *)malloc(outr->count * sizeof(CGRect));
    int j = 0;
    for (int i = 0; i < current->count; i++) {
        CGRect test = CGRectIntersection(current->rects[i], r);
        if (!CGRectIsEmpty(test)) outr->rects[j++] = test;
    }
    if (j) {
        outr->count = j;
        outr->rects = (CGRect *)realloc(outr->rects, outr->count * sizeof(CGRect));
    } else {
        XDestroyRegion(outr);
        outr = XRectangleRegion(0, 0, 0, 0);
    }
    return outr;
}

void Fl_X::collapse()
{
}

CFDataRef Fl_X::CGBitmapContextToTIFF(CGContextRef c)
{ 
	return (CFDataRef)0;
}

void Fl_X::set_key_window()
{
}

int Fl::dnd(void)
{
	return true;
}

/*
static NSBitmapImageRep* rect_to_NSBitmapImageRep(Fl_Window *win, int x, int y, int w, int h)
// the returned value is autoreleased
{
    CGRect rect;
    UIView *winview = nil;
    while (win->window()) {
        x += win->x();
        y += win->y();
        win = win->window();
    }
    if (through_drawRect) {
        CGFloat epsilon = 0;
        //if (fl_mac_os_version >= 100600) epsilon = 0.5; // STR #2887
        //rect = NSMakeRect(x - epsilon, y - epsilon, w, h);
        epsilon = 0.5;
    } else {
        rect = NSMakeRect(x, win->h() - (y + h), w, h);
        // lock focus to win's view
        winview = [fl_xid(win) contentView];
        [winview lockFocus];
    }
    NSBitmapImageRep *bitmap = [[[NSBitmapImageRep alloc] initWithFocusedViewRect: rect] autorelease];
    if (!through_drawRect) [winview unlockFocus];
    return bitmap;
}
 */

unsigned char* Fl_X::bitmap_from_window_rect(Fl_Window *win, int x, int y, int w, int h, int *bytesPerPixel)
/* Returns a capture of a rectangle of a mapped window as a pre-multiplied RGBA array of bytes.
 Alpha values are always 1 (except for the angles of a window title bar)
 so pre-multiplication can be ignored. 
 *bytesPerPixel is always set to the value 4 upon return.
 delete[] the returned pointer after use
 */
{
    /*
    NSBitmapImageRep *bitmap = rect_to_NSBitmapImageRep(win, x, y, w, h);
    if (bitmap == nil) return NULL;
    *bytesPerPixel = [bitmap bitsPerPixel] / 8;
    int bpp = (int)[bitmap bytesPerPlane];
    int bpr = (int)[bitmap bytesPerRow];
    int hh = bpp / bpr; // sometimes hh = h-1 for unclear reason
    int ww = bpr / (*bytesPerPixel); // sometimes ww = w-1
    unsigned char *data = new unsigned char[w * h *  *bytesPerPixel];
    if (w == ww) {
        memcpy(data, [bitmap bitmapData], w * hh *  *bytesPerPixel);
    } else {
        unsigned char *p = [bitmap bitmapData];
        unsigned char *q = data;
        for (int i = 0; i < hh; i++) {
            memcpy(q, p, *bytesPerPixel * ww);
            p += bpr;
            q += w * *bytesPerPixel;
        }
    }
    return data;
     */
    return NULL;
}

CGImageRef Fl_X::CGImage_from_window_rect(Fl_Window *win, int x, int y, int w, int h)
// CFRelease the returned CGImageRef after use
{
	return 0;
}

Window fl_xid(const Fl_Window *w) //ok
{
	Fl_X *temp = Fl_X::i(w);
	return temp ? temp->xid : 0;
}

// no decorated border
int Fl_Window::decorated_w() //ok
{
    return w();
}

int Fl_Window::decorated_h() //ok
{
    return h();
}

// not implentment fd function in ios
void Fl::add_fd(int n, int events, void (*cb)(int, void *), void *v) //ok
{
}

void Fl::add_fd(int fd, void (*cb)(int, void *), void *v) //ok
{
}

void Fl::remove_fd(int n, int events) //ok
{
}

void Fl::remove_fd(int n) //ok
{
}

//==============================================================================
static Fl_Window::DisplayOrientation convertOrientation(UIInterfaceOrientation orientation)
{
	switch (orientation) {
	case UIInterfaceOrientationPortrait:            return Fl_Window::upright;
	case UIInterfaceOrientationPortraitUpsideDown:  return Fl_Window::upsideDown;
	case UIInterfaceOrientationLandscapeLeft:       return Fl_Window::rotatedClockwise;
	case UIInterfaceOrientationLandscapeRight:      return Fl_Window::rotatedAntiClockwise;
	default:                                        return Fl_Window::upright; // unknown orientation!
	}
	return Fl_Window::upright;
}

Fl_Window::DisplayOrientation Fl_Window::getCurrentOrientation()
{
	return convertOrientation([[UIApplication sharedApplication] statusBarOrientation]);
}

static NSUInteger getSupportedOrientations(Fl_Window *w)
{
    NSUInteger allowed = 0;

    if (w->isOrientationEnabled (Fl_Window::upright))              allowed |= UIInterfaceOrientationMaskPortrait;
    if (w->isOrientationEnabled (Fl_Window::upsideDown))           allowed |= UIInterfaceOrientationMaskPortraitUpsideDown;
    if (w->isOrientationEnabled (Fl_Window::rotatedClockwise))     allowed |= UIInterfaceOrientationMaskLandscapeLeft;
    if (w->isOrientationEnabled (Fl_Window::rotatedAntiClockwise)) allowed |= UIInterfaceOrientationMaskLandscapeRight;

    return allowed;
}

/*
static CGRect convertToCGRect (const RectType& r)
{
	return CGRectMake ((CGFloat) r.getX(), (CGFloat) r.getY(), (CGFloat) r.getWidth(), (CGFloat) r.getHeight());
}
*/

//==============================================================================
//========================== implementation ====================================
//==============================================================================
@implementation FLViewController

- (NSUInteger) supportedInterfaceOrientations
{
    //printf("supportedInterfaceOrientations\n");
	FLView *view = (FLView *)[self view];
	Fl_Window *w = [view getFl_Window];
    return getSupportedOrientations(w);
}

- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation
{
	FLView *view = (FLView *)[self view];
	Fl_Window *w = [view getFl_Window];
    return w->isOrientationEnabled (convertOrientation(interfaceOrientation));
}

- (void) willRotateToInterfaceOrientation: (UIInterfaceOrientation) toInterfaceOrientation duration: (NSTimeInterval) duration
{
    (void) toInterfaceOrientation;
    (void) duration;
    
    if ( SYSTEM_VERSION_LESS_THAN(@"8.0") ) {
        if ( toInterfaceOrientation == UIInterfaceOrientationLandscapeLeft || toInterfaceOrientation == UIInterfaceOrientationLandscapeRight ) {
            device_h = real_device_w; device_w = real_device_h;
        } else {
            device_h = real_device_h; device_w = real_device_w;
        }
        printf("w=%d, h=%d\n", device_w, device_h);
        FLView *view = (FLView *)[self view];
        Fl_Window *w = [view getFl_Window];
        w->resize(0, 0, device_w, device_h);
    }
    
    [UIView setAnimationsEnabled: NO]; // disable this because it goes the wrong way and looks like crap.
}

- (void) didRotateFromInterfaceOrientation: (UIInterfaceOrientation) fromInterfaceOrientation
{
    (void) fromInterfaceOrientation;

	//FLView *view = (FLView *)[self view];
	//Fl_Window *w = [view getFl_Window];
	/*
    JuceUIView* juceView = (JuceUIView*) [self view];
    jassert (juceView != nil && juceView->owner != nullptr);
    juceView->owner->updateTransformAndScreenBounds();
	*/
    
    if ( SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(@"8.0") ) {
    if ( fromInterfaceOrientation == UIInterfaceOrientationLandscapeLeft || fromInterfaceOrientation == UIInterfaceOrientationLandscapeRight ) {
        device_h = real_device_h; device_w = real_device_w;
    } else {
        device_h = real_device_w; device_w = real_device_h;
    }
    printf("w=%d, h=%d\n", device_w, device_h);
    FLView *view = (FLView *)[self view];
    Fl_Window *w = [view getFl_Window];
    w->resize(0, 0, device_w, device_h);
    }
    
    [UIView setAnimationsEnabled: YES];
}

- (BOOL)prefersStatusBarHidden
{
    //printf("prefersStatusBarHidden\n");
    FLView *view = (FLView *)[self view];
	Fl_Window *w = [view getFl_Window];
    if ( w->fullscreen_active() ) return YES;
    else {
        //printf("no\n");
        return NO;
    }
}
@end

@implementation FLView

- (FLView*) initWithFlWindow: (Fl_Window*)win contentRect: (CGRect) rect;
{
    [super initWithFrame: rect];
	
	flwindow = win;
    in_key_event = NO;

    hiddenTextView = [[UITextView alloc] initWithFrame: CGRectZero];
    /*
    CGRect r;
    r.origin.x = 10; r.origin.y = 140; r.size.width = 120; r.size.height = 25;
    hiddenTextView = [[UITextView alloc] initWithFrame: r];
    //*/
    [self addSubview: hiddenTextView];
    hiddenTextView.delegate = self;

    hiddenTextView.autocapitalizationType = UITextAutocapitalizationTypeNone;
    hiddenTextView.autocorrectionType = UITextAutocorrectionTypeNo;
    hiddenTextView.keyboardType = UIKeyboardTypeDefault;
    hiddenTextView.text = @"1"; // for backspace keyboard
    
    //[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
    //[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShow) name:UIKeyboardWillShowNotification object:nil];
    
    return self;
}

- (Fl_Window *)getFl_Window
{
	return flwindow;
}

- (void) dealloc
{
    //[[NSNotificationCenter defaultCenter] removeObserver:self];
    
    [hiddenTextView removeFromSuperview];
    [hiddenTextView release];

    [super dealloc];
}

//==============================================================================
- (void) drawRect: (CGRect) r
{
    //if (owner != nullptr) owner->drawRect (r);
	
	fl_lock_function();
	through_drawRect = YES;
	if (fl_x_to_redraw) fl_x_to_redraw->flush();
	else handleUpdateEvent(flwindow);
	through_drawRect = NO;
	fl_unlock_function();
}

//==============================================================================
- (void) touchesBegan: (NSSet*) touches withEvent: (UIEvent*) event
{
    (void) touches;

    //if (owner != nullptr)
    //    owner->handleTouches (event, true, false, false);
    iosMouseHandler(touches, event, self, 0);
}

- (void) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) event
{
    (void) touches;

    //if (owner != nullptr)
    //    owner->handleTouches (event, false, false, false);
    iosMouseHandler(touches, event, self, 1);
    //cocoaMouseWheelHandler(touches, event, self, 1);
    printf("move\n");
}

- (void) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) event
{
    (void) touches;

    //if (owner != nullptr)
    //    owner->handleTouches (event, false, true, false);
    iosMouseHandler(touches, event, self, 2);
}

- (void) touchesCancelled: (NSSet*) touches withEvent: (UIEvent*) event
{
    //if (owner != nullptr)
    //    owner->handleTouches (event, false, true, true);

    [self touchesEnded: touches withEvent: event];
}

//==============================================================================
- (BOOL) becomeFirstResponder
{
    //if (owner != nullptr)
    //    owner->viewFocusGain();

    return true;
}

- (BOOL) resignFirstResponder
{
    //if (owner != nullptr)
    //    owner->viewFocusLoss();

    return true;
}

- (BOOL) canBecomeFirstResponder
{
    if ( Fl::modal_ && (Fl::modal_ != flwindow) ) return NO;
    return !(flwindow->tooltip_window() || flwindow->menu_window());
}

+ (void)prepareEtext: (NSString *)aString
{
    // fills Fl::e_text with UTF-8 encoded aString using an adequate memory allocation
    static char *received_utf8 = NULL;
    static int lreceived = 0;
    char *p = (char *)[aString UTF8String];
    int l = (int)strlen(p);
    if (l > 0) {
        if (lreceived == 0) {
            received_utf8 = (char *)malloc(l + 1);
            lreceived = l;
        } else if (l > lreceived) {
            received_utf8 = (char *)realloc(received_utf8, l + 1);
            lreceived = l;
        }
        strcpy(received_utf8, p);
        Fl::e_text = received_utf8;
    }
    Fl::e_length = l;
}

+ (void)concatEtext: (NSString *)aString
{
    // extends Fl::e_text with aString
    NSString *newstring = [[NSString stringWithUTF8String: Fl::e_text] stringByAppendingString: aString];
    [FLView prepareEtext: newstring];
}

- (BOOL) textView: (UITextView*) textView shouldChangeTextInRange: (NSRange) range replacementText: (NSString*) text
{
    (void) textView;
    
    /*
    const char *s1 = [text UTF8String];
    if ( range.length > 0 ) {
        if ( strlen(s1) == 0 ) printf("delete\n");
        else printf("input:%s\n", s1);
    } else {
        if ( strlen(s1) > 0 ) {
            printf("input:%s\n", s1);
        }
    }
    //*/
    
    // =============================
    NSString *received;
    if ([text isKindOfClass: [NSAttributedString class]]) {
        received = [(NSAttributedString *)text string];
    } else {
        received = (NSString *)text;
    }
    const char *s = [text UTF8String];
    fl_lock_function();
    Fl_Window *target = flwindow;// [(FLWindow *)[self window] getFl_Window];
    if ( range.length > 0 ) {
        if ( strlen(s) == 0 ) {
            int saved_keysym = Fl::e_keysym;
            Fl::e_keysym = FL_BackSpace;
            Fl::handle(FL_KEYBOARD, target);
            Fl::e_keysym = saved_keysym;
        } else {
            if ( 0 == strcmp(s, "\n") || 0 == strcmp(s, "\r") || 0 == strcmp(s, "\r\n") ) {
                Fl::e_keysym = FL_Enter;
                Fl::handle(FL_KEYBOARD, target);
                Fl::e_length = 0;
                if ( spot_win_ != 0 ) {
                    if ( spot_win_->visible_focus() ) {
                        fl_set_spot(0, 0, 0, 0, 0, 0, spot_win_);
                        spot_win_ = 0;
                    }
                }
                fl_unlock_function();
                return NO;
            }
        }
    } else {
        if ( strlen(s) == 0 ) {
            fl_unlock_function();
            return NO;
        } else {
            if ( 0 == strcmp(s, "\n") || 0 == strcmp(s, "\r") || 0 == strcmp(s, "\r\n") ) {
                Fl::e_keysym = FL_Enter;
                Fl::handle(FL_KEYBOARD, target);
                Fl::e_length = 0;
                if ( spot_win_ != 0 ) {
                    if ( spot_win_->visible_focus() ) {
                        fl_set_spot(0, 0, 0, 0, 0, 0, spot_win_);
                        spot_win_ = 0;
                    }
                }
                fl_unlock_function();
                return NO;
            }
        }
    }

    if (in_key_event && Fl_X::next_marked_length && Fl::e_length) {
        // if setMarkedText + insertText is sent during handleEvent, text cannot be concatenated in single FL_KEYBOARD event
        Fl::handle(FL_KEYBOARD, target);
        Fl::e_length = 0;
    }
    if (in_key_event && Fl::e_length) [FLView concatEtext: received];
    else [FLView prepareEtext: received];
    Fl_X::next_marked_length = 0;
    // We can get called outside of key events (e.g., from the character palette, from CJK text input).
    BOOL palette = !(in_key_event || Fl::compose_state);
    if (palette) Fl::e_keysym = 0;
    //printf("e_keysym=%x\n", Fl::e_keysym);
    // YES if key has text attached
    BOOL has_text_key = Fl::e_keysym <= '~' || Fl::e_keysym == FL_Iso_Key || (Fl::e_keysym >= FL_KP && Fl::e_keysym <= FL_KP_Last && Fl::e_keysym != FL_KP_Enter);
    // insertText sent during handleEvent of a key without text cannot be processed in a single FL_KEYBOARD event.
    // Occurs with deadkey followed by non-text key
    if (!in_key_event || !has_text_key) {
        Fl::handle(FL_KEYBOARD, target);
        Fl::e_length = 0;
    } else need_handle = YES;
    selectedRange = NSMakeRange(100, 0); // 100 is an arbitrary value
    // for some reason, with the palette, the window does not redraw until the next mouse move or button push
    // sending a 'redraw()' or 'awake()' does not solve the issue!
    if (palette) Fl::flush();
    //if (fl_mac_os_version < 100600) [(FLTextView *)[[self window] fieldEditor: YES forObject: nil] setActive: NO];
    fl_unlock_function();

    //if (range.length == 0 ) return YES;
    return NO;
}

/*
- (void) keyboardWillShow:(NSNotification *)notification
{
    printf("keyboard show\n");
}

- (void) keyboardWillHide:(NSNotification *)notification
{
    printf("keyboard hide\n");
}
*/

@end

//==============================================================================
@implementation FLWindow
- (FLWindow *)initWithFlWindow: (Fl_Window *)flw contentRect: (CGRect)rect
{
    self = [super initWithFrame: rect];
    if (self) w = flw;
    return self;
}
- (Fl_Window *)getFl_Window
{
    return w;
}

- (void) becomeKeyWindow;
{
	[super becomeKeyWindow];
	
	[self makeKeyWindow];
	
	// FIXIT: save focus current uiwindow?
}
@end

#endif // __FLTK_IPHONEOS__
