// Copyright 2015-2016 by cyantree <cyantree.guo@gmail.com>
/*
bug:
ok 1.mac pinyin type
ok 2.scroll
ok 3.timer, not test
ok 4.check virtual function lost
ok 5.get string and move mouse to overside
6.iconv cannot compile 64bit
7.win10 wm_touch
8.high dpi
*/

#include "Fl_Scintilla.h"

Fl_Scintilla::Fl_Scintilla(int X, int Y, int W, int H, const char* l) : Fl_Group(X, Y, W, H, l)
{
	color(FL_BACKGROUND2_COLOR, FL_SELECTION_COLOR);
	box(FL_DOWN_FRAME);

	swt_.rc_client.x = X+Fl::box_dx(box());
	swt_.rc_client.y = Y+Fl::box_dy(box());
	swt_.rc_client.w = W-Fl::box_dw(box());
	swt_.rc_client.h = H-Fl::box_dh(box());
	swt_.type = 0;
	swt_.wid = this;

	mVScrollBar = new Fl_Scrollbar(0,0,1,1);
	mVScrollBar->linesize(1);
	mVScrollBar->callback((Fl_Callback*)v_scrollbar_cb, this);
	mHScrollBar = new Fl_Scrollbar(0,0,1,1);
	mHScrollBar->linesize(20);
	mHScrollBar->callback((Fl_Callback*)h_scrollbar_cb, this);
	mHScrollBar->type(FL_HORIZONTAL);
	scrollpush_ = 0;

	end();

	scrollbar_width(Fl::scrollbar_size());

	//
	cb_notify_.data = NULL;
	cb_notify_.callback = NULL;

	//
	Platform_Initialise();
#ifdef SCI_LEXER
	Scintilla_LinkLexers();
#endif

	lastKeyDownConsumed = false;
	capturedMouse = false;
	LButtonDown = false;

	wMain = &swt_;
	//printf("fl_scintilla: %x\n", this);

	linesPerScroll = 3;
	wheelDelta = 0;   // Wheel delta from roll

	sysCaretWidth = 0;
	sysCaretHeight = 0;
	caret.period = 530;
	if (caret.period < 0) caret.period = 0;

	Initialise();

	sw_ = Scintilla::Surface::Allocate(0);
	ic_ = new ICONVConverter();
	ic_str_ = NULL;
	ic_str_len_ = 0;

	drag_str_ = NULL;
	drag_str_size_ = 0;
}

Fl_Scintilla::~Fl_Scintilla()
{
	sw_->Release();
	delete sw_;

	delete ic_;
	if ( ic_str_len_ > 0 ) free(ic_str_);

	if ( drag_str_ != NULL ) free(drag_str_);

	Platform_Finalise();

	Finalise();
}

#define SC_INDICATOR_INPUT INDIC_IME
#define SC_INDICATOR_TARGET INDIC_IME+1
#define SC_INDICATOR_CONVERTED INDIC_IME+2
#define SC_INDICATOR_UNKNOWN INDIC_IME_MAX
void Fl_Scintilla::Initialise()
{
	for (TickReason tr = tickCaret; tr <= tickDwell; tr = static_cast<TickReason>(tr + 1)) {
		timeractive_[tr] = 0;
	}
	vs.indicators[SC_INDICATOR_UNKNOWN] = Scintilla::Indicator(INDIC_HIDDEN, Scintilla::ColourDesired(0, 0, 0xff));
	vs.indicators[SC_INDICATOR_INPUT] = Scintilla::Indicator(INDIC_DOTS, Scintilla::ColourDesired(0, 0, 0xff));
	vs.indicators[SC_INDICATOR_CONVERTED] = Scintilla::Indicator(INDIC_COMPOSITIONTHICK, Scintilla::ColourDesired(0, 0, 0xff));
	vs.indicators[SC_INDICATOR_TARGET] = Scintilla::Indicator(INDIC_STRAIGHTBOX, Scintilla::ColourDesired(0, 0, 0xff));
}

void Fl_Scintilla::Finalise()
{
	ScintillaBase::Finalise();
	for (TickReason tr = tickCaret; tr <= tickDwell; tr = static_cast<TickReason>(tr + 1)) {
		FineTickerCancel(tr);
	}
	SetIdle(false);
}

void Fl_Scintilla::NotifyParent(Scintilla::SCNotification scn)
{
	if ( cb_notify_.callback == NULL ) return;
	cb_notify_.callback(&scn, cb_notify_.data);
}

void Fl_Scintilla::resize(int X, int Y, int W, int H)
{
	//printf("resize, v visiable=%d, h visiable=%d\n", mVScrollBar->visible(), mHScrollBar->visible());
	swt_.rc_client.x = X+Fl::box_dx(box());
	swt_.rc_client.y = Y+Fl::box_dy(box());
	swt_.rc_client.w = W-Fl::box_dw(box());
	swt_.rc_client.h = H-Fl::box_dh(box());
	Fl_Group::resize(X,Y,W,H);

	ChangeSize();
}

/** Map the key codes to their equivalent SCK_ form. */
static int KeyTranslate(int keyIn)
{
	switch (keyIn) {
	case FL_Down:
		return SCK_DOWN;
	case FL_Up:
		return SCK_UP;
	case FL_Left:
		return SCK_LEFT;
	case FL_Right:
		return SCK_RIGHT;
	case FL_Home:
		return SCK_HOME;
	case FL_End:
		return SCK_END;
	case FL_Page_Up:
		return SCK_PRIOR;
	case FL_Page_Down:
		return SCK_NEXT;
	case FL_Delete:
		return SCK_DELETE;
	case FL_Insert:
		return SCK_INSERT;
	case FL_Escape:
		return SCK_ESCAPE;
	case FL_BackSpace:
		return SCK_BACK;
	case FL_Tab:
		return SCK_TAB;
	case FL_Enter:
		return SCK_RETURN;
	case FL_KP+'+':
		return SCK_ADD;
	case FL_KP+'-':
		return SCK_SUBTRACT;
	case FL_KP+'/':
		return SCK_DIVIDE;
	case FL_Meta_L:
		return SCK_WIN;
	case FL_Meta_R:
		return SCK_RWIN;
	case FL_Menu:
		return SCK_MENU;
	default:
		return keyIn;
	}
}

