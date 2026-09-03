//////////////////////////////////////////////////////////////////////
//
// Filename    : PacketDiagnostics.cpp
// Description : see PacketDiagnostics.h
//
//////////////////////////////////////////////////////////////////////

#include "Client_PCH.h"
#include "PacketDiagnostics.h"
#include "WireHost.h"

#include <stdarg.h>
#include <stdio.h>

namespace {
	PacketDiagnostics::BugReportFn s_pfnBugReport = NULL;
}

void PacketDiagnostics::setBugReportHook(BugReportFn fn)
{
	s_pfnBugReport = fn;
}

PacketDiagnostics::BugReportFn PacketDiagnostics::getBugReportHook()
{
	return s_pfnBugReport;
}

void PacketDiagnostics::reportBug(const char* format, ...)
{
	if (format == NULL)
		return;

	char buffer[256];
	va_list args;
	va_start(args, format);
	const int written = vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	// vsnprintf NUL-terminates within the buffer, so an over-long report
	// is truncated rather than overrun; a negative return is an encoding
	// error and nothing usable was produced.
	if (written < 0)
		return;

	//------------------------------------------------------------------
	// With a hook installed the text goes there and nowhere else, which
	// is what lets a test capture it. Without one it goes to
	// SendBugReport, which is in this library now (task 5.1's second
	// slice) - so the report reaches the server on its own rather than
	// through the executable installing a hook that called back in
	// here. SendBugReport drops it when the host names no connection.
	//------------------------------------------------------------------
	if (s_pfnBugReport != NULL)
	{
		s_pfnBugReport(buffer);
		return;
	}

	SendBugReport("%s", buffer);
}
