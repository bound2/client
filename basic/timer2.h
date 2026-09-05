/*-----------------------------------------------------------------------------

	Timer2.h

	Timer version 2.

	2000.6.15. KJTINC

-----------------------------------------------------------------------------*/

#ifndef __TIMER2__
#define __TIMER2__

// MonotonicClock.h first: it includes <chrono> ahead of the <windows.h>
// that Basics.h drags in, and this project does not define NOMINMAX.
#include "MonotonicClock.h"
#include "Basics.h"

#include <vector>

typedef long timer_id_t;

#define INVALID_TID						-1
#define INVALID_INDEX					-1

//----------------------------------------------------------------------------
// Class Timer - Timer Manager.
//----------------------------------------------------------------------------
class C_TIMER2
{
private:
	//
	// One timer.
	//
	// tp_prev and d_interval used to be a pair of DWORD millisecond
	// counts read off GetTickCount(). Their 64-bit millisecond rep has no
	// wrap for a "previous + delay" sum to carry past, and the typing
	// keeps a millisecond count apart from every other DWORD
	// (MonotonicClock.h). The public API still speaks DWORD milliseconds
	// and is unchanged.
	//
	struct S_TIMERUNIT
	{
		DWORD								bl_pause;	// being paused ?
		MonotonicClock::TimePoint		tp_prev;		// reference time: last fire, or last reset
		MonotonicClock::Duration		d_interval;	// how long between fires

		//
		// tid
		//
		//
		timer_id_t		tid;
		void				(*fp_proc)(void);	// method to execute
	};

	//
	// Timer queue.
	//
	// A slot's index IS its tid and a deleted slot is never reused, so
	// this only ever grows and every id stays valid for the life of the
	// manager. It used to be a hand-grown realloc()'d array that grew by
	// eight slots at a time, which cannot hold a type with a non-trivial
	// default constructor; the vector grows by its own policy instead,
	// still allocates nothing until the first Add(), and the
	// allocation-failure return of INVALID_TID is preserved below.
	//
	std::vector<S_TIMERUNIT>	m_timer_queue;

	void	Execute(S_TIMERUNIT *pS_timerunit);

	//
	// The one place the "is this a live timer?" test lives. Returns NULL
	// for an out-of-range id and for a deleted slot, which is exactly
	// the pair of checks every mutator below used to spell out.
	//
	S_TIMERUNIT *	Find(timer_id_t tid);

public:
	C_TIMER2();
	~C_TIMER2();

	void	Execute();
	void	Refresh(timer_id_t tid);
	void	Pause(timer_id_t tid);
	void	Continue(timer_id_t tid);
	bool	Delete(timer_id_t &tid);
	void	ResetSpeed(timer_id_t tid, DWORD millisec);

	timer_id_t Add(DWORD dw_millisec, void (*fp_proc)(void));
};

extern C_TIMER2		gC_timer2;

#endif