int Fl_Scintilla::handle(int event)
{
	int button;
	unsigned int clockt;

	if ( event == FL_HIDE ) {
		fl_reset_spot();
		if ( ct.wCallTip.Created() ) SendEditor(SCI_CALLTIPCANCEL);
		if ( ac.lb->Created() ) ac.lb->Destroy();
		return 1;
	}

	if ( event == FL_FOCUS ) {
		fl_set_spot(labelfont(), labelsize(), x(), y(), w(), h(), window());
		SetFocusState(true);
		DestroySystemCaret();
		CreateSystemCaret();
		return 1;
	}

	if ( event == FL_UNFOCUS ) {
		if ( ct.wCallTip.Created() ) SendEditor(SCI_CALLTIPCANCEL);
		if ( ac.lb->Created() ) ac.lb->Destroy();
		fl_reset_spot();
		SetFocusState(false);
		DestroySystemCaret();
		return 1;
	}

	if ( event == FL_ENTER ) {
		Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);
		if (PointInSelMargin(pt)) {
			DisplayCursor(GetMarginCursor(pt));
		} else if (PointInSelection(pt) && !SelectionEmpty()) {
			DisplayCursor(Scintilla::Window::cursorArrow);
		} else if (PointIsHotspot(pt)) {
			DisplayCursor(Scintilla::Window::cursorHand);
		} else {
			if (Fl::event_inside(swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h)) {
				DisplayCursor(Scintilla::Window::cursorText);
			} else {
				DisplayCursor(Scintilla::Window::cursorArrow);
			}
		}
		return 1;
	}

	if ( event == FL_LEAVE ) {
		DisplayCursor(Scintilla::Window::cursorArrow);
		SetTrackMouseLeaveEvent(false);
		MouseLeave();
		return 1;
	}

	if ( event == FL_KEYDOWN ) {
		return handle_key(event);
	}

	if ( event == FL_MOVE ) {
		if (!Fl::event_inside(swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h)) {
			DisplayCursor(Scintilla::Window::cursorArrow);
			return 0;
		}

		const Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);

		// Windows might send WM_MOUSEMOVE even though the mouse has not been moved:
		// http://blogs.msdn.com/b/oldnewthing/archive/2003/10/01/55108.aspx
		if (ptMouseLast.x != pt.x || ptMouseLast.y != pt.y) {
			SetTrackMouseLeaveEvent(true);
			ButtonMoveWithModifiers(pt,
			                        (Fl::event_state(FL_SHIFT) != 0 ? SCI_SHIFT : 0) |
			                        (Fl::event_state(FL_CTRL) != 0 ? SCI_CTRL : 0) |
			                        (Scintilla::Platform::IsKeyDown(FL_ALT) ? SCI_ALT : 0));
		}

		return 1;
	}

	if ( event == FL_PASTE ) {
		if (!Fl::event_text()) return 1;
		DoPaste(Fl::event_text());
		return 1;
	}

	if ( event == FL_DRAG ) {
		if ( scrollpush_ == 1 ) return mVScrollBar->handle(event);
		if ( scrollpush_ == 2 ) return mHScrollBar->handle(event);

		const Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);
		// Windows might send WM_MOUSEMOVE even though the mouse has not been moved:
		// http://blogs.msdn.com/b/oldnewthing/archive/2003/10/01/55108.aspx
		if (ptMouseLast.x != pt.x || ptMouseLast.y != pt.y) {
			SetTrackMouseLeaveEvent(true);
			ButtonMoveWithModifiers(pt, 
				(Fl::event_state(FL_SHIFT) != 0 ? SCI_SHIFT : 0) | 
				(Fl::event_state(FL_CTRL) != 0 ? SCI_CTRL : 0) | 
				(Scintilla::Platform::IsKeyDown(FL_ALT) ? SCI_ALT : 0));
		}

		return 0;
	}

	if ( event == FL_DND_ENTER ) {
		DragEnter();
		return 1;
	}
	if ( event == FL_DND_DRAG ) {
		DragOver();
		return 1;
	}

	if ( event == FL_DND_LEAVE ) {
		DragLeave();
		return 1;
	}

	if ( event == FL_DND_RELEASE ) {
		Drop();
		return 1;
	}

	if ( event == FL_PUSH ) {
		//printf("fl_push\n");

		scrollpush_ = 0;
		if ( Fl::event_inside(mVScrollBar->x(), mVScrollBar->y(), mVScrollBar->w(), mVScrollBar->h()) && mVScrollBar->visible()) {
			scrollpush_ = 1;
			return mVScrollBar->handle(event);
		}
		if ( Fl::event_inside(mHScrollBar->x(), mHScrollBar->y(), mHScrollBar->w(), mHScrollBar->h()) && mHScrollBar->visible()) {
			scrollpush_ = 2;
			return mHScrollBar->handle(event);
		}

		//printf("push\n");
		if (!Fl::event_inside(swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h)) return 0;
		if (Fl::visible_focus() && handle(FL_FOCUS)) Fl::focus(this);

		button = Fl::event_button();
		if ( button == 1 ) {
			clockt = (clock() * 1000) / CLOCKS_PER_SEC;
			ButtonDown(Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y), clockt,
			           Fl::event_state(FL_SHIFT) != 0,
			           Fl::event_state(FL_CTRL) != 0,
			           Scintilla::Platform::IsKeyDown(FL_ALT));
			RefreshIME();
			LButtonDown = true;
			return 1;
		}
		if ( button == 3 ) {
			Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);
			if (!PointInSelection(pt)) {
				CancelModes();
				SetEmptySelection(PositionFromLocation(pt));
			}
			return 1;
		}
		return 0;
	}

	if ( event == FL_RELEASE ) {
		//printf("fl_release\n");

		if ( scrollpush_ == 1 ) return mVScrollBar->handle(event);
		if ( scrollpush_ == 2 ) return mHScrollBar->handle(event);
		scrollpush_ = 0;

		if ( !hasFocus ) return 0;		
		//if ( !Fl::visible_focus() ) return 0;

		button = Fl::event_button();
		if ( button == 1 ) {
			if (!LButtonDown ) {
				clockt = (clock() * 1000) / CLOCKS_PER_SEC;
				ButtonDown(Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y), clockt,
				           Fl::event_state(FL_SHIFT) != 0,
				           Fl::event_state(FL_CTRL) != 0,
				           Scintilla::Platform::IsKeyDown(FL_ALT));
			}

			clockt = (clock() * 1000) / CLOCKS_PER_SEC;
			ButtonUp(Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y), clockt, Fl::event_state(FL_CTRL) != 0);
			LButtonDown = false;
			return 1;
		}
		if ( button == 3 ) {
			if (displayPopupMenu) {
				Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);

				if (!PointInSelection(pt)) {
					CancelModes();
					SetEmptySelection(PositionFromLocation(pt));
					Fl::wait(0);
				}

				if ((pt.x == -1) && (pt.y == -1)) {
					// Caused by keyboard so display menu near caret
					pt = PointMainCaret();
					pt = Scintilla::Point::FromInts(pt.x, pt.y);
				}
				ContextMenu(pt);
			}
			return 1;
		}
		return 0;
	}

	if ( event == FL_MOUSEWHEEL ) {
		// if autocomplete list active then send mousewheel message to it
		if (ac.Active()) {
			SCIWinType *swt = (SCIWinType *)(ac.lb->GetID());
			Fl_Window *win = (Fl_Window *)swt->wid;
			win->handle(event);
			return 1;
		}

		if ( !hasFocus ) return 0;

		// Don't handle datazoom.
		// (A good idea for datazoom would be to "fold" or "unfold" details.
		// i.e. if datazoomed out only class structures are visible, when datazooming in the control
		// structures appear, then eventually the individual statements...)
		if ( Fl::event_state(FL_SHIFT) ) {
			return 0;
		}

		// Either SCROLL or ZOOM. We handle the wheel steppings calculation
		wheelDelta = Fl::event_dy();
		if ( wheelDelta != 0 && linesPerScroll > 0) {
			int linesToScroll = linesPerScroll;
			if (linesPerScroll == 0xffffffff)
				linesToScroll = LinesOnScreen() - 1;
			if (linesToScroll == 0) {
				linesToScroll = 1;
			}
			linesToScroll *= (wheelDelta);

			if (Fl::event_state()&FL_CTRL) {
				// Zoom! We play with the font sizes in the styles.
				// Number of steps/line is ignored, we just care if sizing up or down
				if (linesToScroll < 0) {
					KeyCommand(SCI_ZOOMIN);
				} else {
					KeyCommand(SCI_ZOOMOUT);
				}
			} else {
				// Scroll
				ScrollTo(topLine + linesToScroll);
				redraw();
				//Redraw();
			}
		}
		return 1;
	}

	return 0;//Fl_Group::handle(event);
}

