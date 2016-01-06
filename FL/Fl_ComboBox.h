// commit by cyantree: the widget is independent

#ifndef Fl_ComboBox_H
#define Fl_ComboBox_H

#include "Fl.H"
#include "Fl_Group.H"
#include "Fl_Menu_Window.H"
#include "Fl_Input.H"
#include "Fl_Menu_Button.H"
#include "fl_draw.H"
#include "Fl_Browser.H"
#include <string.h>

class FL_EXPORT Fl_ComboBox : public Fl_Menu_
{
public:
	Fl_ComboBox(int X, int Y, int W, int H, const char *L = 0, int ListShowNum=-1) : Fl_Menu_(X,Y,W,H,L) {
		ListShowNum_ = ListShowNum;

		align(FL_ALIGN_LEFT);
		when(FL_WHEN_RELEASE);
		textfont(FL_HELVETICA);
		this->clear_visible_focus();

		inp_ = new innerInput((void*)this, inp_x(), inp_y(), inp_w(), inp_h());
		inp_->callback(inp_cb, (void*)this);
		inp_->box(FL_THIN_DOWN_BOX);		// cosmetic
		inp_->when(FL_WHEN_CHANGED|FL_WHEN_NOT_CHANGED);

		button_ = new innerButton(menu_x(), menu_y(), menu_w(), menu_h());
		button_->callback(button_cb, (void*)this);
		button_->box(FL_FLAT_BOX);
	}

	~Fl_ComboBox()
	{
		if ( inp_ != NULL ) delete inp_;
		if ( button_ != NULL ) delete button_;
	}

	/** Gets the read-only state of the input field.
	  \return non-zero if this widget is read-only */
	int readonly() const {
		return inp_->type() & FL_INPUT_READONLY;
	}
	/** Sets the read-only state of the input field.
	  \param [in] b if \p b is 0, the text in this widget can be edited by the user */
	void readonly(int b) {
		if (b) inp_->type((uchar)(inp_->type() | FL_INPUT_READONLY));
		else inp_->type((uchar)(inp_->type() & ~FL_INPUT_READONLY));
	}

	int listshownum() { return ListShowNum_; }
	void listshownum(int n) { ListShowNum_ = n; }

	/**
	  Gets the index of the last item chosen by the user.
	  The index is zero initially.
	 */
	int value() const {
		return Fl_Menu_::value();
	}

	/**
	Sets the currently selected value using the index into the menu item array.
	Changing the selected value causes a redraw().
	\param[in] v index of value in the menu item array.
	\returns non-zero if the new value is different to the old one.
	*/
	int value(int v) {
		if (v == -1) return value((const Fl_Menu_Item *)0);
		if (v < 0 || v >= (size() - 1)) return 0;
		if (!Fl_Menu_::value(v)) return 0;
		inp_->value(text(v));
		redraw();
		return 1;
	}

	/**
	Sets the currently selected value using a pointer to menu item.
	Changing the selected value causes a redraw().
	\param[in] v pointer to menu item in the menu item array.
	\returns non-zero if the new value is different to the old one.
	*/
	int value(const Fl_Menu_Item* v) {
		if (!Fl_Menu_::value(v)) return 0;
		redraw();
		return 1;
	}

	Fl_Button *innerbutton() { return button_; }
	Fl_Input *innerinput() {	return inp_; }

protected:
	class innerButton : public Fl_Button
	{
		void draw() {
			draw_box(FL_THIN_UP_BOX, color());
			fl_color(active_r() ? labelcolor() : fl_inactive(labelcolor()));
			int xc = x()+w()/2, yc=y()+h()/2;
			fl_polygon(xc-5,yc-3,xc+5,yc-3,xc,yc+3);
			if (Fl::focus() == this)
				//draw_focus();
				draw_focus(box(),x()+1,y()+1,w()-2,h()-2);
		}
	public:
		innerButton(int X,int Y,int W,int H,const char*L=0) : Fl_Button(X, Y, W, H, L) { box(FL_UP_BOX); }
	};
	innerButton *button_;

