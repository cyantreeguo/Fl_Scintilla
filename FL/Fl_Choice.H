//
// "$Id: Fl_Choice.H 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// Choice header file for the Fast Light Tool Kit (FLTK).
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
   Fl_Choice widget . */

#ifndef Fl_Choice_H
#define Fl_Choice_H

#include "Fl.H"
#include "Fl_Menu_.H"
#include "fl_draw.H"
#include "src/flstring.h"

/**
  \class Fl_Choice
  \brief A button that is used to pop up a menu.

  This is a button that, when pushed, pops up a menu (or hierarchy of menus)
  defined by an array of Fl_Menu_Item objects.
  Motif calls this an OptionButton.

  The only difference between this and a Fl_Menu_Button is that the name of
  the most recent chosen menu item is displayed inside the box, while the
  label is displayed outside the box. However, since the use of this is most
  often to control a single variable rather than do individual callbacks,
  some of the Fl_Menu_Button methods are redescribed here in those terms.

  When the user clicks a menu item, value() is set to that item
  and then:

      - The item's callback is done if one has been set; the
        Fl_Choice is passed as the Fl_Widget* argument,
        along with any userdata configured for the callback.

      - If the item does not have a callback, the Fl_Choice widget's
        callback is done instead, along with any userdata configured
        for it.  The callback can determine which item was picked using
        value(), mvalue(), item_pathname(), etc.

  All three mouse buttons pop up the menu. The Forms behavior of the first
  two buttons to increment/decrement the choice is not implemented.  This
  could be added with a subclass, however.

  The menu will also pop up in response to shortcuts indicated by putting
  a '\&' character in the label().  See Fl_Button::shortcut(int s) for a
  description of this.

  Typing the shortcut() of any of the items will do exactly the same as when
  you pick the item with the mouse.  The '\&' character in item names are
  only looked at when the menu is popped up, however.

  \image html choice.png
  \image latex choice.png  "Fl_Choice" width=4cm
  \todo Refactor the doxygen comments for Fl_Choice changed() documentation.

  \li <tt>int Fl_Widget::changed() const</tt>
      This value is true the user picks a different value. <em>It is turned
      off by value() and just before doing a callback (the callback can turn
      it back on if desired).</em>
  \li <tt>void Fl_Widget::set_changed()</tt>
      This method sets the changed() flag.
  \li <tt>void Fl_Widget::clear_changed()</tt>
      This method clears the changed() flag.
  \li <tt>Fl_Boxtype Fl_Choice::down_box() const</tt>
      Gets the current down box, which is used when the menu is popped up.
      The default down box type is \c FL_DOWN_BOX.
  \li <tt>void Fl_Choice::down_box(Fl_Boxtype b)</tt>
      Sets the current down box type to \p b.
 */
