//
// "$Id: fl_draw_image.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// Image drawing routines for the Fast Light Tool Kit (FLTK).
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

// I hope a simple and portable method of drawing color and monochrome
// images.  To keep this simple, only a single storage type is
// supported: 8 bit unsigned data, byte order RGB, and pixels are
// stored packed into rows with the origin at the top-left.  It is
// possible to alter the size of pixels with the "delta" argument, to
// add alpha or other information per pixel.  It is also possible to
// change the origin and direction of the image data by messing with
// the "delta" and "linedelta", making them negative, though this may
// defeat some of the shortcuts in translating the image for X.

#include "Fl_Platform.h"

#if __FLTK_WIN32__
#  include "os/win32/fl_draw_image.cxx"
#elif __FLTK_MACOSX__
#  include "os/osx/fl_draw_image.cxx"
#elif __FLTK_IPHONEOS__
#  include "os/ios/fl_draw_image.cxx"
#elif __FLTK_LINUX__
#  include "os/linux/fl_draw_image.cxx"
#elif __FLTK_WINCE__
#  include "os/wince/fl_draw_image.cxx"
#elif __FLTK_S60v32__
#  include "os/s60v32/fl_draw_image_s60.cxx"
#elif __FLTK_ANDROID__
#  include "os/android/fl_draw_image.cxx"
#else
#error unsupported platform
#endif

//
// End of "$Id: fl_draw_image.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
