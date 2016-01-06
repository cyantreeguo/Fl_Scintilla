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

// TODO: S60

#include "Fl_Widget.H"
#include "Fl_Window.H"
#include "Fl_Group.H"
#include "Fl.H"
#include "x.H"
#include "fl_draw.H"
#include <utf.h>
#include <estlib.h>
#include "fl_select.h"

CTrapCleanup* Fl_X::TrapCleanup;
int Fl_X::WsSessionReady;
RWsSession Fl_X::WsSession;
CBitmapContext *Fl_X::WindowGc;
CWsScreenDevice *Fl_X::WsScreenDevice;
RTimer Fl_X::Timer;
CFont *Fl_X::Font;
RFs Fl_X::Fs;
RFbsSession Fl_X::FbsSession;
CBitmapContext* Fl_X::SavedGc;
CFbsBitGc* Fl_X::BitmapGc;
CRedrawEventActive* Fl_X::RedrawEventActive;
CWsEventActive* Fl_X::WsEventActive;
CWaitTimeoutActive* Fl_X::WaitTimeoutActive;

Fl_Widget *fl_selection_requestor;

void fl_fix_focus();

Fl_X* Fl_X::make(Fl_Window *w)
{
	// DONE: S60
	Fl_Group::current (NULL);
	if (!Fl_X::WsSessionReady) {
		// TODO: S60, Handle error(s)
		Fl_X::TrapCleanup = CTrapCleanup::New();
		Fl_X::WsSession.Connect();
		Fl_X::WsSession.ClaimSystemPointerCursorList();
		Fl_X::WsSession.SetPointerCursorMode(EPointerCursorWindow);
		Fl_X::WsSession.SetClientCursorMode(EPointerCursorWindow);
		Fl_X::WsScreenDevice = new (ELeave) CWsScreenDevice (Fl_X::WsSession);
		Fl_X::WsScreenDevice->Construct();
		Fl_X::WindowGc = new (ELeave) CWindowGc (Fl_X::WsScreenDevice);
		((CWindowGc*) Fl_X::WindowGc)->Construct();
		Fl_X::Timer.CreateLocal();
		/* Fl_X::WsPointerCursor = new (ELeave) RWsPointerCursor(Fl_X::WsSession);
		Fl_X::WsPointerCursor->Construct (0);
		Fl_X::WsPointerCursor->Activate(); */
		Fl_X::WsSession.SetPointerCursorArea (TRect (0, 0, Fl::w(), Fl::h()));
		Fl_X::WsScreenDevice->GetNearestFontInPixels(Fl_X::Font, TFontSpec(_L("Roman"), 12));
		// Fl_X::WindowGc->UseFont(Fl_X::Font);
		Fl_X::Fs.Connect();
		Fl_X::FbsSession.Connect (Fl_X::Fs);
		Fl_X::BitmapGc = CFbsBitGc::NewL();
		// CActiveScheduler *currentScheduler = CActiveScheduler::Current();
		CActiveScheduler::Install (new (ELeave) CActiveScheduler);
		// SpawnPosixServerThread();
		Fl_X::RedrawEventActive = new (ELeave) CRedrawEventActive();
		Fl_X::WsEventActive = new (ELeave) CWsEventActive();
		Fl_X::WaitTimeoutActive = new (ELeave) CWaitTimeoutActive();
		Fl_X::WsSessionReady = true;
	}

	// if (showit) {
	/* w->set_visible();
	int old_event = Fl::e_number;
	w->handle(Fl::e_number = FL_SHOW);
	Fl::e_number = old_event;
	w->redraw(); */

	Fl_X *x = new (ELeave) Fl_X;
	x->windowGroup = new (ELeave) RWindowGroup (Fl_X::WsSession);
	x->windowGroup->Construct ((unsigned long) &x->w);
	// DONE: Set window group name
	if (w->label()) {
		HBufC *label16 = NULL;
		TRAPD(error, label16 = CnvUtfConverter::ConvertToUnicodeFromUtf8L(TPtr8((unsigned char*) w->label(), strlen(w->label()))));
		if (label16) {
			x->windowGroup->SetName(label16->Des());
			delete label16;
		}
	}
	x->xid = new (ELeave) RWindow(Fl_X::WsSession);
	x->w2 = w;
	x->xid->Construct(*x->windowGroup, (unsigned long) &x->w2);
	x->xid->EnableVisibilityChangeEvents();
	x->xid->EnableOnEvents();
	x->xid->EnableFocusChangeEvents();
	x->xid->PointerFilter(0xffffffff, 0x00000000); // deliver all pointer events
	// x->xid->EnablePointerMoveBuffer();
	x->xid->SetPointerCursor(1);
	// x->windowGroup->SetPointerCursor(0);
	x->setwindow(w);
	x->region = NULL;
	x->other_xid = NULL;
	x->next = Fl_X::first;
	Fl_X::first = x;
	x->xid->SetOrdinalPosition(0);
	x->xid->Activate();
	w->set_visible();
	int old_event = Fl::e_number;
	w->handle(Fl::e_number = FL_SHOW);
	Fl::e_number = old_event;
	w->redraw();
	Fl_X::WsSession.Flush();
	if (w->modal()) {
		Fl::modal_ = w;
		fl_fix_focus();
	}
	return x;
}

