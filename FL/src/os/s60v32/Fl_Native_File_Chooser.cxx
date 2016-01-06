// "$Id: Fl_Native_File_Chooser_WIN32.cxx 10138 2014-05-01 09:00:30Z ianmacarthur $"
//
// FLTK native OS file chooser widget
//
// Copyright 1998-2010 by Bill Spitzak and others.
// Copyright 2004 Greg Ercolano.
// API changes + filter improvements by Nathan Vander Wilt 2005
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

// Any application to multi-folder implementation:
//     http://www.codeproject.com/dialog/selectfolder.asp
//

#include "Fl_Platform.h"
#if __FLTK_S60v32__

#ifndef FL_DOXYGEN		// PREVENT DOXYGEN'S USE OF THIS FILE

#include "Enumerations.H"

#include <stdlib.h>		// malloc
#include <stdio.h>		// sprintf
#include "Fl_Native_File_Chooser.H"

Fl_Native_File_Chooser::Fl_Native_File_Chooser(int val)
{
}

Fl_Native_File_Chooser::~Fl_Native_File_Chooser()
{
}

void Fl_Native_File_Chooser::type(int val)
{
}

int Fl_Native_File_Chooser::type() const
{
}

void Fl_Native_File_Chooser::options(int val)
{
}

int Fl_Native_File_Chooser::options() const
{
	return 0;
}

// RETURNS:
//    0 - user picked a file
//    1 - user cancelled
//   -1 - failed; errmsg() has reason
//
int Fl_Native_File_Chooser::show()
{
	return 0;
}

// RETURN ERROR MESSAGE
const char *Fl_Native_File_Chooser::errmsg() const
{
	return("No error");
}

// GET FILENAME
const char* Fl_Native_File_Chooser::filename() const
{
	return("");
}

// GET FILENAME FROM LIST OF FILENAMES
const char* Fl_Native_File_Chooser::filename(int i) const
{
	return("");
}

// GET TOTAL FILENAMES CHOSEN
int Fl_Native_File_Chooser::count() const
{
	return 0;
}

// PRESET PATHNAME
//     Can be NULL if no preset is desired.
//
void Fl_Native_File_Chooser::directory(const char *val)
{
}

// GET PRESET PATHNAME
//    Can return NULL if none set.
//
const char *Fl_Native_File_Chooser::directory() const
{
	return("");
}

// SET TITLE
//     Can be NULL if no title desired.
//
void Fl_Native_File_Chooser::title(const char *val)
{
}

// GET TITLE
//    Can return NULL if none set.
//
const char *Fl_Native_File_Chooser::title() const
{
	return("");
}

// SET FILTER
//     Can be NULL if no filter needed
//
void Fl_Native_File_Chooser::filter(const char *val)
{
}

// GET FILTER
//    Can return NULL if none set.
//
const char *Fl_Native_File_Chooser::filter() const
{
	return("");
}

// SET 'CURRENTLY SELECTED FILTER'
void Fl_Native_File_Chooser::filter_value(int i)
{
}

// RETURN VALUE OF 'CURRENTLY SELECTED FILTER'
int Fl_Native_File_Chooser::filter_value() const
{
	return 0;
}

// PRESET FILENAME FOR 'SAVE AS' CHOOSER
void Fl_Native_File_Chooser::preset_file(const char* val)
{
}

// GET PRESET FILENAME FOR 'SAVE AS' CHOOSER
const char* Fl_Native_File_Chooser::preset_file() const
{
	return("");
}

int Fl_Native_File_Chooser::filters() const
{
	return(0);
}

#endif /*!FL_DOXYGEN*/

#endif
//
// End of "$Id: Fl_Native_File_Chooser_WIN32.cxx 10138 2014-05-01 09:00:30Z ianmacarthur $".
//
