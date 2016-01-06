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

#include "Fl_Font.H"
#include <x.H>
#include <string.h>

int fl_font_;
int fl_fontsize_;

static void fl_font_s60(int font, int fontsize)
{
}

void Fl_Android_Graphics_Driver::font(Fl_Font fnum, Fl_Fontsize size)
{
	fl_font_s60(fnum, size);
}

static int fl_height_s60()
{
	// DONE: S60
	return 240;
}

int Fl_Android_Graphics_Driver::height()
{
	return fl_height_s60();
}

static int fl_descent_s60()
{
	return 0;//Fl_X::Font->DescentInPixels();
}

int Fl_Android_Graphics_Driver::descent()
{
	return fl_descent_s60();
}

static double fl_width_s60(char const *str, int len)
{
	return 10.0;
}

double Fl_Android_Graphics_Driver::width(const char* c, int n)
{
	return fl_width_s60(c, n);
}

static double fl_width_s60(unsigned int c)
{
	return 10.0;
}

double Fl_Android_Graphics_Driver::width(unsigned int c)
{
	return fl_width_s60(c);
}

static void fl_text_extents_s60(const char *c, int n, int &dx, int &dy, int &w, int &h)
{
	// TODO: S60
}

void Fl_Android_Graphics_Driver::text_extents(const char *c, int n, int &dx, int &dy, int &w, int &h)
{
	fl_text_extents_s60(c, n, dx, dy, w, h);
}

static void fl_draw_s60(char const *str, int len, int x, int y)
{
}

void Fl_Android_Graphics_Driver::draw(const char* str, int n, int x, int y)
{
	fl_draw_s60(str, n, x, y);
}
