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
#include <utf.h>
#include <string.h>

int fl_font_;
int fl_fontsize_;

static void fl_font_s60(int font, int fontsize)
{
	// TODO: S60
	fl_font_ = font;
	fl_fontsize_ = fontsize;
	if (fl_fonts != NULL && Fl_X::WsScreenDevice != NULL) {
		int fontattr = font % 4;
		font /= 4;
		TBuf16<128> fontname_;
		// TInt err;
		CnvUtfConverter::ConvertToUnicodeFromUtf8(fontname_, TPtr8((unsigned char*) fl_fonts[font].fontname, strlen(fl_fonts[font].fontname)));
		TFontSpec fontSpec(fontname_, fontsize);
		TFontStyle fontStyle(
		        (fontattr == FL_ITALIC || fontattr == FL_BOLD_ITALIC) ? EPostureItalic : EPostureUpright,
		        (fontattr == FL_BOLD || fontattr == FL_BOLD_ITALIC) ? EStrokeWeightBold : EStrokeWeightNormal,
		        EPrintPosNormal);
		fontSpec.iFontStyle = fontStyle;
		Fl_X::WsScreenDevice->GetNearestFontInPixels(Fl_X::Font, fontSpec);
		/* if (err != KErrNone)
			{
			fontSpec.iFontStyle=TFontStyle();
			} */
	}
	if (fl_window != NULL) {
		Fl_X::WindowGc->UseFont(Fl_X::Font);
	}
}

void Fl_Gc_Graphics_Driver::font(Fl_Font fnum, Fl_Fontsize size)
{
	fl_font_s60(fnum, size);
}

static int fl_height_s60()
{
	// DONE: S60
	return Fl_X::Font->HeightInPixels();
}

int Fl_Gc_Graphics_Driver::height()
{
	return fl_height_s60();
}

static int fl_descent_s60()
{
	return Fl_X::Font->DescentInPixels();
}

int Fl_Gc_Graphics_Driver::descent()
{
	return fl_descent_s60();
}

static double fl_width_s60(char const *str, int len)
{
	// DONE: S60
	// if (len == 0) len = strlen(str);
	TPtrC8 ptr8 ((const unsigned char*) str, len);
	HBufC *buf = NULL;
	TRAPD(error, buf = CnvUtfConverter::ConvertToUnicodeFromUtf8L(ptr8));
	double w = 0;
	if (buf) {
		w = Fl_X::Font->TextWidthInPixels(buf->Des());
		delete buf;
	}
	return w;
}

double Fl_Gc_Graphics_Driver::width(const char* c, int n)
{
	return fl_width_s60(c, n);
}

static double fl_width_s60(unsigned int c)
{
	unsigned short c16 = c;
	TPtrC ptr16 (&c16, 1);
	return Fl_X::Font->TextWidthInPixels(ptr16);
}

double Fl_Gc_Graphics_Driver::width(unsigned int c)
{
	return fl_width_s60(c);
}

static void fl_text_extents_s60(const char *c, int n, int &dx, int &dy, int &w, int &h)
{
	// TODO: S60
}

void Fl_Gc_Graphics_Driver::text_extents(const char *c, int n, int &dx, int &dy, int &w, int &h)
{
	fl_text_extents_s60(c, n, dx, dy, w, h);
}

static void fl_draw_s60(char const *str, int len, int x, int y)
{
	// DONE: S60
	// if (len == 0) len = strlen(str);
	TPtrC8 ptr8 ((const unsigned char*) str, len);
	HBufC *buf = NULL;
	TRAPD(error, buf = CnvUtfConverter::ConvertToUnicodeFromUtf8L(ptr8));
	if (buf) {
		Fl_X::WindowGc->DrawText (buf->Des(), TPoint(x,y));
		delete buf;
	}
}

void Fl_Gc_Graphics_Driver::draw(const char* str, int n, int x, int y)
{
	fl_draw_s60(str, n, x, y);
}
