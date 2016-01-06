//
// "$Id: Fl_Window_iconize.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// Window minification code for the Fast Light Tool Kit (FLTK).
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

extern char fl_show_iconic; // in Fl_x.cxx

void Fl_Window::iconize()
{
	if (!shown()) {
		fl_show_iconic = 1;
		show();
	} else {
#if __FLTK_WIN32__
		ShowWindow(i->xid, SW_SHOWMINNOACTIVE);
#elif __FLTK_MACOSX__
		i->collapse();
#elif __FLTK_LINUX__
		XIconifyWindow(fl_display, i->xid, fl_screen);
#elif __FLTK_S60v32__
		//
#endif
	}
}

//
// End of "$Id: Fl_Window_iconize.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
