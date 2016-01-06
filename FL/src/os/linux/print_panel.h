//
// "$Id: print_panel.h 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// Print panel for the Fast Light Tool Kit (FLTK).
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

//
// This is a temporary file.  It is only for development and will
// probably be removed later.
//

#ifndef print_panel_h
#define print_panel_h
#include "Fl.H"
#include "Fl_Double_Window.H"
#include "Fl_Group.H"
#include "Fl_Choice.H"
#include "Fl_Button.H"
#include "Fl_Box.H"
#include "Fl_Round_Button.H"
#include "Fl_Input.H"
#include "Fl_Spinner.H"
#include "Fl_Check_Button.H"
#include "Fl_Return_Button.H"
#include "Fl_Progress.H"
enum printing_style {SystemV, BSD};
static Fl_Double_Window* make_print_panel();
static void print_cb(Fl_Return_Button *, void *);
static printing_style print_load();
static void print_update_status();
#endif

//
// End of "$Id: print_panel.h 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
