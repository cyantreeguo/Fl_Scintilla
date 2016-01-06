/*
 * "$Id: scandir.c 9858 2013-04-05 14:14:08Z manolo $"
 *
 * This is a placekeeper stub that pulls in scandir implementations for host
 * systems that do not provide a compatible one natively
 *
 * Copyright 1998-2013 by Bill Spitzak and others.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     http://www.fltk.org/COPYING.php
 *
 * Please report all bugs and problems on the following page:
 *
 *     http://www.fltk.org/str.php
 */

#include "Fl_Platform.h"

#if __FLTK_WIN32__
#  include "os/win32/scandir.cxx"
#elif __FLTK_WINCE__
#  include "os/wince/scandir.cxx"
#else
#  ifndef HAVE_SCANDIR
#   include "os/posix_scandir.cxx"
#  endif /* HAVE_SCANDIR */
#endif

/*
 * End of "$Id: scandir.c 9858 2013-04-05 14:14:08Z manolo $".
 */
