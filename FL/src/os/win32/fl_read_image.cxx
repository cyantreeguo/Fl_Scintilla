//
// "$Id: fl_read_image_win32.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// WIN32 image reading routines for the Fast Light Tool Kit (FLTK).
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

#  include "Fl_RGB_Image.H"
#  include "Fl_Window.H"
#  include "Fl_Plugin.H"
#  include "Fl_Device.H"

static uchar *read_win_rectangle(uchar *p, int X, int Y, int w, int h, int alpha);


static void write_image_inside(Fl_RGB_Image *to, Fl_RGB_Image *from, int to_x, int to_y)
/* Copy the image "from" inside image "to" with its top-left angle at coordinates to_x, to_y.
 Also, exchange top and bottom of "from". Image depth can differ between "to" and "from".
 */
{
  int to_ld = (to->ld() == 0? to->w() * to->d() : to->ld());
  int from_ld = (from->ld() == 0? from->w() * from->d() : from->ld());
  uchar *tobytes = (uchar*)to->array + to_y * to_ld + to_x * to->d();
  const uchar *frombytes = from->array + (from->h() - 1) * from_ld;
  for (int i = from->h() - 1; i >= 0; i--) {
    if (from->d() == to->d()) memcpy(tobytes, frombytes, from->w() * from->d());
    else {
      for (int j = 0; j < from->w(); j++) {
        memcpy(tobytes + j * to->d(), frombytes + j * from->d(), from->d());
      }
    }
    tobytes += to_ld;
    frombytes -= from_ld;
  }
}

/* Captures rectangle x,y,w,h from a mapped window or GL window.
 All sub-GL-windows that intersect x,y,w,h, and their subwindows, are also captured.
 
 Arguments when this function is initially called:
 g: a window or GL window
 p: as in fl_read_image()
 x,y,w,h: a rectangle in window g's coordinates
 alpha: as in fl_read_image()
 full_img: NULL
 
 Arguments when this function recursively calls itself:
 g: an Fl_Group
 p: as above
 x,y,w,h: a rectangle in g's coordinates if g is a window, or in g's parent window coords if g is a group
 alpha: as above
 full_img: NULL, or a previously captured image that encompasses the x,y,w,h rectangle and that
 will be partially overwritten with the new capture
 
 Return value:
 An Fl_RGB_Image* of depth 4 if alpha>0 or 3 if alpha = 0 containing the captured pixels.
 */
static Fl_RGB_Image *traverse_to_gl_subwindows(Fl_Group *g, uchar *p, int x, int y, int w, int h, int alpha,
                                               Fl_RGB_Image *full_img)
{
  if ( g->as_gl_window() ) {
    Fl_Plugin_Manager pm("fltk:device");
    Fl_Device_Plugin *pi = (Fl_Device_Plugin*)pm.plugin("opengl.device.fltk.org");
    if (!pi) return full_img;
    Fl_RGB_Image *img = pi->rectangle_capture(g, x, y, w, h); // bottom to top image
    if (full_img) full_img = img; // top and bottom will be exchanged later
    else { // exchange top and bottom to get a proper FLTK image
      uchar *data = ( p ? p : new uchar[img->w() * img->h() * (alpha?4:3)] );
      full_img = new Fl_RGB_Image(data, img->w(), img->h(), alpha?4:3);
      if (!p) full_img->alloc_array = 1;
      if (alpha) memset(data, alpha, img->w() * img->h() * 4);
      write_image_inside(full_img, img, 0, 0);
      delete img;
    }
  }
  else if ( g->as_window() && (!full_img || (g->window() && g->window()->as_gl_window())) ) {
    // the starting window or one inside a GL window
    if (full_img) g->as_window()->make_current();
    uchar *image_data;
    int alloc_img = (full_img != NULL || p == NULL); // false means use p, don't alloc new memory for image
#ifdef __APPLE_CC__
    // on Darwin + X11, read_win_rectangle() sometimes returns NULL when there are subwindows
    do image_data = read_win_rectangle( (alloc_img ? NULL : p), x, y, w, h, alpha); while (!image_data);
#else
    image_data = read_win_rectangle( (alloc_img ? NULL : p), x, y, w, h, alpha);
#endif
    full_img = new Fl_RGB_Image(image_data, w, h, alpha?4:3);
    if (alloc_img) full_img->alloc_array = 1;
  }
  int n = g->children();
  for (int i = 0; i < n; i++) {
    Fl_Widget *c = g->child(i);
    if ( !c->visible() || !c->as_group()) continue;
    if ( c->as_window() ) {
      int origin_x = x; // compute intersection of x,y,w,h and the c window
      if (x < c->x()) origin_x = c->x();
      int origin_y = y;
      if (y < c->y()) origin_y = c->y();
      int width = c->w();
      if (origin_x + width > c->x() + c->w()) width = c->x() + c->w() - origin_x;
      if (origin_x + width > x + w) width = x + w - origin_x;
      int height = c->w();
      if (origin_y + height > c->y() + c->h()) height = c->y() + c->h() - origin_y;
      if (origin_y + height > y + h) height = y + h - origin_y;
      if (width > 0 && height > 0) {
        Fl_RGB_Image *img = traverse_to_gl_subwindows(c->as_window(), p, origin_x - c->x(),
                                                      origin_y - c->y(), width, height, alpha, full_img);
        if (img == full_img) continue;
        int top;
        if (c->as_gl_window()) {
          top = origin_y - y;
        } else {
          top = full_img->h() - (origin_y - y + img->h());
        }
        write_image_inside(full_img, img, origin_x - x, top);
        delete img;
      }
    }
    else traverse_to_gl_subwindows(c->as_group(), p, x, y, w, h, alpha, full_img);
  }
  return full_img;
}