	class innerInput : public Fl_Input
	{
	public:
		innerInput(void *pv, int X, int Y, int W, int H, const char *l=0) : Fl_Input(X, Y, W, H, l)
		{
			v_ = pv;
			issel_ = 1;
		}
		int handle(int event)
		{
			switch (event) {
			case FL_PUSH:
				if ( Fl::event_clicks() == 0 ) Fl_Input::handle(event);
				if ( ! readonly() ) {
					if ( issel_ == 1 ) {
						position(0, size());
						issel_ = 0;
					}
					if (active_r() && window()) window()->cursor(FL_CURSOR_INSERT);
				} else {
					if (active_r() && window()) window()->cursor(FL_CURSOR_DEFAULT);
				}
				return 1;
			case FL_RELEASE:
				Fl_Input::handle(event);
				if ( readonly() ) {
					button_cb(0, v_);
				}
				return 1;
			case FL_ENTER:
			case FL_MOVE:
				if ( readonly() ) {
					if (active_r() && window()) window()->cursor(FL_CURSOR_DEFAULT);
				} else {
					if (active_r() && window()) window()->cursor(FL_CURSOR_INSERT);
				}
				return 1;
			case FL_UNFOCUS:
				issel_ = 1;
				break;
			}
			return Fl_Input::handle(event);
		}
	private:
		void *v_;
		unsigned char issel_;
	};
	innerInput *inp_;

	static void inp_cb(Fl_Widget*, void *data) {
		Fl_ComboBox *o=(Fl_ComboBox *)data;
		Fl_Widget_Tracker wp(o);
		if (o->inp_->changed()) {
			o->Fl_Widget::set_changed();
			if (o->when() & (FL_WHEN_CHANGED|FL_WHEN_RELEASE))
				o->do_callback();
		} else {
			o->Fl_Widget::clear_changed();
			if (o->when() & FL_WHEN_NOT_CHANGED)
				o->do_callback();
		}

		if (wp.deleted()) return;

		if (o->callback() != default_callback)
			o->Fl_Widget::clear_changed();
	}

	static void button_cb(Fl_Widget*, void *data)
	{
		Fl_ComboBox *o=(Fl_ComboBox *)data;
		o->button_cb_i();
	}
	void button_cb_i()
	{
		Fl_Widget_Tracker wp(this);

		const Fl_Menu_Item* v;
		if (Fl::scheme() || fl_contrast(textcolor(), FL_BACKGROUND2_COLOR) != textcolor()) {
			//v = menu()->pulldown(x(), y(), w(), h(), mvalue(), this);
			//if ( ! inp_->readonly() ) {
				inp_->take_focus();
				inp_->position(0, inp_->size());
			//}
			v = pulldown(x(), y(), w(), h(), mvalue(), this, ListShowNum_);
		} else {
			// In order to preserve the old look-n-feel of "white" menus,
			// temporarily override the color() of this widget...
			Fl_Color c = color();
			color(FL_BACKGROUND2_COLOR);
			//v = menu()->pulldown(x(), y(), w(), h(), mvalue(), this);
			//if ( ! inp_->readonly() ) {
				inp_->take_focus();
				inp_->position(0, inp_->size());
			//}
			v = pulldown(x(), y(), w(), h(), mvalue(), this, ListShowNum_);
			color(c);
		}
		if (!v || v->submenu()) {
			if ( inp_->readonly() ) button_->take_focus();
			return;
		}
		if (v != mvalue()) redraw();
		picked(v);
		inp_->value(v->label());
		if ( ! inp_->readonly() ) {
			inp_->take_focus();
			inp_->position(0, inp_->size());
		} else {
			button_->take_focus();
		}
	}

	// Custom resize behavior -- input stretches, menu button doesn't
	inline int inp_x() { return(x() + Fl::box_dx(box()) ); }
	inline int inp_y() { return(y() + Fl::box_dy(box()) +1); }
	inline int inp_w() { return(w() - Fl::box_dw(box()) - 20); }
	inline int inp_h() { return(h() - Fl::box_dh(box()) -2); }

	inline int menu_x() { return(x() + w() - 20 - Fl::box_dx(box())); }
	inline int menu_y() { return(y() + Fl::box_dy(box())); }
	inline int menu_w() { return(20); }
	inline int menu_h() { return(h() - Fl::box_dh(box())); }

	// Emulates the Forms choice widget.  This is almost exactly the same
	// as an Fl_Menu_Button.  The only difference is the appearance of the
	// button: it draws the text of the current pick and a down-arrow.
	void draw() {}

protected:
	class innerBrower : public Fl_Browser
	{
	public:
		innerBrower(int X,int Y,int W,int H,const char *L=0) : Fl_Browser(X,Y,W,H,L)
		{
			box(FL_BORDER_BOX);
			type(FL_HOLD_BROWSER);
			has_scrollbar(Fl_Browser_::VERTICAL);
		}

