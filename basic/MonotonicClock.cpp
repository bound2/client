//----------------------------------------------------------------------
// MonotonicClock.cpp
//----------------------------------------------------------------------
//
// See MonotonicClock.h for what this is for and why LegacyTicks() is
// platform_get_ticks() rather than a truncation of Now().
//
//----------------------------------------------------------------------

#include "MonotonicClock.h"

#include <atomic>

namespace MonotonicClock {

namespace {

//----------------------------------------------------------------------
// The installed test source, or NULL for the real clock.
//
// Constant-initialised, so there is no static initialisation order
// question: gC_timer2 and every other object that calls Now() from a
// constructor sees NULL here whatever order the translation units are
// initialised in.
//----------------------------------------------------------------------
std::atomic<SourceFn>	g_pfn_test_source{ nullptr };

} // anonymous namespace

//----------------------------------------------------------------------
// Now
//----------------------------------------------------------------------
TimePoint
Now()
{
	const SourceFn pfn_source = g_pfn_test_source.load(std::memory_order_relaxed);

	if (pfn_source != nullptr)
	{
		return pfn_source();
	}

	return std::chrono::time_point_cast<Duration>(Clock::now());
}

//----------------------------------------------------------------------
// LegacyTicks
//
// The real clock path is deliberately the same call GetTickCount()
// expands to, not a value derived from Now(): while a class is only half
// converted, a comparison between a converted site and an unconverted
// one has to keep meaning what it meant.
//
// The test path takes the low 32 bits of the injected source's
// millisecond count, which is what makes the legacy tick's own wrap
// reachable from a test.
//----------------------------------------------------------------------
DWORD
LegacyTicks()
{
	const SourceFn pfn_source = g_pfn_test_source.load(std::memory_order_relaxed);

	if (pfn_source != nullptr)
	{
		const unsigned long long ull_millisec =
			static_cast<unsigned long long>(pfn_source().time_since_epoch().count());

		return static_cast<DWORD>(ull_millisec & 0xFFFFFFFFull);
	}

	return platform_get_ticks();
}

//----------------------------------------------------------------------
// SetTestSource
//----------------------------------------------------------------------
void
SetTestSource(SourceFn pfn_source)
{
	g_pfn_test_source.store(pfn_source, std::memory_order_relaxed);
}

//----------------------------------------------------------------------
// GetTestSource
//----------------------------------------------------------------------
SourceFn
GetTestSource()
{
	return g_pfn_test_source.load(std::memory_order_relaxed);
}

} // namespace MonotonicClock
