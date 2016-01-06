// FLTK Symbian port Copyright 2009 by Sadysta.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.

#include "Fl.H"
#include "x.H"

extern char fl_key_vector[32]; // in Fl_x.cxx

int Fl::event_key(int k) {
  // TODO: S60
/*  if (k > FL_Button && k <= FL_Button+8)
    return Fl::event_state(8<<(k-FL_Button));
  int i;
#  ifdef __sgi
  // get some missing PC keyboard keys:
  if (k == FL_Meta_L) i = 147;
  else if (k == FL_Meta_R) i = 148;
  else if (k == FL_Menu) i = 149;
  else
#  endif
    i = XKeysymToKeycode(fl_display, k);
  if (i==0) return 0;
  return fl_key_vector[i/8] & (1 << (i%8)); */
}

int Fl::get_key(int k) {
  // TODO: S60
  /* fl_open_display();
  XQueryKeymap(fl_display, fl_key_vector);
  return event_key(k); */
}
