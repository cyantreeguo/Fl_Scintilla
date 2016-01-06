//
// "$Id: Fl_PNG_Image.H 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// PNG image header file for the Fast Light Tool Kit (FLTK).
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
   Fl_PNG_Image class . */

#ifndef Fl_PNG_Image_H
#define Fl_PNG_Image_H

#include "Fl.H"
#include "Fl_Image.H"
#include "Fl_Shared_Image.H"
#include "fltk_config.h"
#include <stdio.h>
#include <stdlib.h>
#include "fl_utf8.h"

#include "lodepng.h"

/**
  The Fl_PNG_Image class supports loading, caching,
  and drawing of Portable Network Graphics (PNG) image files. The
  class loads colormapped and full-color images and handles color-
  and alpha-based transparency.
*/
class FL_EXPORT Fl_PNG_Image : public Fl_RGB_Image
{
public:
	Fl_PNG_Image(const char* filename) : Fl_RGB_Image(0,0,0) {
		FILE *fp; // File pointer
		int size;
		unsigned char *buf;

		if ((fp = fl_fopen(filename, "rb")) == NULL) return;
		fseek(fp, 0, SEEK_END);
		size = (int) ftell(fp);
		if ( size <= 0 ) {
			fclose(fp);
			return;
		}
		buf = (unsigned char*)malloc(size);
		fseek(fp, 0, SEEK_SET);
		size = (int) fread(buf, 1, size, fp);
		fclose(fp);

		unsigned error;
		unsigned char* image;
		unsigned int width, height;

		error = lodepng_decode32(&image, &width, &height, buf, size);
		if(error) {
			free(buf);
			return;
		}

		w(width);
		h(height);
		d(4);

		array = new uchar[w() * h() * d()];
		alloc_array = 1;
		unsigned char *ptr = (unsigned char *)array;
		memcpy(ptr, image, w() * h() * d());

		free(image);
		free(buf);
	}

	Fl_PNG_Image (const char *name_png, const unsigned char *buffer, int datasize) : Fl_RGB_Image(0,0,0) {
		unsigned error;
		unsigned char* image;
		unsigned int width, height;

		error = lodepng_decode32(&image, &width, &height, buffer, datasize);
		if(error) return;

		w(width);
		h(height);
		d(4);

		array = new uchar[w() * h() * d()];
		alloc_array = 1;
		unsigned char *ptr = (unsigned char *)array;
		memcpy(ptr, image, w() * h() * d());

		free(image);

		if (w() && h() && name_png) {
			Fl_Shared_Image *si = new Fl_Shared_Image(name_png, this);
			si->add();
		}
	}
};

#endif

//
// End of "$Id: Fl_PNG_Image.H 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