void Fl::copy(const char *stuff, int len, int clipboard, const char *type)
//void Fl::copy(const char *stuff, int len, int clipboard)
{
	// TODO: S60
}

void Fl::paste(Fl_Widget &receiver, int clipboard, const char *type)
//void Fl::paste(Fl_Widget &w, int clipboard)
{
	// TODO: S60
}

Fl_Window* Fl_Window::current_;
Window fl_window;

CWindowGc* fl_GetDC(Window w)
{
	// DONE: S60
	Fl_X::WsSession.Flush();
	if (fl_window) {
		((CWindowGc*)Fl_X::WindowGc)->Deactivate();
		fl_window = NULL;
	}
	fl_window = w;
	((CWindowGc*) Fl_X::WindowGc)->Activate (*fl_window);
	Fl_X::WindowGc->UseFont(Fl_X::Font);
	return ((CWindowGc*) Fl_X::WindowGc);
}

extern float fl_s60_line_width_; // fl_line_style.cxx
extern int fl_s60_line_style_; // fl_line_style.cxx
extern Fl_Color fl_color_; // fl_color_s60.cxx

static float fl_s60_line_width_saved_;
static int fl_s60_line_style_saved_;
static Fl_Color fl_s60_color_saved_;

void fl_save_pen(void)
{
	// DONE: S60
	fl_s60_line_width_saved_ = fl_s60_line_width_;
	fl_s60_line_style_saved_ = fl_s60_line_style_;
	fl_s60_color_saved_ = fl_color_;
}

void fl_restore_pen(void)
{
	// DONE: S60
	fl_s60_line_width_ = fl_s60_line_width_saved_;
	fl_s60_line_style_ = fl_s60_line_style_saved_;
	fl_color_ = fl_s60_color_saved_;
	fl_line_style (fl_s60_line_style_, fl_s60_line_width_);
	fl_color(fl_color_);
}

void fl_reset_spot()
{
	// TODO: S60
}

void fl_set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win)
//void fl_set_spot(int font, int size, int x, int y, int w, int h)
{
	// TODO: S60
}

static int mouse_event(Fl_Window *window, const TPointerEvent &event)
{
	// DONE: S60
	Fl::e_x = Fl::e_x_root = event.iPosition.iX;
	Fl::e_y = Fl::e_y_root = event.iPosition.iY;

	switch (event.iType) {
	case TPointerEvent::EButton1Down:
		Fl::e_keysym = FL_Button + 1;
		return Fl::handle(FL_PUSH, window);

	case TPointerEvent::EButton1Up:
		Fl::e_keysym = FL_Button + 1;
		Fl::e_is_click = 1;
		return Fl::handle(FL_RELEASE, window);

	case TPointerEvent::EButton2Down:
		Fl::e_keysym = FL_Button + 2;
		return Fl::handle(FL_PUSH, window);

	case TPointerEvent::EButton2Up:
		Fl::e_keysym = FL_Button + 2;
		Fl::e_is_click = 1;
		return Fl::handle(FL_RELEASE, window);

	case TPointerEvent::EButton3Down:
		Fl::e_keysym = FL_Button + 3;
		return Fl::handle(FL_PUSH, window);

	case TPointerEvent::EButton3Up:
		Fl::e_keysym = FL_Button + 3;
		Fl::e_is_click = 1;
		return Fl::handle(FL_RELEASE, window);

	case TPointerEvent::EDrag:
		return Fl::handle(FL_DRAG, window);

	case TPointerEvent::EMove:
		return Fl::handle(FL_MOVE, window);
	}

	return 0;
}