int Fl_Scintilla::handle_key(int event)
{
	//printf("keydown:%d\n", Fl::event_key());

	char *s;
	void *p;
	int len, n, del=0;

	// caret
	RefreshIME();

	// shortcut
	if ( ! Fl::compose(del) ) {
		if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='z') {
			ScintillaBase::WndProc(SCI_UNDO,  0, 0);
			return 1;
		}
		if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='x') {
			ScintillaBase::WndProc(SCI_CUT,   0, 0);
			return 1;
		}
		if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='c') {
			ScintillaBase::WndProc(SCI_COPY,  0, 0);
			return 1;
		}
		if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='v') {
			ScintillaBase::WndProc(SCI_PASTE, 0, 0);
			return 1;
		}
		if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='a') {
			ScintillaBase::WndProc(SCI_SELECTALL, 0, 0);
			return 1;
		}

		lastKeyDownConsumed = false;
		int ret = KeyDown(KeyTranslate(Fl::event_key()),
		                  Scintilla::Platform::IsKeyDown(FL_SHIFT),
		                  Scintilla::Platform::IsKeyDown(FL_CTRL),
		                  Scintilla::Platform::IsKeyDown(FL_ALT),
		                  &lastKeyDownConsumed);
		if ( ret || lastKeyDownConsumed ) return 1;
		return 0;
	}

	if (del) {
		int pos = CurrentPosition();
		SetSelection(pos-del, pos);
		ClearSelection();
	}

	// text
	unsigned char c = Fl::event_text()[0];
	if (((Fl::event_key() >= 128) || !iscntrl(c)) || !lastKeyDownConsumed) {
		s = (char *)Fl::event_text();
		len = strlen(s);
		if ( len < 1 ) return 0;
		if ( ! IsUnicodeMode() ) {
			if ( len*8 > ic_str_len_ ) {
				p = realloc(ic_str_, len*8);
				if ( p == NULL ) return 1;
				ic_str_ = (char *)p;
				ic_str_len_ = len*8;
			}
			ic_->Open4MB(CodePageOfDocument());
			n = ic_->convert(s, len, ic_str_, len*8);
			ic_str_[n] = 0;

			s = ic_str_;
			len = n;
		}

		AddCharUTF(s, len);
		return 1;
	}
	return 0;
}

static bool BoundsContains(Scintilla::PRectangle rcBounds, Scintilla::PRectangle rcCheck)
{
	bool contains = true;
	if (!rcCheck.Empty()) {
		if (!rcBounds.Contains(rcCheck)) {
			contains = false;
		}
	}
	return contains;
}
void Fl_Scintilla::draw()
{
	//printf("draw\n");

	fl_push_clip(x(),y(),w(),h());	// prevent drawing outside widget area
	Fl_Color bgcolor = active_r() ? color() : fl_inactive(color());

	//
	if (damage() & FL_DAMAGE_ALL) {
		draw_box(box(), x(), y(), w(), h(), bgcolor);
		// draw that little box in the corner of the scrollbars
		if (mVScrollBar->visible() && mHScrollBar->visible()) {
			fl_rectf(mVScrollBar->x(), mHScrollBar->y(), mVScrollBar->w(), mHScrollBar->h(), FL_GRAY);
		}
	}

	// draw the scrollbars
	if (damage() & (FL_DAMAGE_ALL |FL_DAMAGE_CHILD)) {
		mVScrollBar->damage(FL_DAMAGE_ALL);
		mHScrollBar->damage(FL_DAMAGE_ALL);
	}
	update_child(*mVScrollBar);
	update_child(*mHScrollBar);
	//printf("draw scroll visiable:v%d h%d\n", mVScrollBar->visible(), mHScrollBar->visible());

	//
	int X, Y, W, H;
	if (damage() & FL_DAMAGE_ALL ) {
		if ( ! fl_clip_box(swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h, X, Y, W, H) ) {
			X = swt_.rc_client.x;
			Y = swt_.rc_client.y;
			W = swt_.rc_client.w;
			H = swt_.rc_client.h;
		}

		bool assertsPopup = Scintilla::Platform::ShowAssertionPopUps(false);
		//if ( X+W > swt_.rc_client.x+swt_.rc_client.w ) W = swt_.rc_client.x+swt_.rc_client.w - X;
		fl_push_clip(X, Y, W, H);
		//printf("fl_push_clip:%d %d %d %d\n", X, Y, X+W, Y+H);
		paintState = painting;
		rcPaint = Scintilla::PRectangle::FromInts(X-swt_.rc_client.x, Y-swt_.rc_client.y, X-swt_.rc_client.x+W, Y-swt_.rc_client.y+H);
		Scintilla::PRectangle rcClient = GetClientRectangle();
		paintingAllText = BoundsContains(rcPaint, rcClient);
		if ( sw_ ) {
			//printf("draw rect:%d %d %d %d\n", (int)rcPaint.left, (int)rcPaint.top, (int)rcPaint.Width(), (int)rcPaint.Height());
			sw_->Init((void*)1, &swt_);
			Paint(sw_, rcPaint);
			sw_->Release();
		}

		if (paintState == paintAbandoned) {
			// Painting area was insufficient to cover new styling or brace highlight positions
			FullPaint();
		}

		paintState = notPainting;
		fl_pop_clip();
		Scintilla::Platform::ShowAssertionPopUps(assertsPopup);
	}

	fl_pop_clip();
}

