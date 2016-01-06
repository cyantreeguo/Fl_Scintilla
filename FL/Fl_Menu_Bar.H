//
// "$Id: Fl_Menu_Bar.H 9637 2012-07-24 04:37:22Z matt $"
//
// Menu bar header file for the Fast Light Tool Kit (FLTK).
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
   Fl_Menu_Bar widget . */

#ifndef Fl_Menu_Bar_H
#define Fl_Menu_Bar_H

#include "Fl_Menu_.H"

/**
  This is a button that when pushed pops up a menu (or hierarchy of
  menus) defined by an array of 
  Fl_Menu_Item objects.
  <P ALIGN=CENTER>\image html  menu_button.png</P>
  \image latex  menu_button.png " menu_button" width=5cm
  <P>Normally any mouse button will pop up a menu and it is lined up
  below the button as shown in the picture.  However an Fl_Menu_Button
  may also control a pop-up menu.  This is done by setting the type().
  If type() is zero a normal menu button is produced. 
  If it is nonzero then this is a pop-up menu. The bits in type() indicate 
  what mouse buttons pop up the menu (see Fl_Menu_Button::popup_buttons). </P>
  <P>The menu will also pop up in response to shortcuts indicated by
  putting a '&' character in the label(). </P>
  <P>Typing the shortcut() of any of the menu items will cause
  callbacks exactly the same as when you pick the item with the mouse.
  The '&' character in menu item names are only looked at when the menu is
  popped up, however. </P>

  When the user clicks a menu item, value() is set to that item
  and then:

      - The item's callback is done if one has been set; the
        Fl_Menu_Button is passed as the Fl_Widget* argument, 
        along with any userdata configured for the callback.

      - If the item does not have a callback, the Fl_Menu_Button's callback
        is done instead, along with any userdata configured for it.
        The callback can determine which item was picked using
        value(), mvalue(), item_pathname(), etc.
*/
class FL_EXPORT Fl_Menu_Bar : public Fl_Menu_
{
protected:
	void draw();
public:
	int handle(int);
	/**
	    Creates a new Fl_Menu_Bar widget using the given position,
	  size, and label string. The default boxtype is FL_UP_BOX.
	  <P>The constructor sets menu() to NULL.  See
	  Fl_Menu_ for the methods to set or change the menu. </P>
	  <P>labelsize(), labelfont(), and labelcolor()
	  are used to control how the menubar items are drawn.  They are
	  initialized from the Fl_Menu static variables, but you can
	  change them if desired. </P>
	  <P>label() is ignored unless you change align() to
	  put it outside the menubar.
	  <P>The destructor removes the Fl_Menu_Bar widget and all of its
	  menu items.
	*/
	Fl_Menu_Bar(int X, int Y, int W, int H,const char *l=0);
};

#endif

//
// End of "$Id: Fl_Menu_Bar.H 9637 2012-07-24 04:37:22Z matt $".
//
