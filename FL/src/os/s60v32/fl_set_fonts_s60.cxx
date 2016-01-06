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

#include "x.H"
#include "Fl.H"
#include "Fl_Font.H"
#include <utf.h>

const char* Fl::get_font_name(Fl_Font fnum, int* ap)
{
	return &(fl_fonts[fnum].fontname[0]);
}

Fl_Font Fl::set_fonts(const char* xstarname)
	{
	if (fl_fonts)
		{
		free(fl_fonts);
		fl_fonts = NULL;
		}
	fl_fonts = (Fl_Fontdesc*) malloc(sizeof(Fl_Fontdesc) * Fl_X::WsScreenDevice->NumTypefaces());
	for (int i = 0; i < Fl_X::WsScreenDevice->NumTypefaces(); i++)
		{
		TTypefaceSupport typefaceSupport;
		Fl_X::WsScreenDevice->TypefaceSupport(typefaceSupport, i);
		unsigned char fontname[KMaxTypefaceNameLength];
		TPtr8 fontname_(fontname, KMaxTypefaceNameLength);
		CnvUtfConverter::ConvertFromUnicodeToUtf8(fontname_, typefaceSupport.iTypeface.iName.Des());
		fl_fonts[i].name = &(fl_fonts[i].fontname[0]);
		strncpy(fl_fonts[i].fontname, (char*) fontname, 127);
		}
	return Fl_X::WsScreenDevice->NumTypefaces() * 4;
	}

int Fl::get_font_sizes(Fl_Font fnum, int*& sizep)
	{
	
	}

Fl_Font_Descriptor::Fl_Font_Descriptor(const char* xfontname)
	{
	// TODO: S60
	}

Fl_Font_Descriptor::~Fl_Font_Descriptor()
	{
	// TODO: S60
	}

// static Fl_Fontdesc built_in_table[] = {
		// TODO: S60
		/* {" Arial"},
		{"BArial"},
		{"IArial"},
		{"PArial"},
		{" Courier New"},
		{"BCourier New"},
		{"ICourier New"},
		{"PCourier New"},
		{" Times New Roman"},
		{"BTimes New Roman"},
		{"ITimes New Roman"},
		{"PTimes New Roman"},
		{" Symbol"},
		{" Terminal"},
		{"BTerminal"},
		{" Wingdings"}, */
// };

Fl_Fontdesc *fl_fonts = NULL; // TODO: S60
