//----------------------------------------------------------------------
// test_main.cpp
//----------------------------------------------------------------------
//
// Entry point for the unit test binary. Returns non-zero when any test
// fails so CTest reports the run as failed.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#if defined(_MSC_VER)
#include <crtdbg.h>
#include <stdlib.h>
#endif

namespace {

//----------------------------------------------------------------------
// Keep failures non-interactive.
//
// Tests that cover memory handling can trip the debug heap or an assert.
// By default the MSVC runtime answers those with a modal dialog, which
// would hang an unattended run instead of failing it. Route the reports
// to stderr so the process aborts with a diagnostic and a non-zero exit
// code.
//----------------------------------------------------------------------
void	MakeFailuresNonInteractive()
{
#if defined(_MSC_VER)
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
}

} // namespace

int	main()
{
	MakeFailuresNonInteractive();

	return ::testfw::RunAll();
}
