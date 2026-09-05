/*-----------------------------------------------------------------------------

	Timer2.cpp

	Timer version 2 implementation.

	2000.6.15. KJTINC

-----------------------------------------------------------------------------*/

#include "timer2.h"
#include <cstring>
#include <cstdlib>

//----------------------------------------------------------------------------
// This manager keeps the API it has always had - DWORD milliseconds in,
// timer_id_t out, INVALID_TID for "no timer" - and nothing outside this
// file changed. What changed is underneath: the reference time and the
// interval are a MonotonicClock::TimePoint and a
// MonotonicClock::Duration instead of two DWORD tick counts, so the
// elapsed test in Execute() is a comparison of typed durations and no
// longer depends on 32-bit unsigned arithmetic wrapping the same way on
// both sides. The firing rule is unchanged in every other respect: a
// timer fires when at least its interval has elapsed, and the reference
// time then becomes the current time of that Execute() call rather than
// previous + interval, so a late frame does not build up a debt of
// missed fires.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Global instance
//----------------------------------------------------------------------------
C_TIMER2		gC_timer2;

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
C_TIMER2::C_TIMER2()
{
	// Nothing is allocated here. gC_timer2 is a global, and the realloc()'d
	// array this replaced grew lazily on the first Add(), where a failed
	// allocation is answered with INVALID_TID rather than a terminate()
	// during static initialisation.
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
C_TIMER2::~C_TIMER2()
{
	m_timer_queue.clear();
}

//----------------------------------------------------------------------------
// Find - the live timer with this id, or NULL (private)
//----------------------------------------------------------------------------
C_TIMER2::S_TIMERUNIT *
C_TIMER2::Find(timer_id_t tid)
{
	if (tid < 0 || (size_t)tid >= m_timer_queue.size())
	{
		return NULL;
	}

	S_TIMERUNIT* pUnit = &m_timer_queue[(size_t)tid];
	if (pUnit->tid == INVALID_TID)
	{
		return NULL;
	}

	return pUnit;
}

//----------------------------------------------------------------------------
// Add - Add a new timer
//----------------------------------------------------------------------------
timer_id_t
C_TIMER2::Add(DWORD dw_millisec, void (*fp_proc)(void))
{
	// The slot's index is the id, and slots are never reused.
	const timer_id_t tid = (timer_id_t)m_timer_queue.size();

	S_TIMERUNIT unit;

	// Initialize the timer unit
	unit.tid = tid;
	unit.fp_proc = fp_proc;
	unit.d_interval = MonotonicClock::Millis(dw_millisec);
	unit.tp_prev = MonotonicClock::Now();
	unit.bl_pause = 1; // Start paused

	try
	{
		m_timer_queue.push_back(unit);
	}
	catch (...)
	{
		// The realloc()'d array this replaced answered a failed
		// allocation with INVALID_TID rather than an exception, and
		// callers (VS_UI) test the returned id, so keep that contract.
		return INVALID_TID;
	}

	return tid;
}

//----------------------------------------------------------------------------
// Delete - Delete a timer
//----------------------------------------------------------------------------
bool
C_TIMER2::Delete(timer_id_t &tid)
{
	S_TIMERUNIT* pUnit = Find(tid);
	if (pUnit == NULL)
	{
		return false;
	}

	// Mark as deleted
	pUnit->tid = INVALID_TID;
	pUnit->fp_proc = NULL;
	tid = INVALID_TID;

	return true;
}

//----------------------------------------------------------------------------
// Execute - Execute all active timers
//----------------------------------------------------------------------------
void
C_TIMER2::Execute()
{
	const MonotonicClock::TimePoint tp_now = MonotonicClock::Now();

	// size() is re-read every iteration, as the old m_id_generator bound
	// was: a callback may Add() a timer, and the new one is reached in
	// this same pass - where it is skipped, because Add() starts a timer
	// paused.
	for (size_t i = 0; i < m_timer_queue.size(); i++)
	{
		// Skip invalid or paused timers
		if (m_timer_queue[i].tid == INVALID_TID || m_timer_queue[i].bl_pause)
		{
			continue;
		}

		// Check if timer has elapsed. The rep behind these is 64-bit
		// milliseconds, so there is no 32-bit wrap for a sum to carry
		// past; the typing is what keeps a millisecond count from being
		// compared with anything else.
		if (tp_now - m_timer_queue[i].tp_prev >= m_timer_queue[i].d_interval)
		{
			// Execute the timer callback
			if (m_timer_queue[i].fp_proc != NULL)
			{
				Execute(&m_timer_queue[i]);
			}

			// Update the reference time. Addressed by index rather than
			// through a pointer held across the callback, because a
			// callback that calls Add() can reallocate the queue.
			m_timer_queue[i].tp_prev = tp_now;
		}
	}
}

//----------------------------------------------------------------------------
// Execute - Execute a single timer unit (private)
//----------------------------------------------------------------------------
void
C_TIMER2::Execute(S_TIMERUNIT *pS_timerunit)
{
	if (pS_timerunit != NULL && pS_timerunit->fp_proc != NULL)
	{
		pS_timerunit->fp_proc();
	}
}

//----------------------------------------------------------------------------
// Refresh - Reset a timer's tick count
//----------------------------------------------------------------------------
void
C_TIMER2::Refresh(timer_id_t tid)
{
	S_TIMERUNIT* pUnit = Find(tid);
	if (pUnit != NULL)
	{
		pUnit->tp_prev = MonotonicClock::Now();
	}
}

//----------------------------------------------------------------------------
// Pause - Pause a timer
//----------------------------------------------------------------------------
void
C_TIMER2::Pause(timer_id_t tid)
{
	S_TIMERUNIT* pUnit = Find(tid);
	if (pUnit != NULL)
	{
		pUnit->bl_pause = 1;
	}
}

//----------------------------------------------------------------------------
// Continue - Resume a paused timer
//----------------------------------------------------------------------------
void
C_TIMER2::Continue(timer_id_t tid)
{
	S_TIMERUNIT* pUnit = Find(tid);
	if (pUnit != NULL)
	{
		pUnit->bl_pause = 0;
		pUnit->tp_prev = MonotonicClock::Now();
	}
}

//----------------------------------------------------------------------------
// ResetSpeed - Change a timer's interval
//----------------------------------------------------------------------------
void
C_TIMER2::ResetSpeed(timer_id_t tid, DWORD millisec)
{
	S_TIMERUNIT* pUnit = Find(tid);
	if (pUnit != NULL)
	{
		pUnit->d_interval = MonotonicClock::Millis(millisec);
		pUnit->tp_prev = MonotonicClock::Now();
	}
}
