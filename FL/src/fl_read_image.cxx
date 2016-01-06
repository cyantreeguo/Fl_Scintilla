//
// "$Id: fl_read_image.cxx 9980 2013-09-21 16:41:23Z greg.ercolano $"
//
// X11 image reading routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include "x.H"
#include "Fl.H"
#include "fl_draw.H"
#include "flstring.h"

#ifdef DEBUG
#  include <stdio.h>
#endif // DEBUG

#if __FLTK_WIN32__
#  include "os/win32/fl_read_image.cxx"
#elif __FLTK_MACOSX__
#  include "os/osx/fl_read_image.cxx"
#elif __FLTK_IPHONEOS__
#  include "os/ios/fl_read_image.cxx"
#elif __FLTK_WINCE__
#  include "os/wince/fl_read_image.cxx"
#elif __FLTK_LINUX__
#  include "os/linux/fl_read_image.cxx"
#elif __FLTK_S60v32__
#  include "os/s60v32/fl_read_image_s60.cxx"
#elif __FLTK_ANDROID__
#  include "os/android/fl_read_image.cxx"
#else
#error unsupported platform
#endif

//
// End of "$Id: fl_read_image.cxx 9980 2013-09-21 16:41:23Z greg.ercolano $".
//
