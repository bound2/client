//----------------------------------------------------------------------
// MonotonicClock.h
//----------------------------------------------------------------------
//
// One monotonic clock for the whole client, and the seam that lets a
// test drive it.
//
// The timing code in this tree is written against GetTickCount() and
// timeGetTime(). Both are 32-bit millisecond counters, and on this
// build they are not the same counter - see "Which tick is which"
// below. Two things follow from that, and this header exists to end
// both of them one call site at a time
// (docs/cpp17-cpp20-compatibility-assessment-2026-09-04.md,
// modernization backlog priority 5):
//
//   1. The counter wraps. It is 32 bits of milliseconds, so it returns
//      to zero every 49.7 days, and the shape the client writes its
//      timers in - "previous + delay <= now" - is not wrap-safe. Once
//      previous + delay carries past 2^32 the sum lands *behind* now and
//      the condition is true on every frame from then on: the timer
//      free-runs instead of waiting. The wrap-safe spelling of the same
//      test is "now - previous >= delay", which is why the subtraction
//      form appears in some places and the addition form in others, with
//      nothing marking which is which. A 64-bit millisecond rep removes
//      the trap rather than the choice: "previous + delay <= now" still
//      compiles over a TimePoint and a Duration, but there is no 2^32
//      for the sum to carry past, so both spellings are correct.
//
//   2. A DWORD carries no unit. A millisecond count, a second count and
//      a frame count are the same type, so a mismatch is not a
//      compile error, only a bug that shows up as a wrong rate.
//
// Which tick is which
// -------------------
// Off Windows, Platform.h defines both GetTickCount() and timeGetTime()
// as platform_get_ticks(), which is SDL_GetTicks(). On PLATFORM_WINDOWS
// - the live build - only timeGetTime() is redefined that way;
// GetTickCount() is the real kernel32 function. So the client already
// reads two different counters:
//
//   timeGetTime()   -> platform_get_ticks() -> SDL_GetTicks():
//                      milliseconds since SDL was initialised, 1 ms
//                      resolution.
//   GetTickCount()  -> kernel32: milliseconds since boot, advancing in
//                      steps of the system timer tick (about 15.6 ms).
//
// They share neither epoch nor resolution, and nothing in the source
// marks which sites use which. That is worth knowing before converting
// anything: a site moved off GetTickCount() stops being quantised to
// the 15.6 ms system tick and starts measuring the interval it actually
// asked for, which is a (small) change in when it fires and has to be
// stated when it is made.
//
// What this offers
// ----------------
// Now() returns a std::chrono::steady_clock time point truncated to
// milliseconds - the resolution the code being migrated actually had -
// and Duration is std::chrono::milliseconds. Neither can be confused
// with the other, and neither can be confused with a count of frames.
//
// LegacyTicks() and the epoch question
// ------------------------------------
// A migration that has to happen one call site at a time cannot afford
// two clocks that disagree: while a class has some sites converted and
// some still reading a raw tick, any comparison between them has to keep
// meaning what it meant. So LegacyTicks() does NOT derive its value from
// Now(). With no test source installed it *is* platform_get_ticks() -
// the same call, the same epoch, the same value timeGetTime() would have
// returned on that line - so a class half converted off timeGetTime()
// stays correct. (It is deliberately not Win32 GetTickCount(): that
// counter is not available on every platform this tree builds for, and
// platform_get_ticks() is the one the backlog item names.)
//
// Now() and LegacyTicks() are therefore two monotonic clocks that share
// no epoch, and mixing a TimePoint with a DWORD tick is meaningless; the
// type system already refuses to do it.
//
// Under an injected test source LegacyTicks() returns the low 32 bits of
// that source's millisecond count instead, so a test can drive the
// legacy tick across its wrap deliberately rather than waiting 49.7 days
// for one. It calls the source itself rather than going through Now(),
// so a source that advances on every call is read once by each.
//
// The test seam
// -------------
// SetTestSource() installs a function that Now() calls in place of
// steady_clock::now(). It is the whole reason timing code can be tested
// here at all: real elapsed time in a unit test is either a sleep or a
// flake. ScopedTestSource is the RAII form and is what tests should use,
// because it restores the previous source even when a check fails.
//
// The pointer is atomic because it is read on whatever thread happens to
// call Now(); it is only ever written by a test, on one thread, before
// the code under test runs.
//
//----------------------------------------------------------------------

