//
// "$Id: Fl_Dial.H 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// Dial header file for the Fast Light Tool Kit (FLTK).
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
   Fl_Dial widget . */

#ifndef Fl_Dial_H
#define Fl_Dial_H

#include "Fl.H"
#include "fl_draw.H"
#include <stdlib.h>
#include "fltkmath.h"

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

// values for type():
#define FL_NORMAL_DIAL	0	/**< type() for dial variant with dot */
#define FL_LINE_DIAL	1	/**< type() for dial variant with line */
#define FL_FILL_DIAL	2	/**< type() for dial variant with filled arc */

/**
  The Fl_Dial widget provides a circular dial to control a
  single floating point value.
  <P ALIGN=CENTER>\image html dial.png
  \image latex dial.png "Fl_Dial" width=4cm
  Use type() to set the type of the dial to:
  <UL>
  <LI>FL_NORMAL_DIAL - Draws a normal dial with a knob. </LI>
  <LI>FL_LINE_DIAL - Draws a dial with a line. </LI>
  <LI>FL_FILL_DIAL - Draws a dial with a filled arc. </LI>
  </UL>

*/
class FL_EXPORT Fl_Dial : public Fl_Valuator
{

	short a1,a2;

protected:

	// these allow subclasses to put the dial in a smaller area:
	// All angles are measured with 0 to the right and counter-clockwise
	/**
	Draws dial at given position and size.
	\param[in] X, Y, W, H position and size
	*/
	void draw(int X, int Y, int W, int H)
	{
		if (damage()&FL_DAMAGE_ALL) draw_box(box(), X, Y, W, H, color());
		X += Fl::box_dx(box());
		Y += Fl::box_dy(box());
		W -= Fl::box_dw(box());
		H -= Fl::box_dh(box());
		double angle = (a2-a1)*(value()-minimum())/(maximum()-minimum()) + a1;
		if (type() == FL_FILL_DIAL) {
			// foo: draw this nicely in certain round box types
			int foo = (box() > _FL_ROUND_UP_BOX && Fl::box_dx(box()));
			if (foo) {
				X--;
				Y--;
				W+=2;
				H+=2;
			}
			if (active_r()) fl_color(color());
			else fl_color(fl_inactive(color()));
			fl_pie(X, Y, W, H, 270-a1, angle > a1 ? 360+270-angle : 270-360-angle);
			if (active_r()) fl_color(selection_color());
			else fl_color(fl_inactive(selection_color()));
			fl_pie(X, Y, W, H, 270-angle, 270-a1);
			if (foo) {
				if (active_r()) fl_color(FL_FOREGROUND_COLOR);
				else fl_color(fl_inactive(FL_FOREGROUND_COLOR));
				fl_arc(X, Y, W, H, 0, 360);
			}
			return;
		}
		if (!(damage()&FL_DAMAGE_ALL)) {
			if (active_r()) fl_color(color());
			else fl_color(fl_inactive(color()));
			fl_pie(X+1, Y+1, W-2, H-2, 0, 360);
		}
		fl_push_matrix();
		fl_translate(X+W/2-.5, Y+H/2-.5);
		fl_scale(W-1, H-1);
		fl_rotate(45-angle);
		if (active_r()) fl_color(selection_color());
		else fl_color(fl_inactive(selection_color()));
		if (type()) { // FL_LINE_DIAL
			fl_begin_polygon();
			fl_vertex(0.0,   0.0);
			fl_vertex(-0.04, 0.0);
			fl_vertex(-0.25, 0.25);
			fl_vertex(0.0,   0.04);
			fl_end_polygon();
			if (active_r()) fl_color(FL_FOREGROUND_COLOR);
			else fl_color(fl_inactive(FL_FOREGROUND_COLOR));
			fl_begin_loop();
			fl_vertex(0.0,   0.0);
			fl_vertex(-0.04, 0.0);
			fl_vertex(-0.25, 0.25);
			fl_vertex(0.0,   0.04);
			fl_end_loop();
		} else {
			fl_begin_polygon();
			fl_circle(-0.20, 0.20, 0.07);
			fl_end_polygon();
			if (active_r()) fl_color(FL_FOREGROUND_COLOR);
			else fl_color(fl_inactive(FL_FOREGROUND_COLOR));
			fl_begin_loop();
			fl_circle(-0.20, 0.20, 0.07);
			fl_end_loop();
		}
		fl_pop_matrix();
	}

	/**
	Allows subclasses to handle event based on given position and size.
	\param[in] event, X, Y, W, H event to handle, related position and size.
	*/
	int handle(int event, int X, int Y, int W, int H)
	{
		switch (event) {
		case FL_PUSH: {
			Fl_Widget_Tracker wp(this);
			handle_push();
			if (wp.deleted()) return 1;
		}
		case FL_DRAG: {
			int mx = (Fl::event_x()-X-W/2)*H;
			int my = (Fl::event_y()-Y-H/2)*W;
			if (!mx && !my) return 1;
			double angle = 270-atan2((float)-my, (float)mx)*180/M_PI;
			double oldangle = (a2-a1)*(value()-minimum())/(maximum()-minimum()) + a1;
			while (angle < oldangle-180) angle += 360;
			while (angle > oldangle+180) angle -= 360;
			double val;
			if ((a1<a2) ? (angle <= a1) : (angle >= a1)) {
				val = minimum();
			} else if ((a1<a2) ? (angle >= a2) : (angle <= a2)) {
				val = maximum();
			} else {
				val = minimum() + (maximum()-minimum())*(angle-a1)/(a2-a1);
			}
			handle_drag(clamp(round(val)));
		}
		return 1;
		case FL_RELEASE:
			handle_release();
			return 1;
		case FL_ENTER : /* FALLTHROUGH */
		case FL_LEAVE :
			return 1;
		default:
			return 0;
		}
	}

	/**
	Draws dial at current position and size.
	*/
	void draw()
	{
		draw(x(), y(), w(), h());
		draw_label();
	}

public:
	/**
	Allow subclasses to handle event based on current position and size.
	*/
	int handle(int e)
	{
		return handle(e, x(), y(), w(), h());
	}

	/**
	  Creates a new Fl_Dial widget using the given position, size,
	  and label string. The default type is FL_NORMAL_DIAL.
	*/
	Fl_Dial(int X, int Y, int W, int H, const char* l=0)
	/**
	Creates a new Fl_Dial widget using the given position, size,
	and label string. The default type is FL_NORMAL_DIAL.
	*/
		: Fl_Valuator(X, Y, W, H, l)
	{
		box(FL_OVAL_BOX);
		selection_color(FL_INACTIVE_COLOR); // was 37
		a1 = 45;
		a2 = 315;
	}

	/**
	   Sets Or gets the angles used for the minimum and maximum values.  The default
	   values are 45 and 315 (0 degrees is straight down and the angles
	   progress clockwise).  Normally angle1 is less than angle2, but if you
	   reverse them the dial moves counter-clockwise.
	*/
	short angle1() const {
		return a1;
	}
	/** See short angle1() const */
	void angle1(short a) {
		a1 = a;
	}
	/** See short angle1() const */
	short angle2() const {
		return a2;
	}
	/** See short angle1() const */
	void angle2(short a) {
		a2 = a;
	}
	/** See short angle1() const */
	void angles(short a, short b) {
		a1 = a;
		a2 = b;
	}

};

#endif

//
// End of "$Id: Fl_Dial.H 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