		int handle(int event)
		{
			int X, Y, W, H;
			int my;
			void* l;

			switch (event) {
			case FL_MOVE:
				bbox(X, Y, W, H);
				if (!Fl::event_inside(X, Y+1, W, H-2)) return 0;
				my = Fl::event_y();
				l = find_item(my);
				select_only(l, 0);
				return 1;
			case FL_DRAG:
				if (!Fl::event_inside(x(), y(), w(), h())) return 0;
				return Fl_Browser::handle(event);
			case FL_PUSH:
				return Fl_Browser::handle(event);
			}

			return Fl_Browser::handle(event);
		}

		void *selection() const
		{
			return Fl_Browser::selection();
		}

		int lineno(void *item) const
		{
			return Fl_Browser::lineno(item);
		}

		int incr_height() const { return Fl_Browser::incr_height(); }

		void *item_first() const { return Fl_Browser::item_first(); }
		void *item_next(void *item) const { return Fl_Browser::item_next(item); }

		void bbox(int &X,int &Y,int &W,int &H) const { return Fl_Browser_::bbox(X, Y, W, H); }
	};

#define STATE_INITIAL   0 // no mouse up or down since popup() called
#define STATE_PUSH      1 // mouse has been pushed on a normal item
#define STATE_DONE      2 // exit the popup, the current item was picked
#define STATE_DONE_1    3 // exit the popup, the current item was picked
	class menuwindow : public Fl_Menu_Window
	{
	public:
		int handle(int e)
		{
			void *item;
			int lineno;
			const Fl_Menu_Item* mm;
			int i;
			int tl;
			switch (e) {
			case FL_KEYBOARD:
				move_ = 1;
				switch (Fl::event_key()) {
				case FL_Enter:
				case FL_KP_Enter:
				case ' ':
					if ( Fl::event_key() == ' ' ) {
						if ( ! input_->readonly() ) {
							input_->handle(e);
							return 1;
						}
					}

					/*
					if ( Fl::event_key() == FL_Enter || Fl::event_key() == FL_KP_Enter ) {
						if ( ! input_->readonly() ) {
							select_item = NULL;
							state_ = STATE_DONE_1;
							return 1;
						}
					}
					*/

					item = browser_->selection();
					if ( item == 0 ) {
						select_item = NULL;
						state_ = STATE_DONE;
						return 1;
					}
					select_item = NULL;
					lineno = browser_->lineno(item);
					for (mm=menu->first(), i=0; mm->text; i++, mm = mm->next()) {
						if ( i == lineno-1 ) {
							select_item = mm;
							break;
						}
					}
					state_ = STATE_DONE;
					return 1;
				case FL_Escape:
					select_item = NULL;
					state_ = STATE_DONE;
					return 1;
				case FL_Page_Up:
					item = browser_->selection();
					if ( item == 0 ) return 1;
					lineno = browser_->lineno(item);
					if ( lineno == 1 ) return 1;
					if ( lineno-listshownum_ < 1 ) {
						browser_->select(1);
						browser_->position(0);
					} else {
						browser_->select(lineno-listshownum_+1);
						browser_->position((lineno-listshownum_)*itemheight);
					}
					return 1;
				case FL_Page_Down:
					item = browser_->selection();
					if ( item == 0 ) return 1;
					lineno = browser_->lineno(item);
					if ( lineno == browser_->size() ) return 1;
					if ( lineno > browser_->size() - listshownum_ ) {
						browser_->select(browser_->size());
						browser_->position((browser_->size() - listshownum_)*itemheight);
					} else {
						browser_->select(lineno+listshownum_-1);
						browser_->position((lineno-1)*itemheight);
					}
					return 1;
				case FL_Home:
					if ( input_->readonly() ) {
						Fl_Window::handle(e);
						browser_->select(1);
						return 1;
					}
					input_->handle(e);
					return 1;
				case FL_End:
					if ( input_->readonly() ) {
						Fl_Window::handle(e);
						browser_->select(browser_->size());
						return 1;
					}
					input_->handle(e);
					return 1;
				case FL_Down:
					item = browser_->selection();
					if ( item == 0 ) return 1;
					lineno = browser_->lineno(item);
					if ( lineno == browser_->size() ) return 1;
					browser_->select(lineno+1);
					tl = browser_->position() / itemheight;
					if ( lineno+1 > tl+listshownum_ ) browser_->position((tl+1)*itemheight);
					return 1;
				case FL_Up:
					item = browser_->selection();
					if ( item == 0 ) return 1;
					lineno = browser_->lineno(item);
					if ( lineno == 1 ) return 1;
					browser_->select(lineno-1);
					tl = browser_->position() / itemheight;
					if ( lineno == tl ) browser_->position((tl-1)*itemheight);
					return 1;
				default:
					if ( ! input_->readonly() ) {
						input_->handle(e);
						return 1;
					} else {
						return Fl_Window::handle(e);
					}
				}
			case FL_KEYUP:
				i = Fl::event_key();
				if ( i == FL_Alt_L || i == FL_Alt_R ) {
					select_item = NULL;
					state_ = STATE_DONE;
					return 1;
				}
				break;
			case FL_MOVE:
			case FL_DRAG:
			case FL_PUSH:
				browser_->handle(e);
				return 1;
				/*
			case FL_PUSH:
			{
				browser_->handle(e);

				selected = -1;

				int X, Y, W, H;
				browser_->bbox(X, Y, W, H);
				if (!Fl::event_inside(X, Y, W, H)) return 1;

				item = browser_->selection();
				if ( item == 0 ) return 1;
				selected = browser_->lineno(item);
				//printf("push:%d\n", selected);

				return 1;
			}
			return 1;
			*/
			case FL_UNFOCUS:
#if __FLTK_WIN32__
				select_item = NULL;
				state_ = STATE_DONE;
				return 1;
#else
				return 0;
#endif
			case FL_RELEASE:
				int X, Y, W, H;
				browser_->bbox(X, Y, W, H);
				if (!Fl::event_inside(X, Y, w(), h())) {
					select_item = NULL;
					state_ = STATE_DONE;
					return 1;
				}

				if ( browser_->size() != listshownum_ ) {
					if (!Fl::event_inside(X, Y, w()-browser_->scrollbar_width(), h())) {
						return Fl_Window::handle(e);
					}
				}

				item = browser_->selection();
				if ( item == 0 ) {
					select_item = NULL;
					state_ = STATE_DONE;
					return 1;
				}
				select_item = NULL;
				lineno = browser_->lineno(item);
				for (mm=menu->first(), i=0; mm->text; i++, mm = mm->next()) {
					if ( i == lineno-1 ) {
						select_item = mm;
						break;
					}
				}
				state_ = STATE_DONE;
				return 1;
			}
			return Fl_Menu_Window::handle(e);
		}