bool Fl_Scintilla::PaintContains(Scintilla::PRectangle rc)
{
	if (paintState == painting) {
		return BoundsContains(rcPaint, rc);
	}
	return true;
}

/**
 * Redraw all of text area.
 * This paint will not be abandoned.
 */
void Fl_Scintilla::FullPaint()
{
	paintState = painting;
	rcPaint = GetClientRectangle();
	paintingAllText = true;
	sw_->Init((void*)1, &swt_);
	Paint(sw_, rcPaint);
	sw_->Release();
	paintState = notPainting;
}

static int CodePageFromCharSet(int characterSet, int documentCodePage)
{
	if (documentCodePage == SC_CP_UTF8) return SC_CP_UTF8;

	switch (characterSet) {
	case SC_CHARSET_ANSI:
		return 1252;
	case SC_CHARSET_DEFAULT:
		return documentCodePage;
	case SC_CHARSET_BALTIC:
		return 1257;
	case SC_CHARSET_CHINESEBIG5:
		return 950;
	case SC_CHARSET_EASTEUROPE:
		return 1250;
	case SC_CHARSET_GB2312:
		return 936;
	case SC_CHARSET_GREEK:
		return 1253;
	case SC_CHARSET_HANGUL:
		return 949;
	case SC_CHARSET_MAC:
		return 10000;
	case SC_CHARSET_OEM:
		return 437;
	case SC_CHARSET_RUSSIAN:
		return 1251;
	case SC_CHARSET_SHIFTJIS:
		return 932;
	case SC_CHARSET_TURKISH:
		return 1254;
	case SC_CHARSET_JOHAB:
		return 1361;
	case SC_CHARSET_HEBREW:
		return 1255;
	case SC_CHARSET_ARABIC:
		return 1256;
	case SC_CHARSET_VIETNAMESE:
		return 1258;
	case SC_CHARSET_THAI:
		return 874;
	case SC_CHARSET_8859_15:
		return 28605;
		// Not supported
	case SC_CHARSET_CYRILLIC:
		return documentCodePage;
	case SC_CHARSET_SYMBOL:
		return documentCodePage;
	}
	return documentCodePage;
}

int Fl_Scintilla::CodePageOfDocument() const
{
	return CodePageFromCharSet(vs.styles[STYLE_DEFAULT].characterSet, pdoc->dbcsCodePage);
}

bool Fl_Scintilla::ValidCodePage(int codePage) const
{
	return codePage == 0 || codePage == SC_CP_UTF8 || codePage == 932 || codePage == 936 || codePage == 949 || codePage == 950 || codePage == 1361;
}

void Fl_Scintilla::SetTrackMouseLeaveEvent(bool on)
{
	// Do nothing
}

void Fl_Scintilla::SetMouseCapture(bool on)
{
	capturedMouse = on;
}

bool Fl_Scintilla::HaveMouseCapture()
{
	// Cannot just see if GetCapture is this window as the scroll bar also sets capture for the window
	return capturedMouse;
}

// =========== Copy & Paste =======================================
void Fl_Scintilla::CopyToClipboard(const Scintilla::SelectionText &selectedText)
{
	const char* copy = selectedText.Data();
	int len = static_cast<int>(selectedText.LengthWithTerminator());
	if ( (copy==NULL) || (len<1) ) return;

	if (IsUnicodeMode()) {
		Fl::copy(copy, len, 1);
	} else {
		char *dst;
		int dstsize=len*8;
		dst = (char *)malloc(dstsize);

		ic_->Open4UTF8(CodePageOfDocument());
		int n = ic_->convert((char*)copy, len, dst, dstsize);
		if ( n <= 0 ) {
			free(dst);
			return;
		}

		Fl::copy(dst, n, 1);

		free(dst);
	}
}

void Fl_Scintilla::Copy()
{
	if (!sel.Empty()) {
		Scintilla::SelectionText selectedText;
		CopySelectionRange(&selectedText);
		CopyToClipboard(selectedText);
	}
}

void Fl_Scintilla::CopyAllowLine()
{
	Scintilla::SelectionText selectedText;
	CopySelectionRange(&selectedText, true);
	CopyToClipboard(selectedText);
}

bool Fl_Scintilla::CanPaste()
{
	if (!Editor::CanPaste())
		return false;
	if ( Fl::event_text() != NULL ) return true;
	return false;
}

void Fl_Scintilla::Paste()
{
	Fl::paste(*this, 1);
}

void Fl_Scintilla::DoPaste(const char *s)
{
	const bool isLine = SelectionEmpty();
	ClearSelection(multiPasteMode == SC_MULTIPASTE_EACH);

	unsigned int len = strlen(s);
	if (IsUnicodeMode()) {
		InsertPasteShape(s, len, pasteStream);
	} else {
		char *dst;
		int dstsize=len*8;
		dst = (char *)malloc(dstsize);

		ic_->Open4MB(CodePageOfDocument());
		int n = ic_->convert((char*)s, len, dst, dstsize);
		if ( n <= 0 ) {
			free(dst);
			return;
		}

		InsertPasteShape(dst, n, pasteStream);

		free(dst);
	}
}

// =========== Dnd =======================================
bool Fl_Scintilla::DragThreshold(Scintilla::Point ptStart, Scintilla::Point ptNow)
{
	int xMove = static_cast<int>(std::abs(ptStart.x - ptNow.x));
	int yMove = static_cast<int>(std::abs(ptStart.y - ptNow.y));
	return (xMove > 4) || (yMove > 4);
}

void Fl_Scintilla::SaveDragData(const Scintilla::SelectionText &selectedText)
{
	drag_str_size_ = 0;

	const char* copy = selectedText.Data();
	int len = static_cast<int>(selectedText.LengthWithTerminator());
	if ( (copy==NULL) || (len<1) ) return;

	void *p;
	if (IsUnicodeMode()) {
		p = realloc(drag_str_, len+1);
		if ( p == NULL ) return;
		drag_str_ = (char *)p;
		memcpy(drag_str_, copy, len);
		drag_str_[len] = 0;
		drag_str_size_ = len+1;
		//Fl::copy(copy, len, 1);
	} else {
		char *dst;
		int dstsize=len*8;
		dst = (char *)malloc(dstsize);

		ic_->Open4UTF8(CodePageOfDocument());
		int n = ic_->convert((char*)copy, len, dst, dstsize);
		if ( n <= 0 ) {
			free(dst);
			return;
		}

		p = realloc(drag_str_, len+1);
		if ( p == NULL ) {
			free(dst);
			return;
		}
		drag_str_ = (char *)p;
		memcpy(drag_str_, dst, n+1);
		drag_str_[n] = 0;
		drag_str_size_ = n+1;
		// Fl::copy(dst, n, 1);

		free(dst);
	}
}