//
// 'fl_read_image()' - Read an image from the current window or off-screen buffer
// this is the version for X11 and WIN32. The mac version is in fl_read_image_mac.cxx

uchar *				// O - Pixel buffer or NULL if failed
fl_read_image(uchar *p,		// I - Pixel buffer or NULL to allocate
              int   X,		// I - Left position
              int   Y,		// I - Top position
              int   w,		// I - Width of area to read
              // negative allows capture of window title bar and frame (X11 only)
              int   h,		// I - Height of area to read
              int   alpha)// I - Alpha value for image (0 for none)
{
  if (w < 0 || fl_find(fl_window) == 0) { // read from off_screen buffer or title bar and frame
    return read_win_rectangle(p, X, Y, w, h, alpha); // this function has an X11 and a WIN32 version
  }
  Fl_RGB_Image *img = traverse_to_gl_subwindows(Fl_Window::current(), p, X, Y, w, h, alpha, NULL);
  uchar *image_data = (uchar*)img->array;
  img->alloc_array = 0;
  delete img;
  return image_data;
}

static uchar *				// O - Pixel buffer or NULL if failed
read_win_rectangle(uchar *p,		// I - Pixel buffer or NULL to allocate
              int   X,		// I - Left position
              int   Y,		// I - Top position
              int   w,		// I - Width of area to read
              int   h,		// I - Height of area to read
              int   alpha)  	// I - Alpha value for image (0 for none)
{

	int	d;			// Depth of image

	// Allocate the image data array as needed...
	d = alpha ? 4 : 3;

	if (!p) p = new uchar[w * h * d];

	// Initialize the default colors/alpha in the whole image...
	memset(p, alpha, w * h * d);

	// Grab all of the pixels in the image...

	// Assure that we are not trying to read non-existing data. If it is so, the
	// function should still work, but the out-of-bounds part of the image is
	// untouched (initialized with the alpha value or 0 (black), resp.).

	int ww = w; // We need the original width for output data line size

	int shift_x = 0; // X target shift if X modified
	int shift_y = 0; // Y target shift if X modified

	if (X < 0) {
		shift_x = -X;
		w += X;
		X = 0;
	}
	if (Y < 0) {
		shift_y = -Y;
		h += Y;
		Y = 0;
	}

	if (h < 1 || w < 1) return p;		// nothing to copy

	int line_size = ((3*w+3)/4) * 4;	// each line is aligned on a DWORD (4 bytes)
	uchar *dib = new uchar[line_size*h];	// create temporary buffer to read DIB

	// fill in bitmap info for GetDIBits

	BITMAPINFO   bi;
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = w;
	bi.bmiHeader.biHeight = -h;		// negative => top-down DIB
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 24;		// 24 bits RGB
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = 0;
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	// copy bitmap from original DC (Window, Fl_Offscreen, ...)

	HDC hdc = CreateCompatibleDC(fl_gc);
	HBITMAP hbm = CreateCompatibleBitmap(fl_gc,w,h);

	int save_dc = SaveDC(hdc);			// save context for cleanup
	SelectObject(hdc,hbm);			// select bitmap
	BitBlt(hdc,0,0,w,h,fl_gc,X,Y,SRCCOPY);	// copy image section to DDB

	// copy RGB image data to the allocated DIB

	GetDIBits(hdc, hbm, 0, h, dib, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

	// finally copy the image data to the user buffer

	for (int j = 0; j<h; j++) {
		const uchar *src = dib + j * line_size;			// source line
		uchar *tg = p + (j + shift_y) * d * ww + shift_x * d;	// target line
		for (int i = 0; i<w; i++) {
			uchar b = *src++;
			uchar g = *src++;
			*tg++ = *src++;	// R
			*tg++ = g;	// G
			*tg++ = b;	// B
			if (alpha)
				*tg++ = alpha;	// alpha
		}
	}

	// free used GDI and other structures

	RestoreDC(hdc,save_dc);	// reset DC
	DeleteDC(hdc);
	DeleteObject(hbm);
	delete[] dib;		// delete DIB temporary buffer

	return p;
}

//
// End of "$Id: fl_read_image_win32.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
