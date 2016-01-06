// FLTK Symbian port Copyright 2009 by Sadysta.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.

#include "Fl_Widget.H"
#include "Fl_Window.H"
#include "Fl_Group.H"
#include "Fl.H"
#include "x.H"
#include "fl_draw.H"

/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/
#include <string.h>
#include <jni.h>
#include <android/log.h>

extern int main(int, char *[]);

static int screenWidth_=0;
static int screenHeight_ = 0;
static int dpi_ = 0;

JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_launchAppNative(JNIEnv *env, jobject jcls, jstring s1, jstring s2) {}

/*
 * Class:     org_fltk_android_FLTKActivity
 * Method:    onDestroy_native
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_onDestroyNative(JNIEnv *env, jobject jcls){}

/*
 * Class:     org_fltk_android_FLTKActivity
 * Method:    onPause_native
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_onPauseNative(JNIEnv *env, jobject jcls){}

/*
 * Class:     org_fltk_android_FLTKActivity
 * Method:    onResume_native
 * Signature: ()V
 */
//JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_onResume_native(JNIEnv *env, jobject jcls)
JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_onResumeNative(JNIEnv *env, jobject jcls)
{
  __android_log_print(ANDROID_LOG_INFO, "FLTK", "On Resume Native");
}

/*
 * Class:     org_fltk_android_FLTKActivity
 * Method:    setScreenSize_native
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_setScreenSizeNative(JNIEnv* env, jobject jcls, jint screenWidth, jint screenHeight, jint dpi)
{
  screenWidth_ = screenWidth;
  screenHeight_ = screenHeight;
  dpi_ = dpi;

  __android_log_print(ANDROID_LOG_INFO, "FLTK", "width:%d, height:%d, dpi:%d", screenWidth_, screenHeight_, dpi_);
}

/*
 * Class:     org_fltk_android_FLTKActivity
 * Method:    deliverMessage
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_deliverMessage(JNIEnv *env, jobject jcls, jlong i){}

/*
 * Class:     org_fltk_android_FLTKActivity
 * Method:    alertDismissed
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_alertDismissed(JNIEnv *env, jobject jcls, jlong a1, jint a2){}

///////////////

/*
 * Class:     org_fltk_android_FLTKActivity_ComponentPeerView
 * Method:    handleMouseDown
 * Signature: (JIFFJ)V
 */
JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_00024ComponentPeerView_handleMouseDown
  (JNIEnv *env, jobject jcls, jlong a1, jint a2, jfloat a3, jfloat a4, jlong a5){}

/*
 * Class:     org_fltk_android_FLTKActivity_ComponentPeerView
 * Method:    handleMouseDrag
 * Signature: (JIFFJ)V
 */
//JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_00024ComponentPeerView_handleMouseDrag(JNIEnv *, jobject, jlong, jint, jfloat, jfloat, jlong){}

/*
 * Class:     org_fltk_android_FLTKActivity_ComponentPeerView
 * Method:    handleMouseUp
 * Signature: (JIFFJ)V
 */
//JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_00024ComponentPeerView_handleMouseUp(JNIEnv *, jobject, jlong, jint, jfloat, jfloat, jlong){}

/*
 * Class:     org_fltk_android_FLTKActivity_ComponentPeerView
 * Method:    handleKeyDown
 * Signature: (JII)V
 */
//JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_00024ComponentPeerView_handleKeyDown(JNIEnv *, jobject, jlong, jint, jint){}

/*
 * Class:     org_fltk_android_FLTKActivity_ComponentPeerView
 * Method:    handleKeyUp
 * Signature: (JII)V
 */
//JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_00024ComponentPeerView_handleKeyUp(JNIEnv *, jobject, jlong, jint, jint){}

/*
 * Class:     org_fltk_android_FLTKActivity_ComponentPeerView
 * Method:    viewSizeChanged
 * Signature: (J)V
 */
//JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_00024ComponentPeerView_viewSizeChanged(JNIEnv *, jobject, jlong){}

/*
 * Class:     org_fltk_android_FLTKActivity_ComponentPeerView
 * Method:    focusChanged
 * Signature: (JZ)V
 */
//JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_00024ComponentPeerView_focusChanged(JNIEnv *, jobject, jlong, jboolean){}


/*
 * Class:     org_fltk_android_FLTKActivity_OpenGLView
 * Method:    contextCreated
 * Signature: ()V
 */
//JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_00024OpenGLView_contextCreated(JNIEnv *, jobject){}

/*
 * Class:     org_fltk_android_FLTKActivity_OpenGLView
 * Method:    contextChangedSize
 * Signature: ()V
 */
//JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_00024OpenGLView_contextChangedSize(JNIEnv *, jobject){}

/*
 * Class:     org_fltk_android_FLTKActivity_OpenGLView
 * Method:    render
 * Signature: ()V
 */
//JNIEXPORT void JNICALL Java_org_fltk_android_FLTKActivity_00024OpenGLView_render(JNIEnv *, jobject){}

