#include "Fl_Platform.h"

#if __FLTK_WIN32__
#include "fltk_config_win32.h"
#elif __FLTK_IPHONEOS__
#include "fltk_config_ios.h"
#elif __FLTK_MACOSX__
#include "fltk_config_osx.h"
#elif __FLTK_LINUX__
#include "fltk_config_linux.h"
#elif __FLTK_WINCE__
#include "fltk_config_wince.h"
#elif __FLTK_S60v32__
#include "fltk_config_s60v32.h"
#elif __FLTK_ANDROID__
#include "fltk_config_android.h"
#else
#error unsupported platform
#endif
