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

#if __FLTK_S60v32__

#ifndef __S60_H__
#define __S60_H__

#include <e32std.h>
#include <w32std.h>
#include <fbs.h>
#include <gdi.h>
#include <unistd.h>

typedef RWindow *Window;
typedef TRegion *Fl_Region;
typedef struct Fl_Offscreen_ {
	CFbsBitmap *bmp;
	CFbsBitmapDevice *bmpDev;
	CFbsBitGc *bmpGc;
	Fl_Offscreen_(int w, int h) {
		// TODO: S60
		bmp = new (ELeave) CFbsBitmap;
		bmp->Create(TSize(w, h), EColor16MA);
		TRAPD(error, bmpDev = CFbsBitmapDevice::NewL(bmp));
		bmpGc = CFbsBitGc::NewL();
		bmpGc->Activate (bmpDev);
	}
	~Fl_Offscreen_() {
		// TODO: S60
		delete bmpGc;
		delete bmpDev;
		delete bmp;
	}
} *Fl_Offscreen;
typedef Fl_Offscreen Fl_Bitmask;
typedef struct {
	int x, y;
} XPoint;

#include "Fl_Window.H"

class CRedrawEventActive: public CActive
{
public:
	CRedrawEventActive(): CActive(CActive::EPriorityStandard) {
		CActiveScheduler::Add(this);
	}
	~CRedrawEventActive() {
		Cancel();
	}
	void Start();
private:
	void DoCancel();
	void RunL();
};

class CWsEventActive: public CActive
{
public:
	CWsEventActive(): CActive(CActive::EPriorityStandard) {
		CActiveScheduler::Add(this);
	}
	~CWsEventActive() {
		Cancel();
	}
	void Start();
private:
	void DoCancel();
	void RunL();
};

class CWaitTimeoutActive: public CActive
{
private:
	RTimer iTimer;
public:
	CWaitTimeoutActive(): CActive(CActive::EPriorityStandard) {
		iTimer.CreateLocal();
		CActiveScheduler::Add(this);
	}
	~CWaitTimeoutActive() {
		Cancel();
		iTimer.Close();
	}
	void Start(TTimeIntervalMicroSeconds32 anInterval);
private:
	void DoCancel();
	void RunL();
};

class FL_EXPORT Fl_X
{
public:
	// member variables - add new variables only at the end of this block
	RWindowGroup *windowGroup;
	Window xid;
	Fl_Offscreen other_xid; // for double-buffered windows
	Fl_Window* w, *w2;
	Fl_Region region;
	Fl_X *next;
	int wait_for_expose;
	// HDC private_dc; // used for OpenGL
	// HCURSOR cursor;
	// HDC saved_hdc;  // saves the handle of the DC currently loaded
	// static variables, static functions and member functions
	static Fl_X* first;
	static Fl_X* i(const Fl_Window* w) {
		return w->i;
	}
	/* static int fake_X_wm(const Fl_Window* w,int &X, int &Y,
		                 int &bt,int &bx,int &by); */
	void setwindow(Fl_Window* wi) {
		w=wi;
		wi->i=this;
	}
	void flush() {
		w->flush();
	}
	// void set_minmax(LPMINMAXINFO minmax);
	// void mapraise();
	static Fl_X* make(Fl_Window*);
	static int WsSessionReady;
	static RWsSession WsSession;
	static CWsScreenDevice *WsScreenDevice;
	static RWindowGroup *WindowGroup;
	static CBitmapContext *WindowGc;
	static RTimer Timer;
	static CTrapCleanup *TrapCleanup;
	static RWsPointerCursor *WsPointerCursor;
	static CFont *Font;
	static RFs Fs;
	static RFbsSession FbsSession;
	static CBitmapContext *SavedGc;
	static CFbsBitGc *BitmapGc;
	static CRedrawEventActive *RedrawEventActive;
	static CWsEventActive *WsEventActive;
	static CWaitTimeoutActive *WaitTimeoutActive;

	void set_icons() {}
	static void set_default_icons(const Fl_RGB_Image*[], int) {}
};

inline Window fl_xid(const Fl_Window*w)
{
	Fl_X *temp = Fl_X::i(w);
	return temp ? temp->xid : 0;
}

extern FL_EXPORT void* fl_display;
extern FL_EXPORT Window fl_window;

inline Fl_Offscreen s60_create_offscreen(int w, int h)
{
	// TODO: S60
	// CFbsBitmap *b = new (ELeave) CFbsBitmap();
	// b->Create(TSize(w, h), EColor64K);
	Fl_Offscreen offscr = new Fl_Offscreen_(w, h);
	return offscr;
}

#define fl_create_offscreen(w, h) s60_create_offscreen(w, h)
FL_EXPORT void fl_copy_offscreen(int x,int y,int w,int h,Fl_Offscreen pixmap,int srcx,int srcy);
FL_EXPORT void fl_copy_offscreen_with_alpha(int x,int y,int w,int h,Fl_Offscreen pixmap,int srcx,int srcy);
#define fl_delete_offscreen(offscr) { delete (offscr); }

void fl_begin_offscreen(Fl_Offscreen);
void fl_end_offscreen();

// TODO: S60
/*# define fl_begin_offscreen(b) \
	Fl_X::SavedGc = Fl_X::WindowGc; \
	Fl_X::WindowGc = Fl_X::BitmapGc; \
	Fl_X::BitmapGc->Activate((b)->bmpDev); \
	fl_push_no_clip(); */
/* HDC _sgc=fl_gc; Window _sw=fl_window; \
fl_gc=fl_makeDC(b); int _savedc = SaveDC(fl_gc); fl_window=(HWND)b; fl_push_no_clip() */

// TODO: S60
/*# define fl_end_offscreen() \
	fl_pop_clip(); \
	Fl_X::WindowGc = Fl_X::SavedGc; */
// fl_pop_clip(); RestoreDC(fl_gc, _savedc); DeleteDC(fl_gc); fl_window=_sw; fl_gc = _sgc

extern FL_EXPORT Fl_Bitmask fl_create_bitmask(int w, int h, const uchar *data);
extern FL_EXPORT Fl_Bitmask fl_create_alphamask(int w, int h, int d, int ld, const uchar *data);
extern FL_EXPORT void fl_delete_bitmask(Fl_Bitmask bm);

FL_EXPORT void fl_clip_region(Fl_Region);
inline Fl_Region XRectangleRegion(int x, int y, int w, int h)
{
	// return CreateRectRgn(x,y,x+w,y+h);
	// DONE: S60
	TRegionFix<1> *r = new (ELeave) TRegionFix<1>(TRect(x, y, x+w, y+h));
	return r;
}
FL_EXPORT void XDestroyRegion(Fl_Region r);

FL_EXPORT CWindowGc *fl_GetDC(Window w);

extern FL_EXPORT int fl_parse_color(const char* p, uchar& r, uchar& g, uchar& b);

#endif // __S60_H__

#endif // __FLTK_S60v32__
