//----------------------------------------------------------------------
// test_main.cpp
//----------------------------------------------------------------------
//
// Entry point for the unit test binary. Returns non-zero when any test
// fails so CTest reports the run as failed.
//
//----------------------------------------------------------------------

#include "test_framework.h"

int	main()
{
	return ::testfw::RunAll();
}
