//
// "$Id: Fl_Return_Button.H 9637 2012-07-24 04:37:22Z matt $"
//
// Return button header file for the Fast Light Tool Kit (FLTK).
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

/* \file
   Fl_Return_Button widget . */

#ifndef Fl_Return_Button_H
#define Fl_Return_Button_H

#include "Fl.H"
#include "fl_draw.H"
#include "Fl_Button.H"

/**
  The Fl_Return_Button is a subclass of Fl_Button that
  generates a callback when it is pressed or when the user presses the
  Enter key.  A carriage-return symbol is drawn next to the button label.
  <P ALIGN=CENTER>\image html Fl_Return_Button.png
  \image latex Fl_Return_Button.png "Fl_Return_Button" width=4cm
*/
class FL_EXPORT Fl_Return_Button : public Fl_Button
{
protected:
	static int fl_return_arrow(int x, int y, int w, int h) {
		int size = w;
		if (h<size) size = h;
		int d = (size+2)/4;
		if (d<3) d = 3;
		int t = (size+9)/12;
		if (t<1) t = 1;
		int x0 = x+(w-2*d-2*t-1)/2;
		int x1 = x0+d;
		int y0 = y+h/2;
		fl_color(FL_LIGHT3);
		fl_line(x0, y0, x1, y0+d);
		fl_yxline(x1, y0+d, y0+t, x1+d+2*t, y0-d);
		fl_yxline(x1, y0-t, y0-d);
		fl_color(fl_gray_ramp(0));
		fl_line(x0, y0, x1, y0-d);
		fl_color(FL_DARK3);
		fl_xyline(x1+1, y0-t, x1+d, y0-d, x1+d+2*t);
		return 1;
	}

	void draw() {
		if (type() == FL_HIDDEN_BUTTON) return;
		Fl_Boxtype bt = value() ? (down_box()?down_box():fl_down(box())) : box();
		int dx = Fl::box_dx(bt);
		draw_box(bt, value() ? selection_color() : color());
		int W = h();
		if (w()/3 < W) W = w()/3;
		fl_return_arrow(x()+w()-(W+dx), y(), W, h());
		draw_label(x()+dx, y(), w()-(dx+W+dx), h());
		if (Fl::focus() == this) draw_focus();
	}

public:
	int handle(int event) {
		if (event == FL_SHORTCUT && (Fl::event_key() == FL_Enter || Fl::event_key() == FL_KP_Enter)) {
			simulate_key_action();
			do_callback();
			return 1;
		} else {
			return Fl_Button::handle(event);
		}
	}

	/**
	  Creates a new Fl_Return_Button widget using the given
	  position, size, and label string. The default boxtype is FL_UP_BOX.
	  <P> The inherited destructor deletes the button.
	*/
	Fl_Return_Button(int X, int Y, int W, int H,const char *l=0) : Fl_Button(X,Y,W,H,l) {}
};

#endif

//
// End of "$Id: Fl_Return_Button.H 9637 2012-07-24 04:37:22Z matt $".
//
