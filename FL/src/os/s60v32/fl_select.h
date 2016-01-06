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

#ifndef _FL_SELECT_S60_H_
#define _FL_SELECT_S60_H_

#include <sys/select.h>

void fl_select_s60_start_thread();
void fl_select_s60_stop_thread();
int fl_select_s60(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *t);

#endif // _FL_SELECT_S60_H_
