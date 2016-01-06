// Copyright 2015-2016 by cyantree <cyantree.guo@gmail.com>

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
	mVScrollBar->callback((Fl_Callback*)v_scrollbar_cb, this);
	mHScrollBar = new Fl_Scrollbar(0,0,1,1);
	mHScrollBar->callback((Fl_Callback*)h_scrollbar_cb, this);
	mHScrollBar->type(FL_HORIZONTAL);

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

void Fl_Scintilla::resize(int X, int Y, int W, int H)
{
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
	void *p;
	char *s;
	int len, n;

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
		SetFocusState(false);
		DestroySystemCaret();
		return 1;
	}

	if ( event == FL_KEYDOWN ) {
		if ( Fl::event_key()==FL_Escape) return 0;

		if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='z') { ScintillaBase::WndProc(SCI_UNDO,  0, 0); return 1; }
		if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='x') { ScintillaBase::WndProc(SCI_CUT,   0, 0); return 1; }
		if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='c') { 
			ScintillaBase::WndProc(SCI_COPY,  0, 0); return 1; 
		}
		if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='v') {	ScintillaBase::WndProc(SCI_PASTE, 0, 0); return 1; }
		if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='a') { ScintillaBase::WndProc(SCI_SELECTALL, 0, 0); return 1; }

		lastKeyDownConsumed = false;
		int ret = KeyDown(KeyTranslate(Fl::event_key()), 
			Scintilla::Platform::IsKeyDown(FL_SHIFT), 
			Scintilla::Platform::IsKeyDown(FL_CTRL),
			Scintilla::Platform::IsKeyDown(FL_ALT),
			&lastKeyDownConsumed);
		if ( ret || lastKeyDownConsumed ) return 1;

		int c = Fl::event_text()[0];
		if (((Fl::event_key() >= 128) || !iscntrl(c)) || !lastKeyDownConsumed) {
			s = (char *)Fl::event_text();
			len = strlen(s);
			if ( len < 1 ) return 1;
			if (IsUnicodeMode()) {
				AddCharUTF(s, len);
			} else {
				if ( len*8 > ic_str_len_ ) {
					p = realloc(ic_str_, len*8);
					if ( p == NULL ) return 1;
					ic_str_ = (char *)p;
					ic_str_len_ = len*8;
				}
				ic_->Open4MB(CodePageOfDocument());
				n = ic_->convert(s, len, ic_str_, len*8);
				ic_str_[n] = 0;
				AddCharUTF(ic_str_, n);
			}
		}
		return 1;
	}

	if ( event == FL_MOVE ) {
		if (!Fl::event_inside(swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h)) {
			DisplayCursor(Scintilla::Window::cursorArrow);
			return 1;
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

	//printf("%d\n", event);

	switch (event) {
		case FL_ENTER: {
		//int x, y;
		//Fl::get_mouse(x, y);
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
	case FL_LEAVE: {
		//printf("Fl_Leave\n");
		DisplayCursor(Scintilla::Window::cursorArrow);
		return 1;
	}
	case FL_PUSH:
		if (Fl::visible_focus() && handle(FL_FOCUS)) Fl::focus(this);
		if (!Fl::event_inside(swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h)) {
			mHScrollBar->handle(event);
			mVScrollBar->handle(event);
			return 1;
		}

		button = Fl::event_button();
		if ( button == 1 ) {			
			clockt = (clock() * 1000) / CLOCKS_PER_SEC;
			//printf("left button down, %d %d, %d\n", Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y, clockt);
			ButtonDown(Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y), clockt,
				Fl::event_state(FL_SHIFT) != 0, 
				Fl::event_state(FL_CTRL) != 0, 
				Scintilla::Platform::IsKeyDown(FL_ALT));
			return 1;
		}
		if ( button == 3 ) {
			take_focus();
			Scintilla::Point pt(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);
			if (!PointInSelection(pt)) {
				CancelModes();
				SetEmptySelection(PositionFromLocation(pt));
			}
			return 1;
		}
		return 0;
	case FL_RELEASE: 
		{
			if (!Fl::event_inside(swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h)) {
				mHScrollBar->handle(event);
				mVScrollBar->handle(event);
				return 1;
			}

			button = Fl::event_button();
			if ( button == 1 ) {
				clockt = (clock() * 1000) / CLOCKS_PER_SEC;
				//printf("left button up, %d %d, %d\n", Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y, clockt);
				ButtonUp(Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y), clockt, Fl::event_state(FL_CTRL) != 0);
				return 1;
			}
			if ( button == 3 ) {
				if (displayPopupMenu) {
					Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);
					if ((pt.x == -1) && (pt.y == -1)) {
						// Caused by keyboard so display menu near caret
						pt = PointMainCaret();
						//POINT spt = {static_cast<int>(pt.x), static_cast<int>(pt.y)};
						//::ClientToScreen(MainHWND(), &spt);
						pt = Scintilla::Point::FromInts(pt.x, pt.y);
					}
					ContextMenu(pt);
					return 1;
				}
				
			}
			break;
		}
	case FL_MOUSEWHEEL:
		{
			// if autocomplete list active then send mousewheel message to it
			//*
			if (ac.Active()) {
				SCIWinType *swt = (SCIWinType *)(ac.lb->GetID());
				Fl_Window *win = (Fl_Window *)swt->wid;
				win->handle(event);
				return 1;
				//HWND hWnd = reinterpret_cast<HWND>(ac.lb->GetID());
				//::SendMessage(hWnd, iMessage, wParam, lParam);
				//break;
			}
			//*/

			if ( !hasFocus ) return 1;

			// Don't handle datazoom.
			// (A good idea for datazoom would be to "fold" or "unfold" details.
			// i.e. if datazoomed out only class structures are visible, when datazooming in the control
			// structures appear, then eventually the individual statements...)
			if (Fl::event_state()&FL_SHIFT) {
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
				}
			}
			return 1;
		}
	}

	return Fl_Group::handle(event);
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
	fl_push_clip(x(),y(),w(),h());	// prevent drawing outside widget area
	Fl_Color bgcolor = active_r() ? color() : fl_inactive(color());

	//
	if (damage() & FL_DAMAGE_ALL) {
		draw_box(box(), x(), y(), w(), h(), bgcolor);
		// draw that little box in the corner of the scrollbars
		if (mVScrollBar->visible() && mHScrollBar->visible()) fl_rectf(mVScrollBar->x(), mHScrollBar->y(), mVScrollBar->w(), mHScrollBar->h(), FL_GRAY);
	}

	// draw the scrollbars
	if (damage() & (FL_DAMAGE_ALL | FL_DAMAGE_CHILD)) {
		mVScrollBar->damage(FL_DAMAGE_ALL);
		mHScrollBar->damage(FL_DAMAGE_ALL);
	}
	update_child(*mVScrollBar);
	update_child(*mHScrollBar);

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
		fl_push_clip(X, Y, W, H);
		paintState = painting;
		rcPaint = Scintilla::PRectangle::FromInts(X-swt_.rc_client.x, Y-swt_.rc_client.y, X-swt_.rc_client.x+W, Y-swt_.rc_client.y+H);
		Scintilla::PRectangle rcClient = GetClientRectangle();
		paintingAllText = BoundsContains(rcPaint, rcClient);
		if ( sw_ ) {
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
	if ( Fl::clipboard_contains(Fl::clipboard_plain_text) ) return true;
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
	//Redraw();
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
		if ( p == NULL ) { free(dst); return; }
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
	
	Fl::dnd();

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

// ================================================================
void Fl_Scintilla::NotifyChange()
{
	ScintillaBase::Command(SCEN_CHANGE);
}

void Fl_Scintilla::ScrollText(int /* linesToMove */) 
{
	ScintillaBase::Redraw();
	UpdateSystemCaret();
}

void Fl_Scintilla::SetVerticalScrollPos()
{
	mVScrollBar->value(topLine);
}

void Fl_Scintilla::SetHorizontalScrollPos()
{
	mHScrollBar->value(xOffset);
}

bool Fl_Scintilla::ModifyScrollBars(int nMax, int nPage)
{
	//printf("%s, %d, %d\n", __FUNCTION__, nMax, nPage);
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
		mVScrollBar->clear_visible();		
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

	//
	return modified;
}

void Fl_Scintilla::NotifyFocus(bool focus)
{
	Command(focus ? SCEN_SETFOCUS : SCEN_KILLFOCUS);
	Scintilla::Editor::NotifyFocus(focus);
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
	if ( reason == 0 && timeractive_[0] == 1 ) {
		Fl::remove_timeout(static_time_fun_1, this);
		timeractive_[0] = 0;
	}
	if ( reason == 1 && timeractive_[1] == 1 ) {
		Fl::remove_timeout(static_time_fun_2, this);
		timeractive_[1] = 0;
	}
	if ( reason == 2 && timeractive_[2] == 1 ) {
		Fl::remove_timeout(static_time_fun_1, this);
		timeractive_[2] = 0;
	}
	if ( reason == 3 && timeractive_[3] == 1 ) {
		Fl::remove_timeout(static_time_fun_1, this);
		timeractive_[3] = 0;
	}
}

bool Fl_Scintilla::SetIdle(bool on)
{
	return 0;
	/*
	// On Win32 the Idler is implemented as a Timer on the Scintilla window.  This
	// takes advantage of the fact that WM_TIMER messages are very low priority,
	// and are only posted when the message queue is empty, i.e. during idle time.
	if (idler.state != on) {
		if (on) {
			idler.idlerID = ::SetTimer(MainHWND(), idleTimerID, 10, NULL) ? reinterpret_cast<IdlerID>(idleTimerID) : 0;
		} else {
			::KillTimer(MainHWND(), reinterpret_cast<uptr_t>(idler.idlerID));
			idler.idlerID = 0;
		}
		idler.state = idler.idlerID != 0;
	}
	return idler.state;
	*/
}

void Fl_Scintilla::update_v_scrollbar()
{
	//mVScrollBar->value(mTopLineNum, mNVisibleLines, 1, mNBufferLines+2);
	//mVScrollBar->linesize(3);
}

void Fl_Scintilla::update_h_scrollbar()
{
	//int sliderMax = max(longest_vline(), text_area.w + mHorizOffset);
	//mHScrollBar->value( mHorizOffset, text_area.w, 0, sliderMax );
}

void Fl_Scintilla::v_scrollbar_cb(Fl_Scrollbar* b, Fl_Scintilla* textD)
{
	//if (b->value() == textD->mTopLineNum) return;
	//textD->scroll(b->value(), textD->mHorizOffset);
}

void Fl_Scintilla::h_scrollbar_cb(Fl_Scrollbar* b, Fl_Scintilla* textD)
{
	//if (b->value() == textD->mHorizOffset) return;
	//textD->scroll(textD->mTopLineNum, b->value());
}

// call tip
class Fl_CallWindow : public Fl_Double_Window {
public:
	Fl_CallWindow(Scintilla::CallTip *ct, void dopush(void *x), void *x, int X, int Y, int W, int H, const char *l=0) : Fl_Double_Window(X, Y, W, H, l)
	{
		ct_ = ct;
		cb_notify_.callback = dopush;
		cb_notify_.data = x;

		swt_.type = 2;
		swt_.wid = this;
	}
	SCIWinType swt_;
protected:
	Scintilla::CallTip *ct_;
	void draw()
	{
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

	int handle(int e)
	{
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