void Fl_Scintilla::StartDrag()
{
	Scintilla::Editor::inDragDrop = ddDragging;
	dropWentOutside = true;

	if (!sel.Empty()) {
		Scintilla::SelectionText selectedText;
		CopySelectionRange(&selectedText);
		SaveDragData(selectedText);
		Fl::copy(drag_str_, drag_str_size_, 0);
	}

#if __FLTK_MACOSX__
	Fl_X::dnd(1);
#else
	Fl::dnd();
#endif

	Scintilla::Editor::inDragDrop = ddNone;
	SetDragPosition(Scintilla::SelectionPosition(Scintilla::invalidPosition));
}

void Fl_Scintilla::DragEnter()
{
	// do nothing
}

int Fl_Scintilla::DragOver()
{
	try {
		if ( pdoc->IsReadOnly()) {
			return 0;
		}

		// Update the cursor.
		Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);
		SetDragPosition(SPositionFromLocation(pt, false, false, UserVirtualSpace()));

#if __FLTK_WIN32__
		Fl::wait(0);
#endif

		return 1;
	} catch (...) {
		errorStatus = SC_STATUS_FAILURE;
	}

	return 0;
}

void Fl_Scintilla::DragLeave()
{
	try {
		SetDragPosition(Scintilla::SelectionPosition(Scintilla::invalidPosition));
		return;
	} catch (...) {
		errorStatus = SC_STATUS_FAILURE;
	}
}

void Fl_Scintilla::Drop()
{
	try {
		SetDragPosition(Scintilla::SelectionPosition(Scintilla::invalidPosition));

		Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);
		Scintilla::SelectionPosition movePos = SPositionFromLocation(pt, false, false, UserVirtualSpace());

		char *data = "";
		int len = 0;
		if ( Fl::event_state(FL_CTRL) != 0 ) DropAt(movePos, data, len, 0, 1);
		else DropAt(movePos, data, len, 1, 1);

		return;
	} catch (...) {
		errorStatus = SC_STATUS_FAILURE;
	}
}

// =========== Case =======================================
class CaseFolderDBCS : public Scintilla::CaseFolderTable
{
	// Allocate the expandable storage here so that it does not need to be reallocated
	// for each call to Fold.
	std::vector<wchar_t> utf16Mixed;
	std::vector<wchar_t> utf16Folded;
	int cp;

	ICONVConverter ic_;
public:
	explicit CaseFolderDBCS(int cp_) : cp(cp_) {
		StandardASCII();
	}
	virtual size_t Fold(char *folded, size_t sizeFolded, const char *mixed, size_t lenMixed) {
		if ((lenMixed == 1) && (sizeFolded > 0)) {
			folded[0] = mapping[static_cast<unsigned char>(mixed[0])];
			return 1;
		} else {
			if (lenMixed > utf16Mixed.size()) {
				utf16Mixed.resize(lenMixed + 8);
			}
			ic_.OpenFromCodepage(cp, "WCHAR_T");
			size_t nUtf16Mixed = ic_.convert((char*)mixed,
			                                 static_cast<int>(lenMixed),
			                                 (char*)&utf16Mixed[0],
			                                 static_cast<int>(utf16Mixed.size()));
			/*
			size_t nUtf16Mixed = ::MultiByteToWideChar(cp, 0, mixed,
				static_cast<int>(lenMixed),
				&utf16Mixed[0],
				static_cast<int>(utf16Mixed.size()));
			*/

			if (nUtf16Mixed < 1) {
				// Failed to convert -> bad input
				folded[0] = '\0';
				return 1;
			}

			unsigned int lenFlat = 0;
			for (size_t mixIndex=0; mixIndex < nUtf16Mixed; mixIndex++) {
				if ((lenFlat + 20) > utf16Folded.size()) utf16Folded.resize(lenFlat + 60);
				const char *foldedUTF8 = CaseConvert(utf16Mixed[mixIndex], Scintilla::CaseConversionFold);
				if (foldedUTF8) {
					// Maximum length of a case conversion is 6 bytes, 3 characters
					wchar_t wFolded[20];
					size_t charsConverted = Scintilla::UTF16FromUTF8(foldedUTF8,
					                        strlen(foldedUTF8),
					                        wFolded, ELEMENTS(wFolded));
					for (size_t j=0; j<charsConverted; j++)
						utf16Folded[lenFlat++] = wFolded[j];
				} else {
					utf16Folded[lenFlat++] = utf16Mixed[mixIndex];
				}
			}

			ic_.OpenToCodepage("WCHAR_T", cp);
			size_t lenOut = ic_.GetLength((char*)&utf16Folded[0], lenFlat);
			/*
			size_t lenOut = ::WideCharToMultiByte(cp, 0,
				&utf16Folded[0], lenFlat,
				NULL, 0, NULL, 0);
			*/

			if (lenOut < sizeFolded) {
				ic_.convert((char*)&utf16Folded[0], lenFlat, folded, static_cast<int>(lenOut));
				/*
				::WideCharToMultiByte(cp, 0,
					&utf16Folded[0], lenFlat,
					folded, static_cast<int>(lenOut), NULL, 0);
				*/
				return lenOut;
			} else {
				return 0;
			}
		}
	}
};

Scintilla::CaseFolder *Fl_Scintilla::CaseFolderForEncoding()
{
	ICONVConverter *ic;

	int cpDest = CodePageOfDocument();
	if (cpDest == SC_CP_UTF8) {
		return new Scintilla::CaseFolderUnicode();
	} else {
		if (pdoc->dbcsCodePage == 0) {
			Scintilla::CaseFolderTable *pcf = new Scintilla::CaseFolderTable();
			pcf->StandardASCII();
			// Only for single byte encodings
			int cpDoc = CodePageOfDocument();
			for (int i=0x80; i<0x100; i++) {
				char sCharacter[2] = "A";
				sCharacter[0] = static_cast<char>(i);
				wchar_t wCharacter[20];
				ic = new ICONVConverter();
				ic->OpenFromCodepage(cpDoc, "WCHAR_T");
				unsigned int lengthUTF16 = ic->convert((char*)sCharacter, 1, (char*)wCharacter, ELEMENTS(wCharacter));
				//unsigned int lengthUTF16 = ::MultiByteToWideChar(cpDoc, 0, sCharacter, 1, wCharacter, ELEMENTS(wCharacter));
				if (lengthUTF16 == 1) {
					const char *caseFolded = CaseConvert(wCharacter[0], Scintilla::CaseConversionFold);
					if (caseFolded) {
						wchar_t wLower[20];
						size_t charsConverted = Scintilla::UTF16FromUTF8(caseFolded,
						                        strlen(caseFolded),
						                        wLower, ELEMENTS(wLower));
						if (charsConverted == 1) {
							char sCharacterLowered[20];
							ic->OpenToCodepage("WCHAR_T", cpDoc);
							unsigned int lengthConverted = ic->convert((char*)wLower, static_cast<int>(charsConverted), sCharacterLowered, ELEMENTS(sCharacterLowered));
							//unsigned int lengthConverted = ::WideCharToMultiByte(cpDoc, 0, wLower, static_cast<int>(charsConverted), sCharacterLowered, ELEMENTS(sCharacterLowered), NULL, 0);
							if ((lengthConverted == 1) && (sCharacter[0] != sCharacterLowered[0])) {
								pcf->SetTranslation(sCharacter[0], sCharacterLowered[0]);
							}
						}
					}
				}
				delete ic;
			}
			return pcf;
		} else {
			return new CaseFolderDBCS(cpDest);
		}
	}
}

