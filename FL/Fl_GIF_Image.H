//
// "$Id: Fl_GIF_Image.H 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// GIF image header file for the Fast Light Tool Kit (FLTK).
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
   Fl_GIF_Image widget . */

#ifndef Fl_GIF_Image_H
#define Fl_GIF_Image_H

#include "Fl.H"
#include "Fl_Pixmap.H"
#include "Fl_Shared_Image.H"
#include <stdio.h>
#include <stdlib.h>
#include "fl_utf8.h"
#include "src/flstring.h"

#include "libnsgif.h"

/**
  The Fl_GIF_Image class supports loading, caching,
  and drawing of Compuserve GIF<SUP>SM</SUP> images. The class
  loads the first image and supports transparency.
*/
class FL_EXPORT Fl_GIF_Image : public Fl_RGB_Image
{
private:
	typedef struct {
		int width, height, depth;
		unsigned int delay_ms;
		int size;
		unsigned char *data;
	} GIFFRAME;
	GIFFRAME *frame_;
	int count_;
	int current_;

protected:
	static void *bitmap_create(int width, int height) {
		void *p = malloc(width * height * 4);
		memset(p, 0, width*height*4);
		return p;

		// return calloc(width * height, 4);
	}

	static void bitmap_set_opaque(void *bitmap, unsigned char/*bool*/ opaque) {
		//(void) opaque;  /* unused */
		//assert(bitmap);
	}

	static unsigned char/*bool*/ bitmap_test_opaque(void *bitmap) {
		//assert(bitmap);
		return NSGIF_FALSE;
	}

	static unsigned char *bitmap_get_buffer(void *bitmap) {
		//assert(bitmap);
		return (unsigned char *)bitmap;
	}

	static void bitmap_destroy(void *bitmap) {
		//assert(bitmap);
		if ( bitmap != NULL ) free(bitmap);
	}

	static void bitmap_modified(void *bitmap) {
		//assert(bitmap);
	}

protected:
	void FrameClean() {
		if ( count_ == 0 ) return;
		int i;
		for (i=0; i<(int)count_; i++) {
			if ( frame_[i].data != NULL ) {
				free(frame_[i].data);
				frame_[i].data = NULL;
			}
		}
		free(frame_);
		count_ = 0;
	}

public:
	Fl_GIF_Image(const char* filename) : Fl_RGB_Image(0, 0, 0) {
		count_ = 0;
		current_ = 0;

		FILE *fp;
		int size;
		unsigned char *data;

		if ((fp = fl_fopen(filename, "rb")) == NULL) return;
		fseek(fp, 0, SEEK_END);
		size = (int) ftell(fp);
		if ( size <= 0 ) {
			fclose(fp);
			return;
		}
		data = (unsigned char*)malloc(size);
		fseek(fp, 0, SEEK_SET);
		size = (int) fread(data, 1, size, fp);
		fclose(fp);

		gif_animation gif;
		gif_bitmap_callback_vt bitmap_callbacks = {
			Fl_GIF_Image::bitmap_create,
			Fl_GIF_Image::bitmap_destroy,
			Fl_GIF_Image::bitmap_get_buffer,
			Fl_GIF_Image::bitmap_set_opaque,
			Fl_GIF_Image::bitmap_test_opaque,
			Fl_GIF_Image::bitmap_modified
		};
		gif_result code;
		int i;

		gif_create(&gif, &bitmap_callbacks);
		do {
			code = gif_initialise(&gif, size, data);
			if (code != GIF_OK && code != GIF_WORKING) {
				gif_finalise(&gif);
				free(data);
				return;
			}
		} while (code != GIF_OK);

		if ( gif.frame_count == 0 ) {
			gif_finalise(&gif);
			free(data);
			return;
		}
		count_ = gif.frame_count;
		frame_ = (GIFFRAME *)malloc(sizeof(GIFFRAME) * count_);
		for (i=0; i<count_; i++) frame_[i].data = NULL;

		/* decode the frames */
		for (i = 0; i != gif.frame_count; i++) {
			code = gif_decode_frame(&gif, i);
			if (code != GIF_OK) {
				//warning("gif_decode_frame", code);
				FrameClean();
				gif_finalise(&gif);
				free(data);
				return;
			}
			frame_[i].size = gif.width * gif.height * 4;
			frame_[i].width = gif.width;
			frame_[i].height = gif.height;
			frame_[i].depth = 4;
			frame_[i].delay_ms = gif.frames->frame_delay*10;
			//if ( frame_[i].delay >= 100 ) frame_[i].delay = 18;
			frame_[i].data = (unsigned char *)malloc(frame_[i].size);
			memcpy(frame_[i].data, (unsigned char *)gif.frame_image, frame_[i].size);
			//printf("%s, loop count=%d, delay=%d\n", filename, gif.loop_count, frame_[i].delay);
		}

		gif_finalise(&gif);
		free(data);

		select_frame(current_);
	}

