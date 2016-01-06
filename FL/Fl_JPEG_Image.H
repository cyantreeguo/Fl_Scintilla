//
// "$Id: Fl_JPEG_Image.H 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// JPEG image header file for the Fast Light Tool Kit (FLTK).
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
   Fl_JPEG_Image class . */

#ifndef Fl_JPEG_Image_H
#define Fl_JPEG_Image_H

#include "Fl_Image.H"
#include "Fl_Shared_Image.H"
#include "fl_utf8.h"
#include "Fl.H"
#include "fltk_config.h"
#include <stdio.h>
#include <stdlib.h>

#include "jpgd.h"

/**
  The Fl_JPEG_Image class supports loading, caching,
  and drawing of Joint Photographic Experts Group (JPEG) File
  Interchange Format (JFIF) images. The class supports grayscale
  and color (RGB) JPEG image files.
*/
class FL_EXPORT Fl_JPEG_Image : public Fl_RGB_Image
{
public:
	Fl_JPEG_Image(const char *filename) : Fl_RGB_Image(0,0,0) {
		FILE		*fp;		// File pointer
		int size;
		unsigned char *buf;
		unsigned char *p, *ptr;	// Pointer to pixel values

		if ((fp = fl_fopen(filename, "rb")) == NULL) {
			ld(ERR_FILE_ACCESS);
			return;
		}
		fseek(fp, 0, SEEK_END);
		size = (int) ftell(fp);
		if ( size <= 0 ) {
			fclose(fp);
			ld(ERR_FORMAT);
			return;
		}
		buf = (unsigned char*)malloc(size);
		fseek(fp, 0, SEEK_SET);
		size = (int) fread(buf, 1, size, fp);
		fclose(fp);

		/*
		// Loads a JPEG image from a memory buffer.
		// req_comps can be 1 (grayscale), 3 (RGB), or 4 (RGBA).
		// On return, width/height will be set to the image's dimensions, and actual_comps will be set
		// to either 1 (grayscale) or 3 (RGB).
		unsigned char *decompress_jpeg_image_from_memory(const unsigned char *pSrc_data, int src_data_size,
			int *width, int *height, int *actual_comps, int req_comps);
		*/
		int width, height, ac;
		p = jpgd::decompress_jpeg_image_from_memory(buf, size, &width, &height, &ac, 3);
		if ( p == NULL ) {
			free(buf);
			return;
		}

		w(width);
		h(height);
		d(ac);
		array = new uchar[w() * h() * d()];
		alloc_array = 1;
		ptr = (unsigned char *)array;
		memcpy(ptr, p, w() * h() * d());

		free(p);
		free(buf);
	}

	/**
	 \brief The constructor loads the JPEG image from memory.

	 Construct an image from a block of memory inside the application. Fluid offers
	 "binary Data" chunks as a great way to add image data into the C++ source code.
	 name_png can be NULL. If a name is given, the image is added to the list of
	 shared images (see: fltk3::SharedImage) and will be available by that name.

	 The inherited destructor frees all memory and server resources that are used
	 by the image.

	 There is no error function in this class. If the image has loaded correctly,
	 w(), h(), and d() should return values greater zero.

	 \param name A unique name or NULL
	 \param data A pointer to the memory location of the JPEG image
	 */
	Fl_JPEG_Image(const char *name, const unsigned char *data, int size) : Fl_RGB_Image(0,0,0) {
		unsigned char *p, *ptr;	// Pointer to pixel values
		int width, height, ac;
		p = jpgd::decompress_jpeg_image_from_memory(data, size, &width, &height, &ac, 3);
		if ( p == NULL ) return;

		w(width);
		h(height);
		d(ac);
		array = new uchar[w() * h() * d()];
		alloc_array = 1;
		ptr = (unsigned char *)array;
		memcpy(ptr, p, w() * h() * d());

		free(p);

		if (w() && h() && name) {
			Fl_Shared_Image *si = new Fl_Shared_Image(name, this);
			si->add();
		}
	}
};

#endif

//
// End of "$Id: Fl_JPEG_Image.H 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