std::string Fl_Scintilla::CaseMapString(const std::string &s, int caseMapping)
{
	if ((s.size() == 0) || (caseMapping == cmSame))
		return s;

	int cpDoc = CodePageOfDocument();
	if (cpDoc == SC_CP_UTF8) {
		std::string retMapped(s.length() * Scintilla::maxExpansionCaseConversion, 0);
		size_t lenMapped = CaseConvertString(&retMapped[0], retMapped.length(), s.c_str(), s.length(),
		                                     (caseMapping == cmUpper) ? Scintilla::CaseConversionUpper : Scintilla::CaseConversionLower);
		retMapped.resize(lenMapped);
		return retMapped;
	}

	return s;

	/*
	ICONVConverter *ic;
	ic = new ICONVConverter();
	ic->OpenFromCodepage(cpDoc, "WCHAR_T");
	unsigned int lengthUTF16 = ic->GetLength((char*)s.c_str(), static_cast<int>(s.size()));
	//unsigned int lengthUTF16 = ::MultiByteToWideChar(cpDoc, 0, s.c_str(), static_cast<int>(s.size()), NULL, 0);
	if (lengthUTF16 == 0) {	// Failed to convert
		delete ic;
		return s;
	}

	//DWORD mapFlags = LCMAP_LINGUISTIC_CASING | ((caseMapping == cmUpper) ? LCMAP_UPPERCASE : LCMAP_LOWERCASE);

	// Change text to UTF-16
	std::vector<wchar_t> vwcText(lengthUTF16);
	ic->convert((char*)s.c_str(), static_cast<int>(s.size()), (char*)&vwcText[0], lengthUTF16);
	//::MultiByteToWideChar(cpDoc, 0, s.c_str(), static_cast<int>(s.size()), &vwcText[0], lengthUTF16);

	// Change case
	int charsConverted = ::LCMapStringW(LOCALE_SYSTEM_DEFAULT, mapFlags,
		&vwcText[0], lengthUTF16, NULL, 0);
	std::vector<wchar_t> vwcConverted(charsConverted);
	::LCMapStringW(LOCALE_SYSTEM_DEFAULT, mapFlags,
		&vwcText[0], lengthUTF16, &vwcConverted[0], charsConverted);

	// Change back to document encoding
	unsigned int lengthConverted = ::WideCharToMultiByte(cpDoc, 0,
		&vwcConverted[0], static_cast<int>(vwcConverted.size()),
		NULL, 0, NULL, 0);
	std::vector<char> vcConverted(lengthConverted);
	::WideCharToMultiByte(cpDoc, 0,
		&vwcConverted[0], static_cast<int>(vwcConverted.size()),
		&vcConverted[0], static_cast<int>(vcConverted.size()), NULL, 0);

	delete ic;

	return std::string(&vcConverted[0], vcConverted.size());
	*/
}

// ================================================================
void Fl_Scintilla::NotifyChange()
{
	ScintillaBase::Command(SCEN_CHANGE);
}

void Fl_Scintilla::ScrollText(int /* linesToMove */)
{
	Redraw();
	UpdateSystemCaret();
}

void Fl_Scintilla::SetVerticalScrollPos()
{
	if ( topLine != mVScrollBar->value() ) {
		DwellEnd(true);
		mVScrollBar->value(topLine);
	}
}

void Fl_Scintilla::SetHorizontalScrollPos()
{
	if ( mHScrollBar->value() != xOffset ) {
		DwellEnd(true);
		mHScrollBar->value(xOffset);
	}
}