//JNIEXPORT int JNICALL Java_org_fltk_android_FLTKActivity_nativeInit(JNIEnv* env, jclass cls, jobject array)
JNIEXPORT int JNICALL Java_org_fltk_android_FLTKActivity_nativeInit(JNIEnv* env, jclass cls, jobjectArray array)
{
  	int i;
    int argc;
    int status;

    /* This interface could expand with ABI negotiation, callbacks, etc. */
    //SDL_Android_Init(env, cls);

    //SDL_SetMainReady();

    /* Prepare the arguments. */

    //int len = (*env)->GetArrayLength(env, array);
    int len = env->GetArrayLength(array);
    char* argv[1 + len + 1];
    argc = 0;
    /* Use the name "app_process" so PHYSFS_platformCalcBaseDir() works.
       https://bitbucket.org/MartinFelis/love-android-sdl2/issue/23/release-build-crash-on-start
     */
    argv[argc++] = strdup("app_process");
    for (i = 0; i < len; ++i) {
        const char* utf;
        char* arg = NULL;
        //jstring string = (*env)->GetObjectArrayElement(env, array, i);
        jstring string = (jstring)env->GetObjectArrayElement(array, i);
        if (string) {
            //utf = (*env)->GetStringUTFChars(env, string, 0);
            utf = env->GetStringUTFChars(string, 0);
            if (utf) {
                arg = strdup(utf);
                //(*env)->ReleaseStringUTFChars(env, string, utf);
                env->ReleaseStringUTFChars(string, utf);
            }
            //(*env)->DeleteLocalRef(env, string);
            env->DeleteLocalRef(string);
        }
        if (!arg) {
            arg = strdup("");
        }
        argv[argc++] = arg;
    }
    argv[argc] = NULL;


    /* Run the application. */

    status = main(argc, argv);
    while(1);

    /* Release the arguments. */

    for (i = 0; i < argc; ++i) {
        free(argv[i]);
    }

    /* Do not issue an exit or the whole application will terminate instead of just the SDL thread */
    /* exit(status); */

    return status;
}

// *****************************************************
void fl_fix_focus();

Fl_X* Fl_X::make(Fl_Window *w)
{
	return 0;
}

void Fl::copy(const char *stuff, int len, int clipboard, const char *type)
{
}

void Fl::paste(Fl_Widget &receiver, int clipboard, const char *type)
{
}

Fl_Window* Fl_Window::current_;
Window fl_window;

void* fl_GetDC(Window w)
{
}

void fl_save_pen(void)
{
}

void fl_restore_pen(void)
{
}

void fl_reset_spot()
{
	// TODO: S60
}

void fl_set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win)
//void fl_set_spot(int font, int size, int x, int y, int w, int h)
{
	// TODO: S60
}

static void nothing() {}
void (*fl_lock_function)() = nothing;
void (*fl_unlock_function)() = nothing;

int fl_wait(double time_to_wait)
{
	return 0;
}

int fl_ready()
{
	// TODO: S60
}

void Fl_Window::make_current()
{
}

void Fl_Window::show()
{
	// DONE: S60
	if (!shown()) {
		//Fl_X::make(this);
		this->resize (0, 0, Fl::w(), Fl::h());
	} else {
		// DONE: S60, Bring it to top
		//fl_xid(this)->SetVisible(true);
		//fl_xid(this)->SetOrdinalPosition(0);
	}
}

void Fl_Window::resize(int x, int y, int w, int h)
{
	// TODO: S60
	// fl_xid(this)->SetExtent(TPoint(x, y), TSize(w, h));
	Fl_Group::resize (x, y, w, h);
}

void Fl_Window::size_range_()
{
	// TODO: S60
	size_range_set = 1;
}

void Fl_Window::label(char const *name, char const *iname)
{
	// TODO: S60
}

int Fl::x()
{
	// DONE: S60
	return 0;
}

int Fl::y()
{
	// DONE: S60
	return 0;
}

int Fl::w()
{
	return screenWidth_;
}

int Fl::h()
{
	return screenHeight_;
}

void Fl::get_mouse(int &x, int &y)
{
}

const char *fl_filename_name(const char *name)
{
	const char *p,*q;
	if (!name) return (0);
	q = name;
	if (q[0] && q[1]==':') q += 2; // skip leading drive letter
	for (p = q; *p; p++) if (*p == '/' || *p == '\\') q = p+1;
	return q;
}

char fl_show_iconic;

// Re-used from Win32 implemenatation
void Fl::add_fd(int n, int events, void (*cb)(int, void*), void *v)
{
}

void Fl::add_fd(int fd, void (*cb)(int, void*), void* v)
{
	//Fl::add_fd(fd, POLLIN, cb, v);
}

void Fl::remove_fd(int n, int events)
{
}

void Fl::remove_fd(int n)
{
	Fl::remove_fd(n, -1);
}

void Fl::add_timeout(double time, Fl_Timeout_Handler cb, void* data)
{
	Fl::repeat_timeout(time, cb, data);
}

void Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void* data)
{
}

int Fl::has_timeout(Fl_Timeout_Handler cb, void* data)
{
	return 0;
}

void Fl::remove_timeout(Fl_Timeout_Handler cb, void* data)
{
}

void fl_clipboard_notify_change()
{

}

void Fl_Window::fullscreen_x()
{
}

void Fl_Window::fullscreen_off_x(int X, int Y, int W, int H)
{
}