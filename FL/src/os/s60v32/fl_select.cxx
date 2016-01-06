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

#include <e32base.h>
#include "fl_select.h"

static RThread fl_select_s60_thread_;
static RMutex fl_select_s60_loop_mutex_;
static RMutex fl_select_s60_req_mutex_;
static TBool fl_select_s60_thread_keep_running_;
static struct timeval *fl_s60_t_;
static fd_set *fl_s60_rfds_, *fl_s60_wfds_, *fl_s60_efds_;
static int fl_s60_nfds_;
static int fl_select_s60_result_;
static int fl_s60_select_finished_;

TInt fl_select_s60_thread_func(TAny*)
{
	while (fl_select_s60_thread_keep_running_) {
		if (fl_s60_nfds_ > 0) {
			fl_select_s60_loop_mutex_.Wait();
			fl_select_s60_result_ = select (fl_s60_nfds_, fl_s60_rfds_, fl_s60_wfds_, fl_s60_efds_, fl_s60_t_);
			fl_s60_nfds_ = 0;
			fl_s60_select_finished_ = 1;
			fl_select_s60_loop_mutex_.Signal();
		} else {
			User::After(1000);
		}
	}
	return 0;
}

void fl_select_s60_start_thread()
{
	fl_select_s60_thread_keep_running_ = 1;
	User::LeaveIfError(fl_select_s60_thread_.Create (_L("Fl_Select_Thread"), fl_select_s60_thread_func, 65536, &User::Heap(), 0));
	User::LeaveIfError(fl_select_s60_loop_mutex_.CreateLocal());
	User::LeaveIfError(fl_select_s60_req_mutex_.CreateLocal());
	// fl_select_s60_mutex_.Wait();
	fl_select_s60_thread_.Resume();
}

void fl_select_s60_stop_thread()
{
	fl_select_s60_thread_keep_running_ = 0;
	TRequestStatus status;
	fl_select_s60_thread_.Rendezvous(status);
	User::WaitForRequest(status);
	fl_select_s60_thread_.Close();
	fl_select_s60_loop_mutex_.Close();
	fl_select_s60_req_mutex_.Close();
}

int fl_select_s60(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *t)
{
	fl_select_s60_req_mutex_.Wait();
	fl_select_s60_loop_mutex_.Wait();
	fl_s60_nfds_ = nfds;
	fl_s60_rfds_ = rfds;
	fl_s60_wfds_ = wfds;
	fl_s60_efds_ = efds;
	fl_s60_t_ = t;
	fl_s60_select_finished_ = 0;
	fl_select_s60_loop_mutex_.Signal();
	while (!fl_s60_select_finished_) {
		fl_select_s60_loop_mutex_.Wait();
		fl_select_s60_loop_mutex_.Signal();
	}
	int result = fl_select_s60_result_;
	fl_select_s60_req_mutex_.Signal();
	return result;
}