bool Fl_Scintilla::ModifyScrollBars(int nMax, int nPage)
{
	unsigned char vshow=1, hshow=1;

	bool modified = false;
	int pos, windowSize, first, total;
	first = (int)mVScrollBar->minimum();
	total = (int)mVScrollBar->maximum();
	windowSize = (int)mVScrollBar->slider_size();
	pos = (int)mVScrollBar->value();
	int vertEndPreferred = nMax;
	if (!verticalScrollBarVisible) nPage = vertEndPreferred + 1;
	if ( nPage > vertEndPreferred ) vshow = 0;
	if ( (first != 0) || (total != vertEndPreferred) || (windowSize != nPage) || (pos != 0) ) {
		mVScrollBar->value(0, nPage, 0, vertEndPreferred);
		mVScrollBar->value(pos);
		modified = true;
	}

	Scintilla::PRectangle rcText = GetTextRectangle();
	int horizEndPreferred = scrollWidth;
	if (horizEndPreferred < 0) horizEndPreferred = 0;
	unsigned int pageWidth = static_cast<unsigned int>(rcText.Width());
	if (!horizontalScrollBarVisible || Wrapping()) pageWidth = horizEndPreferred + 1;
	if ( (int)pageWidth > horizEndPreferred ) hshow = 0;
	first = (int)mHScrollBar->minimum();
	total = (int)mHScrollBar->maximum();
	windowSize = (int)mHScrollBar->slider_size();
	pos = (int)mHScrollBar->value();
	if ( (first != 0) || (total != horizEndPreferred) || (windowSize != pageWidth) || (pos != 0)) {
		mHScrollBar->value(0, pageWidth, 0, horizEndPreferred);
		mHScrollBar->value(pos);
		modified = true;
		if (scrollWidth < (int)(pageWidth)) {
			HorizontalScrollTo(0);
		}
	}

	//
	if ( vshow == 0 && hshow == 0 ) {
		swt_.rc_client.x = x()+Fl::box_dx(box());
		swt_.rc_client.y = y()+Fl::box_dy(box());
		swt_.rc_client.w = w()-Fl::box_dw(box());
		swt_.rc_client.h = h()-Fl::box_dh(box());
		mVScrollBar->clear_visible();
		mHScrollBar->clear_visible();
	} else if ( vshow == 1 && hshow == 0 ) {
		swt_.rc_client.x = x()+Fl::box_dx(box());
		swt_.rc_client.y = y()+Fl::box_dy(box());
		swt_.rc_client.w = w()-Fl::box_dw(box()) - scrollbar_width();
		swt_.rc_client.h = h()-Fl::box_dh(box());
		mVScrollBar->set_visible();
		mVScrollBar->resize(swt_.rc_client.x+swt_.rc_client.w, swt_.rc_client.y, scrollbar_width(), swt_.rc_client.h);
		mHScrollBar->clear_visible();
	} else if ( vshow == 0 && hshow == 1 ) {
		swt_.rc_client.x = x()+Fl::box_dx(box());
		swt_.rc_client.y = y()+Fl::box_dy(box());
		swt_.rc_client.w = w()-Fl::box_dw(box());
		swt_.rc_client.h = h()-Fl::box_dh(box()) - scrollbar_width();
		mVScrollBar->clear_visible(); //printf("clear v scroll......, %d %d %d %d\n", swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h);
		mHScrollBar->set_visible();
		mHScrollBar->resize(swt_.rc_client.x, swt_.rc_client.y+swt_.rc_client.h, swt_.rc_client.w, scrollbar_width());
	} else if ( vshow == 1 && hshow == 1 ) {
		swt_.rc_client.x = x()+Fl::box_dx(box());
		swt_.rc_client.y = y()+Fl::box_dy(box());
		swt_.rc_client.w = w()-Fl::box_dw(box()) - scrollbar_width();
		swt_.rc_client.h = h()-Fl::box_dh(box()) - scrollbar_width();
		mVScrollBar->set_visible();
		mVScrollBar->resize(swt_.rc_client.x+swt_.rc_client.w, swt_.rc_client.y, scrollbar_width(), swt_.rc_client.h);
		mHScrollBar->set_visible();
		mHScrollBar->resize(swt_.rc_client.x, swt_.rc_client.y+swt_.rc_client.h, swt_.rc_client.w, scrollbar_width());
	}
	if (paintState == notPainting) DropGraphics(false);
	//damage(FL_DAMAGE_ALL);
	//Redraw();
	damage(FL_DAMAGE_ALL, mVScrollBar->x(), mHScrollBar->y(), mVScrollBar->w(), mHScrollBar->h());

	//printf("%s, %d, %d, modify:%d, new rc:%d %d %d %d, v%d, h%d\n", __FUNCTION__, nMax, nPage, modified, swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h, mVScrollBar->visible(), mHScrollBar->visible());
	
	return modified;
}

void Fl_Scintilla::NotifyFocus(bool focus)
{
	Command(focus ? SCEN_SETFOCUS : SCEN_KILLFOCUS);
	Scintilla::Editor::NotifyFocus(focus);
}

void Fl_Scintilla::RefreshIME()
{
	Scintilla::Point pos = PointMainCaret();
	int x = Scintilla::RoundXYPosition(pos.x)+swt_.rc_client.x;
	int y = Scintilla::RoundXYPosition(pos.y)+swt_.rc_client.y;
	int height = fl_height(labelfont(), labelsize());
#ifdef __FLTK_MACOSX__
	Fl::insertion_point_location(x, y+height, height);
#else
	fl_set_spot(labelfont(), labelsize(), x, y+height, w(), h(), window());
#endif
}

void Fl_Scintilla::UpdateSystemCaret()
{
	if (hasFocus) {
		if (HasCaretSizeChanged()) {
			DestroySystemCaret();
			CreateSystemCaret();
		}
	}
}

void Fl_Scintilla::MoveImeCarets(int offset)
{
	// Move carets relatively by bytes.
	for (size_t r=0; r<sel.Count(); r++) {
		int positionInsert = sel.Range(r).Start().Position();
		sel.Range(r).caret.SetPosition(positionInsert + offset);
		sel.Range(r).anchor.SetPosition(positionInsert + offset);
	}
}

bool Fl_Scintilla::HasCaretSizeChanged() const
{
	if (
	        ( (0 != vs.caretWidth) && (sysCaretWidth != vs.caretWidth) )
	        || ((0 != vs.lineHeight) && (sysCaretHeight != vs.lineHeight))
	) {
		return true;
	}
	return false;
}

bool Fl_Scintilla::CreateSystemCaret()
{
	sysCaretWidth = vs.caretWidth;
	if (0 == sysCaretWidth) sysCaretWidth = 1;
	sysCaretHeight = vs.lineHeight;

	return 1;
}

bool Fl_Scintilla::DestroySystemCaret()
{
	return 1;
}

bool Fl_Scintilla::FineTickerRunning(TickReason reason)
{
	return timeractive_[reason-tickCaret] != 0;
}

void Fl_Scintilla::FineTickerStart(TickReason reason, int millis, int tolerance)
{
	FineTickerCancel(reason);
	double tick_timeout=millis/1000.0;
	if ( reason == tickCaret && timeractive_[0] == 0 ) {
		Fl::add_timeout(tick_timeout, static_time_fun_1, this);
		timeractive_[0] = 1;
		timetick_[0] = tick_timeout;
	}
	if ( reason == tickScroll && timeractive_[1] == 0 ) {
		Fl::add_timeout(tick_timeout, static_time_fun_2, this);
		timeractive_[1] = 1;
		timetick_[1] = tick_timeout;
	}
	if ( reason == tickWiden && timeractive_[2] == 0 ) {
		Fl::add_timeout(tick_timeout, static_time_fun_3, this);
		timeractive_[2] = 1;
		timetick_[2] = tick_timeout;
	}
	if ( reason == tickDwell && timeractive_[3] == 0 ) {
		Fl::add_timeout(tick_timeout, static_time_fun_4, this);
		timeractive_[3] = 1;
		timetick_[3] = tick_timeout;
	}
	if ( reason == tickPlatform && timeractive_[4] == 0 ) {
		Fl::add_timeout(tick_timeout, static_time_fun_5, this);
		timeractive_[4] = 1;
		timetick_[4] = tick_timeout;
	}
}

void Fl_Scintilla::FineTickerCancel(TickReason reason)
{
	if ( reason == tickCaret && timeractive_[0] == 1 ) {
		Fl::remove_timeout(static_time_fun_1, this);
		timeractive_[0] = 0;
	}
	if ( reason == tickScroll && timeractive_[1] == 1 ) {
		Fl::remove_timeout(static_time_fun_2, this);
		timeractive_[1] = 0;
	}
	if ( reason == tickWiden && timeractive_[2] == 1 ) {
		Fl::remove_timeout(static_time_fun_3, this);
		timeractive_[2] = 0;
	}
	if ( reason == tickDwell && timeractive_[3] == 1 ) {
		Fl::remove_timeout(static_time_fun_4, this);
		timeractive_[3] = 0;
	}
	if ( reason == tickPlatform && timeractive_[4] == 1 ) {
		Fl::remove_timeout(static_time_fun_5, this);
		timeractive_[4] = 0;
	}
}

