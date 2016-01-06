//
// "$Id: fl_set_fonts.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// More font utilities for the Fast Light Tool Kit (FLTK).
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

#include "Fl.H"
#include "x.H"
#include "Fl_Font.H"
#include "flstring.h"
#include <stdlib.h>

#if __FLTK_WIN32__
#  include "os/win32/fl_set_fonts.cxx"
#elif __FLTK_MACOSX__
#  include "os/osx/fl_set_fonts.cxx"
#elif __FLTK_IPHONEOS__
#  include "os/ios/fl_set_fonts.cxx"
#elif __FLTK_WINCE__
#  include "os/wince/fl_set_fonts.cxx"
#elif __FLTK_LINUX__
#if USE_XFT
#  include "os/linux/xft_fl_set_fonts.cxx"
#else
#  include "os/linux/x_fl_set_fonts.cxx"
#endif // WIN32
#elif __FLTK_S60v32__
#  include "os/s60v32/fl_set_fonts_s60.cxx"
#elif __FLTK_ANDROID__
#  include "os/android/fl_set_fonts.cxx"
#else
#error unsupported platform
#endif

//
// End of "$Id: fl_set_fonts.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
