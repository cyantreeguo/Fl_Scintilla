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

#include <FL/fl_draw.H>
#include <FL/x.H>

static void fl_draw_image_innards(const uchar *buf, int x, int y, int w, int h, int delta, int linedelta, int mono, Fl_Draw_Image_Cb cb, void *userdata)
	{
	// TODO: S60
	if (linedelta == 0) linedelta = w * delta;
	const void *array = buf;
	uchar *tmpBuf = 0;
	if (cb)
		{
	    tmpBuf = new uchar[ h*w*delta ];
	    for (int i=0; i<h; i++)
	    	{
	    	cb(userdata, 0, i, w, tmpBuf+i*w*delta);
	    	}
	    array = (void*)tmpBuf;
	    linedelta = w*delta;
		}
	CFbsBitmap *bmp = new (ELeave) CFbsBitmap;
	bmp->Create (TSize(w, h), mono ? EGray2 : delta <= 2 ? EGray256 : delta == 3 ? EColor16M : EColor16MA);
	// TODO: Handle error(s)
	TRAPD(error,
		bmp->LockHeapLC(ETrue);
		// uchar *sysbuf = (uchar*) bmp->DataAddress();
		uchar *bmpData = (uchar*) bmp->DataAddress();
		TInt bmpLineDelta;
		bmpLineDelta = bmp->ScanLineLength(w, bmp->DisplayMode());
		if (delta == 4)
			{
			for (int iy = 0; iy < h; iy++)
				{
				for (int ix = 0; ix < w; ix++)
					{
					bmpData[0] = buf[2];
					bmpData[1] = buf[1];
					bmpData[2] = buf[0];
					bmpData[3] = buf[3];
					buf += delta;
					bmpData += delta;
					}
				bmpData = bmpData - w * delta + bmpLineDelta;
				buf = buf - w * delta + linedelta;
				}
			} else
				if (delta == 3)
					{
					for (int iy = 0; iy < h; iy++)
						{
						//TPtr8 ptr((uchar*) buf + iy * linedelta, linedelta);
						//bmp->SetScanLine(ptr, iy);
						for (int ix = 0; ix < w; ix++)
							{
							bmpData[0] = buf[2];
							bmpData[1] = buf[1];
							bmpData[2] = buf[0];
							bmpData[3] = 0xFF;
							buf += delta;
							bmpData += delta;
							}
						// memcpy (bmpData, buf, linedelta);
						// bmpData += bmpLineDelta;
						// buf += linedelta;
						bmpData = bmpData - w * delta + bmpLineDelta;
						buf = buf - w * delta + linedelta;
						}
					} else
						if (delta == 2 || delta == 1)
							{
							for (int iy = 0; iy < h; iy++)
								{
								memcpy (bmpData, buf, linedelta);
								bmpData += bmpLineDelta;
								buf += linedelta;
								}
							}
		CleanupStack::PopAndDestroy(); // LockHeapLC()
		); // TRAPD
	Fl_X::WindowGc->BitBlt (TPoint(x, y), bmp);
	delete bmp;
	if (cb)
		{
		delete [] tmpBuf;
		}
	// bmp->UnlockHeap(ETrue);
	
	}

void Fl_Gc_Graphics_Driver::draw_image(const uchar* buf, int x, int y, int w, int h, int d, int l)
	{
	// DONE: S60
	fl_draw_image_innards(buf,x,y,w,h,d,l,(d<3&&d>-3),0,0);
	}

void Fl_Gc_Graphics_Driver::draw_image(Fl_Draw_Image_Cb cb, void* data, int x, int y, int w, int h, int d)
	{
	// DONE: S60
	fl_draw_image_innards(0,x,y,w,h,d,0,(d<3&&d>-3),cb,data);
	}

void Fl_Gc_Graphics_Driver::draw_image_mono(const uchar* buf, int x, int y, int w, int h, int d, int l)
	{
	// DONE: S60
	fl_draw_image_innards(buf,x,y,w,h,d,l,1,0,0);
	}

void Fl_Gc_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int x, int y, int w, int h,int d)
	{
	// DONE: S60
	fl_draw_image_innards(0,x,y,w,h,d,0,1,cb,data);
	}

void fl_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b)
	{
	// DONE: S60
	fl_color((r << 24) | (g << 16) | (b << 8));
	fl_rectf (x, y, w, h);
	}