static int scancode_s60_to_fltk(unsigned int s60ScanCode)
{
	switch (s60ScanCode) {
	case EStdKeyDevice0:
		return ' ';
	case EStdKeyEnter:
	case EStdKeyDevice3:
		return FL_Enter;
	case EStdKeyBackspace:
		return FL_BackSpace;
	case EStdKeyLeftArrow:
		return FL_Left;
	case EStdKeyRightArrow:
		return FL_Right;
	case EStdKeyUpArrow:
		return FL_Up;
	case EStdKeyDownArrow:
		return FL_Down;
	}

	return 0;
}

static int keycode_s60_to_fltk(unsigned int s60KeyCode)
{
	switch (s60KeyCode) {
	case EKeyDevice0:
		return ' ';
	case EKeyEnter:
	case EKeyDevice3:
		return FL_Enter;
	case EKeyBackspace:
		return FL_BackSpace;
	case EKeyLeftArrow:
		return FL_Left;
	case EKeyRightArrow:
		return FL_Right;
	case EKeyUpArrow:
		return FL_Up;
	case EKeyDownArrow:
		return FL_Down;
	}
	return 0;
}

static int key_event (Fl_Window *window, const TWsEvent &event)
{
	char buffer[16];
	const TKeyEvent &kev = *event.Key();
	unsigned int keyCode = keycode_s60_to_fltk(kev.iCode);
	unsigned int scanCode = scancode_s60_to_fltk(kev.iScanCode);
	switch (event.Type()) {
	case EEventKey:
		if (0) { // keyCode == ' ') // keyCode)
			buffer[0] = ' ';
			buffer[1] = 0;
			Fl::e_length = 1;
			Fl::e_keysym = keyCode;
			Fl::e_text = buffer;
		} else if ((kev.iCode >= '0' && kev.iCode <= '9') || kev.iCode == '*' || kev.iCode == '#') { // || kev.iCode == ' ')
			buffer[0] = kev.iCode;
			buffer[1] = 0;
			Fl::e_keysym = kev.iCode;
			Fl::e_length = 1;
			Fl::e_text = buffer;
		} else {
			return 0;
		}
		return Fl::handle (FL_KEYBOARD, window);

	case EEventKeyUp:
		if (scanCode) {
			Fl::e_length = 0;
			Fl::e_keysym = scanCode;
			return Fl::handle (FL_KEYUP, window);
		} else
			return 0;

	case EEventKeyDown:
		if (scanCode) {
			Fl::e_length = 0;
			Fl::e_keysym = scanCode;
			return Fl::handle (FL_KEYDOWN, window);
		} else
			return 0;
	}
	return 0;
}

/* void fl_s60_wait_for_any_request_ (int count, ...)
	{
	va_list l;
	va_start (l, count);
	for (int i = 0; i < count; i++)
		{
		TRequestStatus *requestStatus = va_arg(l, TRequestStatus*);
		}
	va_end (l);
	} */

#define FL_SYMBIANEVENT_WSEVENT 0x01
#define FL_SYMBIANEVENT_WSREDRAW 0x02
#define FL_SYMBIANEVENT_WAITTIMEOUT 0x04
#define FL_SYMBIANEVENT_USERTIMEOUT 0x08

static int fl_symbianevent_mask;

static TWsEvent wsEvent;
static TWsRedrawEvent redrawEvent;

