// Copyright 2015-2016 by cyantree <cyantree.guo@gmail.com>

#ifndef FL_SCINTILLA_H
#define FL_SCINTILLA_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

#include <cmath>
#include <stdexcept>
#include <new>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#undef PLAT_WIN
#include "Platform.h"

#include "ILexer.h"
#include "Scintilla.h"

#ifdef SCI_LEXER
#include "SciLexer.h"
#endif
#include "StringCopy.h"
#ifdef SCI_LEXER
#include "LexerModule.h"
#endif
#include "Position.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "XPM.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"
#include "CaseConvert.h"
#include "UniConversion.h"
#include "Selection.h"
#include "PositionCache.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "Editor.h"

#include "AutoComplete.h"
#include "ScintillaBase.h"

#ifdef SCI_LEXER
#include "ExternalLexer.h"
#endif

#include "PlatFLTK.h"
#include "Fl.H"
#include "fl_draw.H"
#include "Fl_Group.H"
#include "Fl_Widget.H"
#include "Fl_Scrollbar.H"
#include "Fl_Window.H"
#include "FL/Fl_Menu_Button.H"
#include "FL/Fl_Double_Window.H"
#include "FL/Fl_Box.H"

class Fl_Scintilla : public Fl_Group, public Scintilla::ScintillaBase	
{
public:
	explicit Fl_Scintilla(int X, int Y, int W, int H, const char* l = 0);
	virtual ~Fl_Scintilla();

	void SetNotify(void OnNotify(Scintilla::SCNotification *scn, void *data), void *data) 
	{
		cb_notify_.data = data;
		cb_notify_.callback = OnNotify;
	}

	sptr_t SendEditor(unsigned int iMessage, uptr_t wParam=0, sptr_t lParam=0)
	{
		return ScintillaBase::WndProc(iMessage, wParam, lParam);
	}

private:
	struct {
		void (*callback)(Scintilla::SCNotification *scn, void *data);
		void *data;
	} cb_notify_;

	void draw();
	int handle(int e);
	void resize(int X, int Y, int W, int H);

protected:
	ICONVConverter *ic_;
	char *ic_str_;
	int ic_str_len_;

	Scintilla::Surface *sw_;
	
	SCIWinType swt_;
	Fl_Scrollbar* mHScrollBar;
	Fl_Scrollbar* mVScrollBar;
	int scrollbar_width_;

	static void h_scrollbar_cb(Fl_Scrollbar* w, Fl_Scintilla* d);
	static void v_scrollbar_cb( Fl_Scrollbar* w, Fl_Scintilla* d);
	void update_v_scrollbar();
	void update_h_scrollbar();
	int scrollbar_width() const { return scrollbar_width_; }
	void scrollbar_width(int W) { scrollbar_width_ = W; }

	bool lastKeyDownConsumed;

	bool capturedMouse;

	unsigned int linesPerScroll;	///< Intellimouse support
	int wheelDelta; ///< Wheel delta from roll	

	virtual void Initialise();
	virtual void Finalise();
	int CodePageOfDocument() const;
	virtual bool ValidCodePage(int codePage) const
	{ 
		return codePage == 0 || codePage == SC_CP_UTF8 || codePage == 932 || codePage == 936 || codePage == 949 || codePage == 950 || codePage == 1361;
	}

	virtual void ScrollText(int linesToMove);
	virtual void SetVerticalScrollPos();
	virtual void SetHorizontalScrollPos();
	virtual bool ModifyScrollBars(int nMax, int nPage);

	virtual void Copy();
	virtual void CopyAllowLine();
	virtual bool CanPaste();
	virtual void CopyToClipboard(const Scintilla::SelectionText &selectedText);
	virtual void Paste();
	void DoPaste(const char *s);

	virtual void ClaimSelection() {}
	virtual void NotifyChange();
	virtual void NotifyFocus(bool focus);
	virtual void NotifyParent(Scintilla::SCNotification scn)
	{
		if ( cb_notify_.callback == NULL ) return;
		cb_notify_.callback(&scn, cb_notify_.data);
		/*
		scn.nmhdr.hwndFrom = MainHWND();
		scn.nmhdr.idFrom = GetCtrlID();
		::SendMessage(::GetParent(MainHWND()), WM_NOTIFY,
			GetCtrlID(), reinterpret_cast<LPARAM>(&scn));
		printf("%s\n", __FUNCTION__);
		*/
	}
	virtual bool FineTickerAvailable() { return true; }
	virtual bool FineTickerRunning(Scintilla::Editor::TickReason reason);
	virtual void FineTickerStart(Scintilla::Editor::TickReason reason, int millis, int tolerance);
	virtual void FineTickerCancel(Scintilla::Editor::TickReason reason);
	virtual bool SetIdle(bool on);
	virtual void SetMouseCapture(bool on);
	virtual bool HaveMouseCapture();
	virtual bool PaintContains(Scintilla::PRectangle rc);
	void FullPaint();