		menuwindow(const Fl_Menu_Item* m, int X, int Y, int Wp, int Hp, const Fl_Menu_Item* picked, const Fl_Menu_* pbutton, int listshownum, Fl_Input *input) : Fl_Menu_Window(X, Y, Wp, Hp, 0)
		{
			input_ = input;
			select_item = picked;
			listshownum_ = listshownum;
			move_ = 0;

			int scr_x, scr_y, scr_w, scr_h;
			int ty = Y;
			int i;
			int numitems;

			Fl::screen_work_area(scr_x, scr_y, scr_w, scr_h);

			browser_ = new innerBrower(0, 0, 1, 1);
			if (m) {
				const Fl_Menu_Item* mm;
				for (mm=m->first(), i=0; mm->text; i++, mm = mm->next()) browser_->add(mm->label());
			}
			browser_->position(0);

			end();
			fl_cursor(FL_CURSOR_ARROW); // add by cyantree
			set_modal();
			clear_border();
			box(FL_THIN_DOWN_BOX);
			menu = m;
			if (m) m = m->first(); // find the first item that needs to be rendered
			color(pbutton && !Fl::scheme() ? pbutton->color() : FL_GRAY);
			selected = -1;
			{
				i = 0;
				if (m) for (const Fl_Menu_Item* m1=m; ; m1 = m1->next(), i++) {
					if (picked) {
						if (m1 == picked) {
							selected = i;
							picked = 0;
						} else if (m1 > picked) {
							selected = i-1;
							picked = 0;
							Wp = Hp = 0;
						}
					}
					if (!m1->text) break;
				}
				numitems = i;
			}

			itemheight = 1;
			if (m) {
				itemheight = browser_->incr_height();
			}

			int n;
			if ( selected == -1 ) {
				picked = 0;
				browser_->select(1);
			} else {
				browser_->select(selected+1);
				n = listshownum_;
				if ( listshownum_ < 1 ) n = browser_->size();
				if ( selected > browser_->size() - n ) {
					browser_->position((browser_->size() - n)*itemheight);
				} else {
					browser_->topline(selected+1);
				}
			}

			if (X < scr_x) X = scr_x;
			// this change improves popup submenu positioning at right screen edge,
			// but it makes right_edge argument useless
			if (X > scr_x+scr_w-Wp) X = scr_x+scr_w-Wp;
			x(X+1);
			w(Wp-2);
			if ( listshownum < 1 ) {
				h(numitems ? itemheight*numitems+4 : 0);
				listshownum_ = browser_->size();
			} else {
				if ( listshownum >= numitems ) h(numitems ? itemheight*numitems+4 : 0);
				else h(numitems ? itemheight*listshownum+4 : 0);
			}

			ty = Y+Hp;
			// if the menu hits the bottom of the screen, we try to draw
			// it above the menubar instead. We will not adjust any menu
			// that has a selected item.
			if (ty+h()>scr_y+scr_h && ty-h()>=scr_y) {
				if (Hp>1) {
					// if we know the height of the Fl_Menu_, use it
					ty = ty-Hp-h();
				} else {
					// draw the menu to the right
					ty = ty-h()+itemheight+Fl::box_dy(box());
				}
			}

			if (m) {
				y(ty);
			} else {
				y(ty-2);
				h(20);
			}

			browser_->resize(0, 0, w(), h());
		}