static int fl_s60_wait_result_;

extern int fl_font_, fl_fontsize_; // fl_font_s60.cxx

static void (*fl_s60_user_timeout_cb_)(void*);
static void *fl_s60_user_timeout_param_;

void CRedrawEventActive::Start()
{
	Fl_X::WsSession.RedrawReady(&iStatus);
	SetActive();
}

void CRedrawEventActive::DoCancel()
{
	Fl_X::WsSession.RedrawReadyCancel();
}

void CRedrawEventActive::RunL()
{
	// TODO: S60, Handle redraw events
	fl_symbianevent_mask = FL_SYMBIANEVENT_WSREDRAW;
	Fl_X::WsSession.GetRedraw (redrawEvent);
	CActiveScheduler::Stop();
	return;
}

void CWsEventActive::DoCancel()
{
	Fl_X::WsSession.EventReadyCancel();
}

void CWsEventActive::Start()
{
	Fl_X::WsSession.EventReady(&iStatus);
	SetActive();
}

void CWsEventActive::RunL()
{
	// TODO: S60, Handle events
	fl_symbianevent_mask = FL_SYMBIANEVENT_WSEVENT;
	Fl_X::WsSession.GetEvent(wsEvent);
	CActiveScheduler::Stop();
	return;
}

void CWaitTimeoutActive::DoCancel()
{
	iTimer.Cancel();
}

void CWaitTimeoutActive::Start(TTimeIntervalMicroSeconds32 anInterval)
{
	iTimer.After(iStatus, anInterval);
	SetActive();
}

void CWaitTimeoutActive::RunL()
{
	fl_symbianevent_mask = FL_SYMBIANEVENT_WAITTIMEOUT;
	CActiveScheduler::Stop();
}

// Just like in Win32
static void nothing() {}
void (*fl_lock_function)() = nothing;
void (*fl_unlock_function)() = nothing;

// Re-used from Win32 implementation
#include <sys/socket.h>
#include <sys/select.h>
static int maxfd = 0;
static fd_set fdsets[3];

#define POLLIN 1
#define POLLOUT 4
#define POLLERR 8

static int nfds = 0;
static int fd_array_size = 0;
static struct FD {
	int fd;
	short events;
	void (*cb)(int, void*);
	void* arg;
} *fd = 0;