class FL_EXPORT Fl_Choice : public Fl_Menu_
{
protected:
	// Emulates the Forms choice widget.  This is almost exactly the same
	// as an Fl_Menu_Button.  The only difference is the appearance of the
	// button: it draws the text of the current pick and a down-arrow.
	void draw() {
		int dx = Fl::box_dx(FL_DOWN_BOX);
		int dy = Fl::box_dy(FL_DOWN_BOX);
		int H = h() - 2 * dy;
		int W = (H > 20) ? 20 : H;
		int X = x() + w() - W - dx;
		int Y = y() + dy;
		int w1 = (W - 4) / 3;
		if (w1 < 1) w1 = 1;
		int x1 = X + (W - 2 * w1 - 1) / 2;
		int y1 = Y + (H - w1 - 1) / 2;

		if (Fl::scheme()) {
			draw_box(FL_UP_BOX, color());

			fl_color(active_r() ? labelcolor() : fl_inactive(labelcolor()));
			if (Fl::is_scheme("plastic")) {
				// Show larger up/down arrows...
				fl_polygon(x1, y1 + 3, x1 + w1, y1 + w1 + 3, x1 + 2 * w1, y1 + 3);
				fl_polygon(x1, y1 + 1, x1 + w1, y1 - w1 + 1, x1 + 2 * w1, y1 + 1);
			} else {
				// Show smaller up/down arrows with a divider...
				x1 = x() + w() - 13 - dx;
				y1 = y() + h() / 2;
				fl_polygon(x1, y1 - 2, x1 + 3, y1 - 5, x1 + 6, y1 - 2);
				fl_polygon(x1, y1 + 2, x1 + 3, y1 + 5, x1 + 6, y1 + 2);

				fl_color(fl_darker(color()));
				fl_yxline(x1 - 7, y1 - 8, y1 + 8);

				fl_color(fl_lighter(color()));
				fl_yxline(x1 - 6, y1 - 8, y1 + 8);
			}
		} else {
			if (fl_contrast(textcolor(), FL_BACKGROUND2_COLOR) == textcolor()) {
				draw_box(FL_DOWN_BOX, FL_BACKGROUND2_COLOR);
			} else {
				draw_box(FL_DOWN_BOX, fl_lighter(color()));
			}
			draw_box(FL_UP_BOX,X,Y,W,H,color());

			fl_color(active_r() ? labelcolor() : fl_inactive(labelcolor()));
			fl_polygon(x1, y1, x1 + w1, y1 + w1, x1 + 2 * w1, y1);
		}

		W += 2 * dx;

		if (mvalue()) {
			Fl_Menu_Item m = *mvalue();
			if (active_r()) m.activate();
			else m.deactivate();

			// ERCO
			int xx = x() + dx, yy = y() + dy + 1, ww = w() - W, hh = H - 2;

			fl_push_clip(xx, yy, ww, hh);

			if ( Fl::scheme()) {
				Fl_Label l;
				l.value = m.text;
				l.image = 0;
				l.deimage = 0;
				l.type = m.labeltype_;
				l.font = m.labelsize_ || m.labelfont_ ? m.labelfont_ : textfont();
				l.size = m.labelsize_ ? m.labelsize_ : textsize();
				l.color= m.labelcolor_ ? m.labelcolor_ : textcolor();
				if (!m.active()) l.color = fl_inactive((Fl_Color)l.color);
				fl_draw_shortcut = 2; // hack value to make '&' disappear
				l.draw(xx+3, yy, ww>6 ? ww-6 : 0, hh, FL_ALIGN_LEFT);
				fl_draw_shortcut = 0;
				if ( Fl::focus() == this ) draw_focus(box(), xx, yy, ww, hh);
			} else {
				fl_draw_shortcut = 2; // hack value to make '&' disappear
				m.draw(xx, yy, ww, hh, this, Fl::focus() == this);
				fl_draw_shortcut = 0;
			}

			fl_pop_clip();
		}

		draw_label();
	}

public:
	/**
	Create a new Fl_Choice widget using the given position, size and label string.
	The default boxtype is \c FL_UP_BOX.

	The constructor sets menu() to NULL.
	See Fl_Menu_ for the methods to set or change the menu.

	\param[in] X, Y, W, H position and size of the widget
	\param[in] L widget label, default is no label
	*/
	Fl_Choice(int X, int Y, int W, int H, const char *L = 0) : Fl_Menu_(X,Y,W,H,L) {
		align(FL_ALIGN_LEFT);
		when(FL_WHEN_RELEASE);
		textfont(FL_HELVETICA);
		box(FL_FLAT_BOX);
		down_box(FL_BORDER_BOX);
	}

	/**
	  Gets the index of the last item chosen by the user.
	  The index is zero initially.
	 */
	int value() const {
		return Fl_Menu_::value();
	}

	/**
	Sets the currently selected value using the index into the menu item array.
	Changing the selected value causes a redraw().
	\param[in] v index of value in the menu item array.
	\returns non-zero if the new value is different to the old one.
	*/
	int value(int v) {
		if (v == -1) return value((const Fl_Menu_Item *)0);
		if (v < 0 || v >= (size() - 1)) return 0;
		if (!Fl_Menu_::value(v)) return 0;
		redraw();
		return 1;
	}

	/**
	Sets the currently selected value using a pointer to menu item.
	Changing the selected value causes a redraw().
	\param[in] v pointer to menu item in the menu item array.
	\returns non-zero if the new value is different to the old one.
	*/
	int value(const Fl_Menu_Item* v) {
		if (!Fl_Menu_::value(v)) return 0;
		redraw();
		return 1;
	}

	int handle(int e) {
		if (!menu() || !menu()->text) return 0;
		const Fl_Menu_Item* v;
		switch (e) {
		case FL_ENTER:
		case FL_LEAVE:
			return 1;

		case FL_KEYBOARD:
			if (Fl::event_key() != ' ' ||
			    (Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META))) return 0;
		case FL_PUSH:
			if (Fl::visible_focus()) Fl::focus(this);
J1:
			if (Fl::scheme() || fl_contrast(textcolor(), FL_BACKGROUND2_COLOR) != textcolor()) {
				v = menu()->pulldown(x(), y(), w(), h(), mvalue(), this);
			} else {
				// In order to preserve the old look-n-feel of "white" menus,
				// temporarily override the color() of this widget...
				Fl_Color c = color();
				color(FL_BACKGROUND2_COLOR);
				v = menu()->pulldown(x(), y(), w(), h(), mvalue(), this);
				color(c);
			}
			if (!v || v->submenu()) return 1;
			if (v != mvalue()) redraw();
			picked(v);
			return 1;
		case FL_SHORTCUT:
			if (Fl_Widget::test_shortcut()) goto J1;
			v = menu()->test_shortcut();
			if (!v) return 0;
			if (v != mvalue()) redraw();
			picked(v);
			return 1;
		case FL_FOCUS:
		case FL_UNFOCUS:
			if (Fl::visible_focus()) {
				redraw();
				return 1;
			} else return 0;
		default:
			return 0;
		}
	}
};

#endif

//
// End of "$Id: Fl_Choice.H 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
