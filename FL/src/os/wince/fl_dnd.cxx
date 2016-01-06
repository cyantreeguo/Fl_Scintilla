//
// "$Id: fl_dnd_win32.cxx 9677 2012-08-18 11:32:50Z AlbrechtS $"
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

// This file contains win32-specific code for fltk which is always linked
// in.  Search other files for "WIN32" or filenames ending in _win32.cxx
// for other system-specific code.

#include "Fl.H"
#include "x.H"
#include "Fl_Window.H"
#include "fl_utf8.h"
#include "flstring.h"
#include <stdio.h>
#include <stdlib.h>
#if __FLTK_WINCE__
#include "wince_compate.h"
#else
#include <sys/types.h>
#endif
#include <objidl.h>
#include <time.h>
#if defined(__CYGWIN__)
#include <sys/time.h>
#include <unistd.h>
#endif

extern char *fl_selection_buffer[2];
extern int fl_selection_length[2];
extern int fl_selection_buffer_length[2];
extern char fl_i_own_selection[2];
extern char *fl_locale2utf8(const char *s, UINT codepage = 0);
extern unsigned int fl_codepage;

Fl_Window *fl_dnd_target_window = 0;

int Fl::dnd()
{
	return 0;
}

//
// End of "$Id: fl_dnd_win32.cxx 9677 2012-08-18 11:32:50Z AlbrechtS $".
//
