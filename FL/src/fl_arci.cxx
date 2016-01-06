//
// "$Id: fl_arci.cxx 9023 2011-08-30 07:50:16Z AlbrechtS $"
//
// Arc (integer) drawing functions for the Fast Light Tool Kit (FLTK).
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

/**
  \file fl_arci.cxx
  \brief Utility functions for drawing circles using integers
*/

// "integer" circle drawing functions.  These draw the limited
// circle types provided by X and NT graphics.  The advantage of
// these is that small ones draw quite nicely (probably due to stored
// hand-drawn bitmaps of small circles!) and may be implemented by
// hardware and thus are fast.

// Probably should add fl_chord.

// 3/10/98: created

#include "fl_draw.H"
#include "x.H"
#if defined(WIN32) || defined(__S60_32__)
#  include "fltkmath.h"
#endif
#include "fltk_config.h"

void Fl_Graphics_Driver::arc(int x,int y,int w,int h,double a1,double a2)
{
	if (w <= 0 || h <= 0) return;

#if __FLTK_WIN32__
	int xa = x+w/2+int(w*cos(a1/180.0*M_PI));
	int ya = y+h/2-int(h*sin(a1/180.0*M_PI));
	int xb = x+w/2+int(w*cos(a2/180.0*M_PI));
	int yb = y+h/2-int(h*sin(a2/180.0*M_PI));
	if (fabs(a1 - a2) < 90) {
		if (xa == xb && ya == yb) SetPixel(fl_gc, xa, ya, fl_RGB());
		else Arc(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb);
	} else Arc(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb);
#elif __FLTK_WINCE__
	int xa = x+w/2+int(w*cos(a1/180.0*M_PI));
	int ya = y+h/2-int(h*sin(a1/180.0*M_PI));
	int xb = x+w/2+int(w*cos(a2/180.0*M_PI));
	int yb = y+h/2-int(h*sin(a2/180.0*M_PI));
	if (fabs(a1 - a2) < 90) {
		if (xa == xb && ya == yb) SetPixel(fl_gc, xa, ya, fl_RGB());
		else Ellipse(fl_gc, x, y, x+w, y+h);
	} else Ellipse(fl_gc, x, y, x+w, y+h);
#elif __FLTK_MACOSX__
  #if defined(__APPLE_QUARTZ__)
	a1 = (-a1)/180.0f*M_PI;
	a2 = (-a2)/180.0f*M_PI;
	float cx = x + 0.5f*w - 0.5f, cy = y + 0.5f*h - 0.5f;
	CGContextSetShouldAntialias(fl_gc, true);
	if (w!=h) {
		CGContextSaveGState(fl_gc);
		CGContextTranslateCTM(fl_gc, cx, cy);
		CGContextScaleCTM(fl_gc, w-1.0f, h-1.0f);
		CGContextAddArc(fl_gc, 0, 0, 0.5, a1, a2, 1);
		CGContextRestoreGState(fl_gc);
	} else {
		float r = (w+h)*0.25f-0.5f;
		CGContextAddArc(fl_gc, cx, cy, r, a1, a2, 1);
	}
	CGContextStrokePath(fl_gc);
	CGContextSetShouldAntialias(fl_gc, false);
  #endif
#elif __FLTK_IPHONEOS__
  #if defined(__APPLE_QUARTZ__)
	a1 = (-a1)/180.0f*M_PI;
	a2 = (-a2)/180.0f*M_PI;
	float cx = x + 0.5f*w - 0.5f, cy = y + 0.5f*h - 0.5f;
	CGContextSetShouldAntialias(fl_gc, true);
	if (w!=h) {
		CGContextSaveGState(fl_gc);
		CGContextTranslateCTM(fl_gc, cx, cy);
		CGContextScaleCTM(fl_gc, w-1.0f, h-1.0f);
		CGContextAddArc(fl_gc, 0, 0, 0.5, a1, a2, 1);
		CGContextRestoreGState(fl_gc);
	} else {
		float r = (w+h)*0.25f-0.5f;
		CGContextAddArc(fl_gc, cx, cy, r, a1, a2, 1);
	}
	CGContextStrokePath(fl_gc);
	CGContextSetShouldAntialias(fl_gc, false);
  #endif
#elif __FLTK_LINUX__
  #if defined(USE_X11)
	XDrawArc(fl_display, fl_window, fl_gc, x,y,w-1,h-1, int(a1*64),int((a2-a1)*64));
  #endif
#elif __FLTK_S60v32__
	// DONE: S60
	int xa = x+w/2+int(w*cos(a1/180.0*M_PI));
	int ya = y+h/2-int(h*sin(a1/180.0*M_PI));
	int xb = x+w/2+int(w*cos(a2/180.0*M_PI));
	int yb = y+h/2-int(h*sin(a2/180.0*M_PI));
	Fl_X::WindowGc->DrawArc(TRect(x, y, x + w, y + h), TPoint(xa, ya), TPoint(xb, yb));
#elif __FLTK_ANDROID__
	// FIXIT
#else
#error unsupported platform
#endif
}