#ifndef __MONOTONIC_CLOCK_H__
#define __MONOTONIC_CLOCK_H__

// <chrono> ahead of Platform.h: Platform.h drags in <windows.h> on
// PLATFORM_WINDOWS, and this project does not define NOMINMAX. That only
// helps a translation unit that sees this header before any other one
// reaching <windows.h> - timer2.h includes it first for that reason -
// and MSVC's <chrono> guards its min/max against the macros anyway.
#include <chrono>

#include "Platform.h"

namespace MonotonicClock {

//----------------------------------------------------------------------
// The clock, its duration and its time point.
//
// TimePoint is pinned to millisecond resolution rather than
// steady_clock's native tick (nanoseconds on MSVC) for two reasons: it
// is the resolution every call site being migrated already had, and a
// millisecond rep keeps the 64-bit counter far away from overflow when
// a test walks it past the 32-bit wrap.
//----------------------------------------------------------------------
typedef std::chrono::steady_clock					Clock;
typedef std::chrono::milliseconds					Duration;
typedef std::chrono::time_point<Clock, Duration>	TimePoint;

//----------------------------------------------------------------------
// A replacement for steady_clock::now(), installed by a test.
//----------------------------------------------------------------------
typedef TimePoint (*SourceFn)();

//----------------------------------------------------------------------
// The current monotonic time, from the installed test source if there is
// one and from steady_clock otherwise.
//----------------------------------------------------------------------
TimePoint	Now();

//----------------------------------------------------------------------
// The legacy 32-bit millisecond tick: exactly what timeGetTime() /
// platform_get_ticks() returns, unless a test source is installed, in
// which case it is the low 32 bits of that source's millisecond count.
// See the header comment for why it is not derived from Now(), and for
// why it is not Win32 GetTickCount().
//----------------------------------------------------------------------
DWORD		LegacyTicks();

//----------------------------------------------------------------------
// Test seam. Passing NULL restores the real clock. Prefer
// ScopedTestSource below, which cannot leak an installed source out of a
// failing test.
//----------------------------------------------------------------------
void		SetTestSource(SourceFn pfn_source);
SourceFn	GetTestSource();

//----------------------------------------------------------------------
// Installs a test source for a scope and restores whatever was there
// before - including NULL, the real clock.
//----------------------------------------------------------------------
class ScopedTestSource
{
public:
	explicit ScopedTestSource(SourceFn pfn_source)
		: m_pfn_previous(GetTestSource())
	{
		SetTestSource(pfn_source);
	}

	~ScopedTestSource()
	{
		SetTestSource(m_pfn_previous);
	}

	ScopedTestSource(const ScopedTestSource&) = delete;
	ScopedTestSource& operator=(const ScopedTestSource&) = delete;

private:
	SourceFn	m_pfn_previous;
};

//----------------------------------------------------------------------
// Conversions for call sites that still speak in DWORD milliseconds -
// which is every public API this migration is required to leave alone.
//----------------------------------------------------------------------
inline Duration
Millis(DWORD dw_millisec)
{
	return Duration(static_cast<Duration::rep>(dw_millisec));
}

//----------------------------------------------------------------------
// A time point built from an absolute millisecond count. Only tests
// should need this; production code gets its time points from Now().
//----------------------------------------------------------------------
inline TimePoint
FromMillis(unsigned long long ull_millisec)
{
	return TimePoint(Duration(static_cast<Duration::rep>(ull_millisec)));
}

} // namespace MonotonicClock

#endif // __MONOTONIC_CLOCK_H__
