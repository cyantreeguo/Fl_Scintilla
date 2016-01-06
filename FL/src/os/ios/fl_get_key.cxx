//
// "$Id: Fl_get_key_mac.cxx 9952 2013-07-23 16:02:44Z manolo $"
//
// MacOS keyboard state routines for the Fast Light Tool Kit (FLTK).
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
#include "fltk_config.h"

#if __FLTK_IPHONEOS__

//: returns true, if that key was pressed during the last event
int Fl::event_key(int k)
{
	return get_key(k);
}

//: returns true, if that key is pressed right now
int Fl::get_key(int k)
{
    return 0;
}

#endif

//
// End of "$Id: Fl_get_key_mac.cxx 9952 2013-07-23 16:02:44Z manolo $".
//