void Fl_Graphics_Driver::pie(int x,int y,int w,int h,double a1,double a2)
{
	if (w <= 0 || h <= 0) return;

#if __FLTK_WIN32__
	if (a1 == a2) return;
	int xa = x+w/2+int(w*cos(a1/180.0*M_PI));
	int ya = y+h/2-int(h*sin(a1/180.0*M_PI));
	int xb = x+w/2+int(w*cos(a2/180.0*M_PI));
	int yb = y+h/2-int(h*sin(a2/180.0*M_PI));
	SelectObject(fl_gc, fl_brush());
	if (fabs(a1 - a2) < 90) {
		if (xa == xb && ya == yb) {
			MoveToEx(fl_gc, x+w/2, y+h/2, 0L);
			LineTo(fl_gc, xa, ya);
			SetPixel(fl_gc, xa, ya, fl_RGB());
		} else Pie(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb);
	} else Pie(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb);
#elif __FLTK_WINCE__
	if (a1 == a2) return;
	int xa = x+w/2+int(w*cos(a1/180.0*M_PI));
	int ya = y+h/2-int(h*sin(a1/180.0*M_PI));
	int xb = x+w/2+int(w*cos(a2/180.0*M_PI));
	int yb = y+h/2-int(h*sin(a2/180.0*M_PI));
	SelectObject(fl_gc, fl_brush());
	if (fabs(a1 - a2) < 90) {
		if (xa == xb && ya == yb) {
			MoveToEx(fl_gc, x+w/2, y+h/2, 0L);
			LineTo(fl_gc, xa, ya);
			SetPixel(fl_gc, xa, ya, fl_RGB());
		} //else Pie(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb);
	} //else Pie(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb);
#elif __FLTK_MACOSX__
  #if defined(__APPLE_QUARTZ__)
	a1 = (-a1)/180.0f*M_PI;
	a2 = (-a2)/180.0f*M_PI;
	float cx = x + 0.5f*w - 0.5f, cy = y + 0.5f*h - 0.5f;
	CGContextSetShouldAntialias(fl_gc, true);
	if (w!=h) {
		CGContextSaveGState(fl_gc);
		CGContextTranslateCTM(fl_gc, cx, cy);
		CGContextScaleCTM(fl_gc, w, h);
		CGContextAddArc(fl_gc, 0, 0, 0.5, a1, a2, 1);
		CGContextAddLineToPoint(fl_gc, 0, 0);
		CGContextClosePath(fl_gc);
		CGContextRestoreGState(fl_gc);
	} else {
		float r = (w+h)*0.25f;
		CGContextAddArc(fl_gc, cx, cy, r, a1, a2, 1);
		CGContextAddLineToPoint(fl_gc, cx, cy);
		CGContextClosePath(fl_gc);
	}
	CGContextFillPath(fl_gc);
	CGContextSetShouldAntialias(fl_gc, false);
  #endif
#elif __FLTK_IPHONEOS__
  #if defined(__APPLE_QUARTZ__)
	a1 = (-a1)/180.0f*M_PI;
	a2 = (-a2)/180.0f*M_PI;
	float cx = x + 0.5f*w - 0.5f, cy = y + 0.5f*h - 0.5f;
	CGContextSetShouldAntialias(fl_gc, true);
	if (w!=h) {
		CGContextSaveGState(fl_gc);
		CGContextTranslateCTM(fl_gc, cx, cy);
		CGContextScaleCTM(fl_gc, w, h);
		CGContextAddArc(fl_gc, 0, 0, 0.5, a1, a2, 1);
		CGContextAddLineToPoint(fl_gc, 0, 0);
		CGContextClosePath(fl_gc);
		CGContextRestoreGState(fl_gc);
	} else {
		float r = (w+h)*0.25f;
		CGContextAddArc(fl_gc, cx, cy, r, a1, a2, 1);
		CGContextAddLineToPoint(fl_gc, cx, cy);
		CGContextClosePath(fl_gc);
	}
	CGContextFillPath(fl_gc);
	CGContextSetShouldAntialias(fl_gc, false);
  #endif
#elif __FLTK_LINUX__
  #if defined(USE_X11)
	XDrawArc(fl_display, fl_window, fl_gc, x,y,w-1,h-1, int(a1*64),int((a2-a1)*64));
	XFillArc(fl_display, fl_window, fl_gc, x,y,w-1,h-1, int(a1*64),int((a2-a1)*64));
  #endif
#elif __FLTK_S60v32__
	// DONE: S60
	int xa = x+w/2+int(w*cos(a1/180.0*M_PI));
	int ya = y+h/2-int(h*sin(a1/180.0*M_PI));
	int xb = x+w/2+int(w*cos(a2/180.0*M_PI));
	int yb = y+h/2-int(h*sin(a2/180.0*M_PI));
	Fl_X::WindowGc->DrawPie(TRect(x, y, x + w, y + h), TPoint(xa, ya), TPoint(xb, yb));
#elif __FLTK_ANDROID__
	// FIXIT	
#else
#error unsupported platform
#endif
}

//
// End of "$Id: fl_arci.cxx 9023 2011-08-30 07:50:16Z AlbrechtS $".
//
