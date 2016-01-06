#if __FLTK_ANDROID__

#include "Fl_Window.H"

#include <jni.h>

typedef void* Fl_Region;
typedef void* Window;
struct XPoint {
	int x, y;
};

typedef void* Fl_Offscreen;
extern FL_EXPORT void* fl_display;

FL_EXPORT void fl_open_display();
FL_EXPORT void fl_close_display();

inline Fl_Region XRectangleRegion(int x, int y, int w, int h)
{
	/*
	Fl_Region R = (Fl_Region)malloc(sizeof(*R));
	R->count = 1;
	R->rects = (CGRect *)malloc(sizeof(CGRect));
	*(R->rects) = fl_cgrectmake_cocoa(x, y, w, h);
	return R;
	*/
	return NULL;
}
inline void XDestroyRegion(Fl_Region r)
{
	if(r) {
		//free(r->rects);
		//free(r);
	}
}

class FL_EXPORT Fl_X
{
public:
	Window xid;
	Fl_Offscreen other_xid;
	Fl_Window *w;
	Fl_Region region;
	Fl_Region subRegion; 
	Fl_X *next;
	char wait_for_expose;
	char backbuffer_bad; // used for XDBE
	static Fl_X* first;
	static Fl_X* i(const Fl_Window* wi) {
		return wi->i;
	}
	void setwindow(Fl_Window* wi) {
		w=wi;
		wi->i=this;
	}
	void sendxjunk();
	static void set_default_icons(const Fl_RGB_Image*[], int){}
	void set_icons(){}
	int set_cursor(Fl_Cursor);
	int set_cursor(const Fl_RGB_Image*, int, int);
	// kludges to get around protection:
	void flush() {
		w->flush();
	}
	static void x(Fl_Window* wi, int X) {
		wi->x(X);
	}
	static void y(Fl_Window* wi, int Y) {
		wi->y(Y);
	}
	static int ewmh_supported();
	static int xrender_supported();

	static Fl_X* make(Fl_Window*);
};

inline Window fl_xid(const Fl_Window*w)
{
	Fl_X *temp = Fl_X::i(w);
	return temp ? temp->xid : 0;
}

typedef void* Fl_Offscreen;
typedef void* Fl_Bitmask;

inline Fl_Offscreen fl_create_offscreen(int w, int h)
{
	return 0;
}
FL_EXPORT void fl_copy_offscreen(int x,int y,int w,int h,Fl_Offscreen pixmap,int srcx,int srcy);
FL_EXPORT void fl_copy_offscreen_with_alpha(int x,int y,int w,int h,Fl_Offscreen pixmap,int srcx,int srcy);

inline void fl_delete_offscreen(Fl_Offscreen offscr)
{

}

inline void fl_begin_offscreen(Fl_Offscreen ctx)
{

}

inline void fl_end_offscreen()
{

}


FL_EXPORT void fl_copy_offscreen(int x,int y,int w,int h,void *pixmap,int srcx,int srcy);

int fl_parse_color(const char* p, uchar& r, uchar& g, uchar& b);

extern FL_EXPORT Fl_Bitmask fl_create_bitmask(int w, int h, const uchar *data);
extern FL_EXPORT Fl_Bitmask fl_create_alphamask(int w, int h, int d, int ld, const uchar *data);
extern FL_EXPORT void fl_delete_bitmask(Fl_Bitmask bm);

inline void fl_open_display()
{

}

#endif