//*
int fl_wait(double time_to_wait)
{
	// TODO: S60
	//printf("run..................\n");

	// idle processing
	static char in_idle;
	if (Fl::idle && !in_idle) {
		in_idle = 1;
		Fl::idle();
		in_idle = 0;
	}

	// ported from Win32 version
	if (nfds) { // TODO: Implement sockets polling!!!
		timeval t;
		t.tv_sec = 0;
		t.tv_usec = 0;

		fd_set fdt[3];
		memcpy(fdt, fdsets, sizeof fdt); // one shot faster fdt init
		// CActiveScheduler *currentScheduler = CActiveScheduler::Current();
		// CActiveScheduler::Install(NULL);
		if (fl_select_s60(maxfd+1,&fdt[0],&fdt[1],&fdt[2],&t)) {
			// We got something - do the callback!
			for (int i = 0; i < nfds; i ++) {
				int f = fd[i].fd;
				short revents = 0;
				if (FD_ISSET(f, &fdt[0])) revents |= POLLIN;
				if (FD_ISSET(f, &fdt[1])) revents |= POLLOUT;
				if (FD_ISSET(f, &fdt[2])) revents |= POLLERR;
				if (fd[i].events & revents) fd[i].cb(f, fd[i].arg);
			}
			time_to_wait = 0.000001; // just peek for any messages
		} else {
			// we need to check them periodically, so set a short timeout:
			if (time_to_wait > .001) time_to_wait = .001;
		}
		// t.tv_sec = t.tv_usec = 0;
		// select(0, NULL, NULL, NULL, &t); // Hope it cancels requests issued by select???
		// CActiveScheduler::Install(currentScheduler);
	}

	if (Fl::idle) //  || Fl::damage())
		time_to_wait = 0.0;

	if (!Fl_X::RedrawEventActive->IsActive()) {
		//Fl_X::RedrawEventActive->Cancel();
		Fl_X::RedrawEventActive->Start();
	}
	if (!Fl_X::WsEventActive->IsActive()) {
		// Fl_X::WsEventActive->Cancel();
		Fl_X::WsEventActive->Start();
	}
	Fl_X::WaitTimeoutActive->Cancel();
	if (time_to_wait < 2100) {
		Fl_X::WaitTimeoutActive->Start(time_to_wait * 1000000);
	}
	fl_unlock_function();
	CActiveScheduler::Start();
	fl_lock_function();

	if (fl_symbianevent_mask == FL_SYMBIANEVENT_WSREDRAW) {
		if (!redrawEvent.Handle()) {
			fl_s60_wait_result_ = 0;
			goto no_handle;
		}
		Fl_Window *w = *((Fl_Window**) redrawEvent.Handle());
		// w->redraw();
		Fl_X *ip = Fl_X::i(w);
		fl_GetDC(fl_xid(w));
		fl_save_pen();
		ip->xid->BeginRedraw();
		fl_font(fl_font_, fl_fontsize_);
		ip->flush();
		ip->xid->EndRedraw();
		fl_restore_pen();
		Fl_X::WsSession.Flush();
		((CWindowGc*) Fl_X::WindowGc)->Deactivate();
		fl_window = NULL;
		Fl_X::WsSession.Flush();
		fl_s60_wait_result_ = 1;
	} else if (fl_symbianevent_mask == FL_SYMBIANEVENT_WSEVENT) {
		if (!wsEvent.Handle()) {
			fl_s60_wait_result_ = 0;
			goto no_handle;
		}
		Fl_Window *w = *((Fl_Window**) wsEvent.Handle());
		if (wsEvent.Type() == EEventPointer)
			fl_s60_wait_result_ = mouse_event (w, *wsEvent.Pointer());
		else if (wsEvent.Type() == EEventKey || wsEvent.Type() == EEventKeyUp || wsEvent.Type() == EEventKeyDown) {
			fl_s60_wait_result_ = key_event (w, wsEvent);
		} else if (wsEvent.Type() == EEventFocusGained)
			fl_s60_wait_result_ = Fl::handle(FL_FOCUS, w);
		else if (wsEvent.Type() == EEventFocusLost)
			fl_s60_wait_result_ = Fl::handle(FL_UNFOCUS, w);
	} else if (fl_symbianevent_mask == FL_SYMBIANEVENT_WAITTIMEOUT) {
		fl_s60_wait_result_ = 0;
	} else if (fl_symbianevent_mask == FL_SYMBIANEVENT_USERTIMEOUT) {
		fl_s60_user_timeout_cb_(fl_s60_user_timeout_param_);
		fl_s60_wait_result_ = 1;
	}

no_handle:

	Fl::flush();
	// Fl_X::first->xid->Invalidate();
	return fl_s60_wait_result_;
}
//*/

/*
int fl_wait(double time_to_wait)
{
	// TODO: S60
	TRequestStatus redrawReadyStatus, eventReadyStatus, timeoutStatus;
	Fl_X::WsSession.RedrawReadyCancel();
	Fl_X::WsSession.RedrawReady (&redrawReadyStatus);
	Fl_X::WsSession.EventReadyCancel();
	Fl_X::WsSession.EventReady(&eventReadyStatus);
	Fl_X::Timer.Cancel();
	if (time_to_wait < 2100) {
		Fl_X::Timer.After (timeoutStatus, time_to_wait * 1000000);
	}
	User::WaitForAnyRequest(); // redrawReadyStatus, eventReadyStatus);
	if (redrawReadyStatus != KRequestPending) {

	} else if (eventReadyStatus != KRequestPending) {

	} else if (time_to_wait < 2100 && timeoutStatus != KRequestPending) {
		return 0;
	}
	TInt schedulerError;
	TBool runResult = CActiveScheduler::Current()->RunIfReady(schedulerError, CActive::EPriorityIdle);
	CActiveScheduler::Current();
	return 0;
}
//*/