		~menuwindow()
		{
			if ( browser_ != NULL ) delete browser_;
			hide();
		}

		int itemheight;	// zero == menubar
		int selected;
		const Fl_Menu_Item* menu;
		innerBrower *browser_;
		unsigned char state_;
		const Fl_Menu_Item* select_item;
		int listshownum_;
		Fl_Input *input_;
		unsigned char move_;
	};

	static int owenevent(int event, Fl_Window *w)
	{
		if ( event == FL_UNFOCUS ) {
			Fl_Window *w = Fl::grab();
			int r;
			Fl::grab(0);
			r = Fl::handle_(event, w);
			Fl::grab(w);
			return r;
		}

		return Fl::handle_(event, w);
	}

	const Fl_Menu_Item* pulldown(int X, int Y, int W, int H, const Fl_Menu_Item* initial_item, const Fl_Menu_* pbutton, int listshownum) const
	{
		unsigned char flag = 1;

		Fl_Group::current(0); // fix possible user error...

		int wx=X, wy=Y;
		for (Fl_Window* w = pbutton->window(); w; w = w->window()) {
			wx += w->x();
			wy += w->y();
		}

		menuwindow mw(this->menu(), wx, wy, W, H, initial_item, pbutton, listshownum, inp_);
		mw.state_ = STATE_INITIAL;

		char *old_s, *s;
		s = (char*)inp_->value();
		old_s = (char *)malloc(strlen(s)+1);
		strcpy(old_s, s);

		void *item;
		int lineno=0, l=0;
		item = mw.browser_->selection();
		if ( item == 0 ) {
			inp_->value("");
		} else {
			lineno = mw.browser_->lineno(item);
			inp_->value(mw.browser_->text(lineno));
			inp_->position(0, inp_->size());
		}

		Fl::event_dispatch(owenevent);
		Fl::grab(mw);

		// the main loop, runs until p.state goes to DONE_STATE:
		for (;;) {
			if (!mw.shown()) mw.show();

			Fl::wait();
			if (mw.state_ == STATE_DONE) break;
			if (mw.state_ == STATE_DONE_1 ) {
				flag = 0;
				break;
			}

			if ( mw.move_ == 1 ) {
				item = mw.browser_->selection();
				if ( item == 0 ) {
					inp_->value("");
				} else {
					l = mw.browser_->lineno(item);
					if ( l != lineno ) {
						lineno = l;
						inp_->value(mw.browser_->text(lineno));
						inp_->position(0, inp_->size());
					}
				}
				mw.move_ = 0;
			}
		}

		const Fl_Menu_Item* m = mw.select_item;
		mw.hide();
		Fl::event_dispatch(NULL);
		Fl::grab(0);

		if ( m == NULL ) {
			if ( 1 == flag ) inp_->value(old_s);
			inp_->position(0, inp_->size());
		}
		free(old_s);

		return m;
	}

private:
	int ListShowNum_;
};

#endif // !Fl_ComboBox_H
