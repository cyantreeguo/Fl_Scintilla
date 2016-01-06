// "$Id: Fl_Native_File_Chooser.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// FLTK native OS file chooser widget
//
// Copyright 1998-2010 by Bill Spitzak and others.
// Copyright 2004 Greg Ercolano.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include "Fl_Platform.h"

/*
#if __FLTK_WIN32__
#include "os\win32\Fl_Native_File_Chooser.cxx"
#elif __FLTK_IPHONEOS__
#include "Fl_Native_File_Chooser.H"
#elif __FLTK_MACOSX__
#include "fltk_config_mac.h"
#elif __FLTK_LINUX__
#include "fltk_config_linux.h"
#elif __FLTK_WINCE__
#include "os\wince\Fl_Native_File_Chooser.cxx"
#else
#error unsupported platform
#endif
*/

// Use Windows' chooser
#if __FLTK_WIN32__
#include "os/win32/Fl_Native_File_Chooser.cxx"
#endif

#if __FLTK_WINCE__
#include "os/wince/Fl_Native_File_Chooser.cxx"
#endif

// Use Apple's chooser
#ifdef __APPLE__
#include "Fl_Native_File_Chooser.H"
#endif

// All else falls back to FLTK's own chooser
//#if ! defined(__APPLE__) && !defined(WIN32)
#if __FLTK_LINUX__
#include "os/linux/fltk_Fl_Native_File_Chooser.cxx"
#endif

#if __FLTK_S60v32__
#include "os/s60v32/Fl_Native_File_Chooser.cxx"
#endif

#if __FLTK_ANDROID__
#include "os/android/Fl_Native_File_Chooser.cxx"
#endif

const char *Fl_Native_File_Chooser::file_exists_message = "File exists. Are you sure you want to overwrite?";

//
// End of "$Id: Fl_Native_File_Chooser.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