bool Fl_Scintilla::SetIdle(bool on)
{
	// On Win32 the Idler is implemented as a Timer on the Scintilla window.  This
	// takes advantage of the fact that WM_TIMER messages are very low priority,
	// and are only posted when the message queue is empty, i.e. during idle time.
	if (idler.state != on) {
		if (on) {
			idle_dwstart_ = 0;			
			Fl::add_timeout(10.0/1000.0, static_time_idle, this);
			idler.idlerID = (void*)1; //::SetTimer(MainHWND(), idleTimerID, 10, NULL) ? reinterpret_cast<IdlerID>(idleTimerID) : 0;
		} else {
			Fl::remove_timeout(static_time_idle, this);
			//::KillTimer(MainHWND(), reinterpret_cast<uptr_t>(idler.idlerID));
			idler.idlerID = 0;
		}
		idler.state = idler.idlerID != 0;
	}
	return idler.state;
}

void Fl_Scintilla::time_fun_idle()
{
	if ( ! idler.state ) return;

	if (idler.state) {
		//if (lParam || (WAIT_TIMEOUT == MsgWaitForMultipleObjects(0, 0, 0, 0, QS_INPUT|QS_HOTKEY))) {
		if (Idle()) {
			// User input was given priority above, but all events do get a turn.  Other
			// messages, notifications, etc. will get interleaved with the idle messages.

			// However, some things like WM_PAINT are a lower priority, and will not fire
			// when there's a message posted.  So, several times a second, we stop and let
			// the low priority events have a turn (after which the timer will fire again).

			unsigned int dwCurrent = (unsigned int)clock();
			unsigned int dwStart = idle_dwstart_ ? idle_dwstart_ : dwCurrent;
			const unsigned int maxWorkTime = 50;

			if (dwCurrent >= dwStart && dwCurrent > maxWorkTime && dwCurrent - maxWorkTime < dwStart) {
				//PostMessage(MainHWND(), SC_WIN_IDLE, dwStart, 0);
				idle_dwstart_ = dwStart;
				Fl::repeat_timeout(10.0/1000.0, static_time_idle, this);
			}
		} else {
			SetIdle(false);
		}
	}
}

void Fl_Scintilla::time_fun(int index)
{
	if ( index == 0 ) TickFor(tickCaret);
	else if ( index == 1 ) TickFor(tickScroll);
	else if ( index == 2 ) TickFor(tickWiden);
	else if ( index == 3 ) TickFor(tickDwell);
	else if ( index == 4 ) TickFor(tickPlatform);

	if ( index == 0 ) Fl::repeat_timeout(timetick_[0], static_time_fun_1, this);
	if ( index == 1 ) Fl::repeat_timeout(timetick_[1], static_time_fun_2, this);
	if ( index == 2 ) Fl::repeat_timeout(timetick_[2], static_time_fun_3, this);
	if ( index == 3 ) Fl::repeat_timeout(timetick_[3], static_time_fun_4, this);
	if ( index == 4 ) Fl::repeat_timeout(timetick_[4], static_time_fun_5, this);
}

// ================================================================
void Fl_Scintilla::update_v_scrollbar()
{
	ScrollTo(mVScrollBar->value());
}

void Fl_Scintilla::update_h_scrollbar()
{
	int xPos = xOffset;
	Scintilla::PRectangle rcText = GetTextRectangle();
	int pageWidth = static_cast<int>(rcText.Width() * 2 / 3);

	//printf("%d, %d\n", mHScrollBar->value(), xOffset);
	HorizontalScrollTo(mHScrollBar->value());
}

void Fl_Scintilla::v_scrollbar_cb(Fl_Scrollbar* b, Fl_Scintilla* textD)
{
	textD->update_v_scrollbar();
}

void Fl_Scintilla::h_scrollbar_cb(Fl_Scrollbar* b, Fl_Scintilla* textD)
{
	textD->update_h_scrollbar();
}

// =========== call tip =======================================
class Fl_CallWindow : public Fl_Double_Window
{
public:
	Fl_CallWindow(Scintilla::CallTip *ct, void dopush(void *x), void *x, int X, int Y, int W, int H, const char *l=0) : Fl_Double_Window(X, Y, W, H, l) {
		ct_ = ct;
		cb_notify_.callback = dopush;
		cb_notify_.data = x;

		swt_.type = 2;
		swt_.wid = this;
	}
	SCIWinType swt_;
protected:
	Scintilla::CallTip *ct_;
	void draw() {
		fl_push_clip(0, 0, w(), h());

		Scintilla::Surface *surfaceWindow = Scintilla::Surface::Allocate(0);
		if (surfaceWindow) {
			surfaceWindow->Init((void*)2, &swt_);
			surfaceWindow->SetUnicodeMode(SC_CP_UTF8 == ct_->codePage);
			surfaceWindow->SetDBCSMode(ct_->codePage);
			ct_->PaintCT(surfaceWindow);
			surfaceWindow->Release();
			delete surfaceWindow;
		}

		fl_pop_clip();
	}

	int handle(int e) {
		if ( e == FL_PUSH ) {
			int x = Fl::event_x();
			int y = Fl::event_y();
			ct_->MouseClick(Scintilla::Point::FromInts(x, y));
			cb_notify_.callback(cb_notify_.data);
			return 1;
		}

		return Fl_Double_Window::handle(e);
	}

	struct {
		void (*callback)(void *data);
		void *data;
	} cb_notify_;
};

void Fl_Scintilla::CallTip_DoPush()
{
	CallTipClick();
}
static void calltip_dopush(void *x)
{
	Fl_Scintilla *sci = (Fl_Scintilla *)x;
	sci->CallTip_DoPush();
}
void Fl_Scintilla::CreateCallTipWindow(Scintilla::PRectangle rc)
{
	if ( ct.wCallTip.Created()) return;

	int wx=x(), wy=y();
	for (Fl_Window* w = this->window(); w; w = w->window()) {
		wx += w->x();
		wy += w->y();
	}

	Fl_Group::current(0);
	callwin_ = new Fl_CallWindow(&ct, calltip_dopush, this, (int)rc.left+wx, (int)rc.top+wy, (int)rc.Width(), (int)rc.Height());
	callwin_->end();
	callwin_->clear_border();
	callwin_->set_tooltip_window();

	Fl_CallWindow* cw = (Fl_CallWindow*)callwin_;
	cw->swt_.main_client.x = wx;
	cw->swt_.main_client.y = wy;
	ct.wCallTip = &(cw->swt_);
	ct.wDraw = ct.wCallTip;
}