int fl_ready()
{
	// TODO: S60
}

void Fl_Window::make_current()
{
	// DONE: S60
	fl_GetDC(fl_xid(this));
	current_ = this;
	fl_clip_region(0);
}

void Fl_Window::show()
{
	// DONE: S60
	if (!shown()) {
		Fl_X::make(this);
		this->resize (0, 0, Fl::w(), Fl::h());
	} else {
		// DONE: S60, Bring it to top
		fl_xid(this)->SetVisible(true);
		fl_xid(this)->SetOrdinalPosition(0);
	}
}

void Fl_Window::resize(int x, int y, int w, int h)
{
	// TODO: S60
	// fl_xid(this)->SetExtent(TPoint(x, y), TSize(w, h));
	Fl_Group::resize (x, y, w, h);
}

void Fl_Window::size_range_()
{
	// TODO: S60
	size_range_set = 1;
}

void Fl_Window::label(char const *name, char const *iname)
{
	// TODO: S60
}

int Fl::x()
{
	// DONE: S60
	return 0;
}

int Fl::y()
{
	// DONE: S60
	return 0;
}

int Fl::w()
{
	// DONE: S60
	TPixelsTwipsAndRotation sizeAndRotation;
	Fl_X::WsScreenDevice->GetDefaultScreenSizeAndRotation(sizeAndRotation);
	return sizeAndRotation.iPixelSize.iWidth;
}

int Fl::h()
{
	// DONE: S60
	TPixelsTwipsAndRotation sizeAndRotation;
	Fl_X::WsScreenDevice->GetDefaultScreenSizeAndRotation(sizeAndRotation);
	return sizeAndRotation.iPixelSize.iHeight;
}

void Fl::get_mouse(int &x, int &y)
{
	// DONE: S60
	TPoint pt;
	pt = Fl_X::WsSession.PointerCursorPosition();
	x = pt.iX;
	y = pt.iY;
}

const char *fl_filename_name(const char *name)
{
	const char *p,*q;
	if (!name) return (0);
	q = name;
	if (q[0] && q[1]==':') q += 2; // skip leading drive letter
	for (p = q; *p; p++) if (*p == '/' || *p == '\\') q = p+1;
	return q;
}

char fl_show_iconic;

/*
void Fl::get_system_colors()
	{
	// TODO: S60
	}
*/

// Re-used from Win32 implemenatation
void Fl::add_fd(int n, int events, void (*cb)(int, void*), void *v)
{
	/* static int been_here = 0;
	if (!been_here)
		{
		been_here = 1;
		FD_ZERO(&fdsets[0]);
		FD_ZERO(&fdsets[1]);
		FD_ZERO(&fdsets[2]);
		} */

	remove_fd(n,events);
	int i = nfds++;
	if (i >= fd_array_size) {
		fd_array_size = 2*fd_array_size+1;
		fd = (FD*)realloc(fd, fd_array_size*sizeof(FD));
	}
	fd[i].fd = n;
	fd[i].events = (short)events;
	fd[i].cb = cb;
	fd[i].arg = v;

	if (events & POLLIN) FD_SET((unsigned)n, &fdsets[0]);
	if (events & POLLOUT) FD_SET((unsigned)n, &fdsets[1]);
	if (events & POLLERR) FD_SET((unsigned)n, &fdsets[2]);
	if (n > maxfd) maxfd = n;
}

void Fl::add_fd(int fd, void (*cb)(int, void*), void* v)
{
	Fl::add_fd(fd, POLLIN, cb, v);
}

void Fl::remove_fd(int n, int events)
{
	int i,j;
	for (i=j=0; i<nfds; i++) {
		if (fd[i].fd == n) {
			short e = fd[i].events & ~events;
			if (!e) continue; // if no events left, delete this fd
			fd[i].events = e;
		}
		// move it down in the array if necessary:
		if (j<i) {
			fd[j]=fd[i];
		}
		j++;
	}
	nfds = j;

	if (events & POLLIN) FD_CLR(unsigned(n), &fdsets[0]);
	if (events & POLLOUT) FD_CLR(unsigned(n), &fdsets[1]);
	if (events & POLLERR) FD_CLR(unsigned(n), &fdsets[2]);
}

