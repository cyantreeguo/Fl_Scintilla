//
// "$Id: fl_dnd.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// Drag & Drop code for the Fast Light Tool Kit (FLTK).
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

#include "Fl_Platform.h"

#if __FLTK_WIN32__
#  include "os/win32/fl_dnd.cxx"
#elif __FLTK_IPHONEOS__
#elif __FLTK_MACOSX__
#elif __FLTK_LINUX__
#  include "os/linux/x_fl_dnd.cxx"
#elif __FLTK_WINCE__
#  include "os/wince/fl_dnd.cxx"
#elif __FLTK_S60v32__
#  include "os/s60v32/fl_dnd_s60.cxx"
#elif __FLTK_ANDROID__
#  include "os/android/fl_dnd.cxx"
#else
#error unsupported platform
#endif

//
// End of "$Id: fl_dnd.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