	Fl_GIF_Image(const char *name, const unsigned char *data, int size) : Fl_RGB_Image(0,0,0) {
		count_ = 0;
		current_ = 0;

		gif_animation gif;
		gif_bitmap_callback_vt bitmap_callbacks = {
			Fl_GIF_Image::bitmap_create,
			Fl_GIF_Image::bitmap_destroy,
			Fl_GIF_Image::bitmap_get_buffer,
			Fl_GIF_Image::bitmap_set_opaque,
			Fl_GIF_Image::bitmap_test_opaque,
			Fl_GIF_Image::bitmap_modified
		};
		gif_result code;
		int i;

		unsigned char *p = (unsigned char *)data;
		gif_create(&gif, &bitmap_callbacks);
		do {
			code = gif_initialise(&gif, size, p);
			if (code != GIF_OK && code != GIF_WORKING) {
				gif_finalise(&gif);
				return;
			}
		} while (code != GIF_OK);

		if ( gif.frame_count == 0 ) {
			gif_finalise(&gif);
			return;
		}
		count_ = gif.frame_count;
		frame_ = (GIFFRAME *)malloc(sizeof(GIFFRAME) * count_);
		for (i=0; i<count_; i++) frame_[i].data = NULL;

		/* decode the frames */
		for (i = 0; i != gif.frame_count; i++) {
			code = gif_decode_frame(&gif, i);
			if (code != GIF_OK) {
				//warning("gif_decode_frame", code);
				FrameClean();
				gif_finalise(&gif);
				return;
			}
			frame_[i].size = gif.width * gif.height * 4;
			frame_[i].width = gif.width;
			frame_[i].height = gif.height;
			frame_[i].depth = 4;
			frame_[i].delay_ms = gif.frames->frame_delay*10;
			//if ( frame_[i].delay >= 100 ) frame_[i].delay = 18;
			frame_[i].data = (unsigned char *)malloc(frame_[i].size);
			memcpy(frame_[i].data, (unsigned char *)gif.frame_image, frame_[i].size);
			//printf("%s, loop count=%d, delay=%d\n", filename, gif.loop_count, frame_[i].delay);
		}

		gif_finalise(&gif);

		select_frame(current_);

		if (w() && h() && name) {
			Fl_Shared_Image *si = new Fl_Shared_Image(name, this);
			si->add();
		}
	}

	~Fl_GIF_Image() {
		FrameClean();
	}

	void select_frame(int index) { /* Used to select a given frame number. */
		if ( index < 0 || index >= count_ ) return;
		uncache();
		w(frame_[index].width);
		h(frame_[index].height);
		d(frame_[index].depth);
		array = frame_[index].data;
		alloc_array = 0;
		current_ = index;
	}

	inline int framecount() {
		return count_;
	}
	inline int framecurrent() {
		return current_;
	}
	int framedelay_ms(int index) {
		if ( index < 0 || index >= count_ ) return 0;
		return frame_[index].delay_ms;
	}
};

#endif

//
// End of "$Id: Fl_GIF_Image.H 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
