/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/**
 *  \file SDL_platform.h
 *
 *  Try to get a standard set of platform defines.
 */

// 2014-07-06 cyantree, modify by SDL
/*
__FLTK_WIN32__
__FLTK_LINUX__
__FLTK_MACOSX__
__FLTK_WINCE__
__FLTK_IPHONEOS__
__FLTK_ANDROID__
__FLTK_S60v32__ // symbian s60 v3 fp2

__FLTK_AIX__
__FLTK_HAIKU__
__FLTK_BSDI__
__FLTK_DREAMCAST__
__FLTK_FREEBSD__
__FLTK_HPUX__
__FLTK_IRIX__
__FLTK_NETBSD__
__FLTK_OPENBSD__
__FLTK_OS2__
__FLTK_OSF__
__FLTK_QNXNTO__
__FLTK_RISCOS__
__FLTK_SOLARIS__
__FLTK_WINRT__
__FLTK_PSP__

__FLTK_Dummy__ // test, for port fltk to other operating system
*/

#ifndef _FL_Platform_h
#define _FL_Platform_h

#if defined(_AIX)
#undef __FLTK_AIX__
#define __FLTK_AIX__     1
#endif
#if defined(__HAIKU__)
#undef __FLTK_HAIKU__
#define __FLTK_HAIKU__   1
#endif
#if defined(bsdi) || defined(__bsdi) || defined(__bsdi__)
#undef __FLTK_BSDI__
#define __FLTK_BSDI__    1
#endif
#if defined(_arch_dreamcast)
#undef __FLTK_DREAMCAST__
#define __FLTK_DREAMCAST__   1
#endif
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__DragonFly__)
#undef __FLTK_FREEBSD__
#define __FLTK_FREEBSD__ 1
#endif
#if defined(hpux) || defined(__hpux) || defined(__hpux__)
#undef __FLTK_HPUX__
#define __FLTK_HPUX__    1
#endif
#if defined(sgi) || defined(__sgi) || defined(__sgi__) || defined(_SGI_SOURCE)
#undef __FLTK_IRIX__
#define __FLTK_IRIX__    1
#endif
#if defined(linux) || defined(__linux) || defined(__linux__)
#undef __FLTK_LINUX__
#define __FLTK_LINUX__   1
#endif
#if defined(ANDROID) || defined(__ANDROID__)
#undef __FLTK_ANDROID__
#undef __FLTK_LINUX__ /* do we need to do this? */
#define __FLTK_ANDROID__ 1
#endif

#if defined(__APPLE__)
/* lets us know what version of Mac OS X we're compiling on */
#include "AvailabilityMacros.h"
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE
/* if compiling for iPhone */
#undef __FLTK_IPHONEOS__
#define __FLTK_IPHONEOS__ 1
#undef __FLTK_MACOSX__
#elif TARGET_IPHONE_SIMULATOR
// iOS Simulator
#undef __FLTK_IPHONEOS__
#define __FLTK_IPHONEOS__ 1
#undef __FLTK_MACOSX__
#else
/* if not compiling for iPhone */
#undef __FLTK_MACOSX__
#define __FLTK_MACOSX__  1
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1050
# error SDL for Mac OS X only supports deploying on 10.5 and above.
#endif /* MAC_OS_X_VERSION_MIN_REQUIRED < 1050 */
#endif /* TARGET_OS_IPHONE */
#endif /* defined(__APPLE__) */

#if defined(__NetBSD__)
#undef __FLTK_NETBSD__
#define __FLTK_NETBSD__  1
#endif
#if defined(__OpenBSD__)
#undef __FLTK_OPENBSD__
#define __FLTK_OPENBSD__ 1
#endif
#if defined(__OS2__)
#undef __FLTK_OS2__
#define __FLTK_OS2__     1
#endif
#if defined(osf) || defined(__osf) || defined(__osf__) || defined(_OSF_SOURCE)
#undef __FLTK_OSF__
#define __FLTK_OSF__     1
#endif
#if defined(__QNXNTO__)
#undef __FLTK_QNXNTO__
#define __FLTK_QNXNTO__  1
#endif
#if defined(riscos) || defined(__riscos) || defined(__riscos__)
#undef __FLTK_RISCOS__
#define __FLTK_RISCOS__  1
#endif
#if defined(__SVR4)
#undef __FLTK_SOLARIS__
#define __FLTK_SOLARIS__ 1
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
/* Try to find out if we're compiling for WinRT or non-WinRT */
/* If _USING_V110_SDK71_ is defined it means we are using the v110_xp or v120_xp toolset. */
#if /*defined(__MINGW32__) ||*/ (defined(_MSC_VER) && (_MSC_VER >= 1700) && !_USING_V110_SDK71_)	// _MSC_VER==1700 for MSVC 2012
#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#undef __FLTK_WINDOWS__
#define __FLTK_WINDOWS__   1
// See if we're compiling for WinRT:
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#undef __FLTK_WINRT__
#define __FLTK_WINRT__ 1
#endif
#elif defined(_WIN32_WCE)
#undef __FLTK_WINCE__
#define __FLTK_WINCE__ 1
#else
#undef __FLTK_WINDOWS__
#define __FLTK_WINDOWS__   1
#endif /* _MSC_VER < 1700 */
#endif /* defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__) */

#if defined(__FLTK_WINDOWS__)
#undef __FLTK_WIN32__
#define __FLTK_WIN32__ 1
#endif
#if defined(__PSP__)
#undef __FLTK_PSP__
#define __FLTK_PSP__ 1
#endif

#if defined(__S60_32__)
#undef __FLTK_WIN32__
#undef __FLTK_S60v32__
#define __FLTK_S60v32__   1
#endif

#endif /* _FL_Platform_h */