void Fl::remove_fd(int n)
{
	Fl::remove_fd(n, -1);
}

// Timers
class CTimeoutActive: public CActive
{
public:
	RTimer iTimer;
	Fl_Timeout_Handler iCallback;
	void *iParam;
	CTimeoutActive(): CActive(CActive::EPriorityStandard) {
		iTimer.CreateLocal();
		CActiveScheduler::Add(this);
	}
	~CTimeoutActive() {
		Cancel();
		iTimer.Close();
	}
	void Start(TTimeIntervalMicroSeconds32 anInterval) {
		iTimer.After(iStatus, anInterval);
		SetActive();
	}
private:
	void DoCancel() {
		iTimer.Cancel();
	}
	void RunL() {
		// fl_lock_function();
		fl_symbianevent_mask = FL_SYMBIANEVENT_USERTIMEOUT;
		fl_s60_user_timeout_cb_ = iCallback;
		fl_s60_user_timeout_param_ = iParam;
		iCallback = NULL; // ready for Fl::repeat_timeout()
		CActiveScheduler::Stop();
		// fl_unlock_function();
	}
};

// static RArray<CTimeoutActive*> fl_s60_timeouts_;
static CTimeoutActive **fl_s60_timeout_slots_;
static int fl_s60_timeout_slots_cnt_;

void Fl::add_timeout(double time, Fl_Timeout_Handler cb, void* data)
{
	Fl::repeat_timeout(time, cb, data);
}

void Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void* data)
{
	CTimeoutActive *timeoutActive = NULL;

	for (int i = 0; i < fl_s60_timeout_slots_cnt_; i++) {
		if (fl_s60_timeout_slots_[i] == NULL || fl_s60_timeout_slots_[i]->iCallback == NULL) {
			timeoutActive = fl_s60_timeout_slots_[i];
			break;
		}
	}

	if (timeoutActive == NULL) {
		timeoutActive = new (ELeave) CTimeoutActive;
		int i;
		for (i = 0; i < fl_s60_timeout_slots_cnt_; i++) {
			if (fl_s60_timeout_slots_[i] == NULL) {
				fl_s60_timeout_slots_[i] = timeoutActive;
				break;
			}
		}
		if (i == fl_s60_timeout_slots_cnt_) {
			fl_s60_timeout_slots_cnt_ += 8;
			fl_s60_timeout_slots_ = (CTimeoutActive**) realloc (fl_s60_timeout_slots_, fl_s60_timeout_slots_cnt_ * sizeof(CTimeoutActive*));
			// fl_s60_timeouts_.Append(timeoutActive);
			fl_s60_timeout_slots_[fl_s60_timeout_slots_cnt_ - 8] = timeoutActive;
		}
	}

	timeoutActive->iCallback = cb;
	timeoutActive->iParam = data;
	if (time < 2100) {
		timeoutActive->Start(time * 1000000);
	}
}

int Fl::has_timeout(Fl_Timeout_Handler cb, void* data)
{
	for (int i = 0; i < fl_s60_timeout_slots_cnt_; i++) {
		if (fl_s60_timeout_slots_[i] != NULL && fl_s60_timeout_slots_[i]->iCallback == cb && fl_s60_timeout_slots_[i]->iParam == data) {
			return 1;
		}
	}
	return 0;
}

void Fl::remove_timeout(Fl_Timeout_Handler cb, void* data)
{
	for (int i = 0; i < fl_s60_timeout_slots_cnt_; i++) {
		if (fl_s60_timeout_slots_[i]->iCallback == cb && (fl_s60_timeout_slots_[i]->iParam == data || data == NULL)) {
			fl_s60_timeout_slots_[i]->Cancel();
			delete fl_s60_timeout_slots_[i];
			fl_s60_timeout_slots_[i] = NULL;
			// fl_s60_timeouts_.Remove(i);
			// i--;
		}
	}
}