	char *drag_str_;
	int drag_str_size_;
	virtual bool DragThreshold(Scintilla::Point ptStart, Scintilla::Point ptNow);
	virtual void StartDrag();
	void DragEnter();
	int DragOver();
	void DragLeave();
	void Drop();
	void SaveDragData(const Scintilla::SelectionText &selectedText);

	virtual Scintilla::CaseFolder *CaseFolderForEncoding()
	{
		printf("%s\n", __FUNCTION__);
		return NULL;
	}
	virtual std::string CaseMapString(const std::string &s, int caseMapping)
	{
		printf("%s\n", __FUNCTION__);
		return 0;
	}

	Fl_Window *callwin_;
	virtual void CreateCallTipWindow(Scintilla::PRectangle rc);

	static void cb_popmenu(Fl_Widget *widget, void *data)
	{
		Fl_Scintilla *sci = (Fl_Scintilla *)(widget->user_data());
#if __FLTK_MACOSX__ || __FLTK_IPHONEOS__
		int cmd = (uintptr_t)data;
#else
		int cmd = (int)data;
#endif
		sci->Command(cmd);
	}
	virtual void AddToPopUp(const char *label, int cmd = 0, bool enabled = true)
	{
		Fl_Menu_Button *m = (Fl_Menu_Button*)(popup.GetID());
		m->user_data(this);
		if (!label[0]) {
			//m->add("_");
		} else if (enabled) {
			if ( strcmp(label, "Redo") == 0 ) m->add(label, 0, cb_popmenu, (void*)cmd, FL_MENU_DIVIDER); 
			else if ( strcmp(label, "Delete") == 0 ) m->add(label, 0, cb_popmenu, (void*)cmd, FL_MENU_DIVIDER);
			else m->add(label, 0, cb_popmenu, (void*)cmd);
			//m->add(label, 0, cb_popmenu, (void*)cmd);
		} else {
			if ( strcmp(label, "Redo") == 0 ) m->add(label, 0, cb_popmenu, (void*)cmd, FL_MENU_INACTIVE|FL_MENU_DIVIDER); 
			else if ( strcmp(label, "Delete") == 0 ) m->add(label, 0, cb_popmenu, (void*)cmd, FL_MENU_INACTIVE|FL_MENU_DIVIDER);
			else m->add(label, 0, cb_popmenu, (void*)cmd, FL_MENU_INACTIVE);
			//m->add(label, 0, 0, 0, FL_MENU_INACTIVE);
		}
	}

	virtual sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) { return 0; } // ok

	friend class ScintillaEditBase;

private:
	enum { invalidTimerID, standardTimerID, idleTimerID, fineTimerStart };
	unsigned char timeractive_[5];
	double timetick_[5];
	static void static_time_fun_1(void *x) { Fl_Scintilla *sci = (Fl_Scintilla*)x; sci->time_fun(0); }
	static void static_time_fun_2(void *x) { Fl_Scintilla *sci = (Fl_Scintilla*)x; sci->time_fun(1); }
	static void static_time_fun_3(void *x) { Fl_Scintilla *sci = (Fl_Scintilla*)x; sci->time_fun(2); }
	static void static_time_fun_4(void *x) { Fl_Scintilla *sci = (Fl_Scintilla*)x; sci->time_fun(3); }
	static void static_time_fun_5(void *x) { Fl_Scintilla *sci = (Fl_Scintilla*)x; sci->time_fun(4); }
	void time_fun(int index)
	{
		//return;
		if (index == idleTimerID && idler.state) {
			//SendMessage(MainHWND(), SC_WIN_IDLE, 0, 1);
		} else {
			if ( index == 0 ) TickFor(tickCaret);
			//else if ( index == 1 ) TickFor(tickScroll);
			else if ( index == 2 ) TickFor(tickWiden);
			else if ( index == 3 ) TickFor(tickDwell);
			else if ( index == 4 ) TickFor(tickPlatform);
			//TickFor(static_cast<TickReason>(index-tickCaret));
			//if( index == 0 )printf("%d\n", index);
		}
		if ( index == 0 ) Fl::repeat_timeout(timetick_[0], static_time_fun_1, this);
		if ( index == 1 ) Fl::repeat_timeout(timetick_[1], static_time_fun_2, this);
		if ( index == 2 ) Fl::repeat_timeout(timetick_[2], static_time_fun_3, this);
		if ( index == 3 ) Fl::repeat_timeout(timetick_[3], static_time_fun_4, this);
		if ( index == 4 ) Fl::repeat_timeout(timetick_[4], static_time_fun_5, this);
	}

	// For use in creating a system caret
	bool HasCaretSizeChanged() const;
	bool CreateSystemCaret();
	bool DestroySystemCaret();
	int sysCaretWidth;
	int sysCaretHeight;

	void MoveImeCarets(int offset);
	virtual void UpdateSystemCaret();

public:
	void CallTip_DoPush();

protected:
	virtual void SetTrackMouseLeaveEvent(bool on);
};

#endif
