//////////////////////////////////////////////////////////////////////
//
// Filename    : PacketDiagnostics.cpp
// Description : see PacketDiagnostics.h
//
//////////////////////////////////////////////////////////////////////

#include "Client_PCH.h"
#include "PacketDiagnostics.h"

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
	if (s_pfnBugReport == NULL || format == NULL)
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

	s_pfnBugReport(buffer);
}
