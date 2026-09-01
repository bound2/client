//----------------------------------------------------------------------
// test_packet_diagnostics.cpp
//----------------------------------------------------------------------
//
// PacketDiagnostics (docs/RESTRUCTURING.md task 2.4): the wire library's
// bug-report seam. The executable installs its SendBugReport at the
// composition root; a test binary installs nothing and a report must
// then be a silent no-op - which is also exactly what the library's
// standalone link relies on.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "PacketDiagnostics.h"

#include <string>
#include <cstring>

namespace {

std::string	s_LastReport;
int		s_Reports = 0;

void	Capture(const char* message)
{
	s_LastReport = message;
	s_Reports++;
}

struct HookGuard
{
	PacketDiagnostics::BugReportFn m_Previous;
	HookGuard() : m_Previous(PacketDiagnostics::getBugReportHook()) { s_Reports = 0; s_LastReport.clear(); }
	~HookGuard() { PacketDiagnostics::setBugReportHook(m_Previous); }
};

} // namespace

TEST(PacketDiagnostics, ReportWithoutAHookIsANoOp)
{
	HookGuard guard;
	PacketDiagnostics::setBugReportHook(NULL);
	PacketDiagnostics::reportBug("too large PacketSize ID)%d %d/%d", 1, 2, 3);
	CHECK_EQ(0, s_Reports);
}

TEST(PacketDiagnostics, HookReceivesTheFormattedText)
{
	HookGuard guard;
	PacketDiagnostics::setBugReportHook(&Capture);
	PacketDiagnostics::reportBug("too large PacketSize ID)%d %d/%d", 284, 1000, 12);
	CHECK_EQ(1, s_Reports);
	CHECK(s_LastReport == "too large PacketSize ID)284 1000/12");
}

// Over-long reports are cut at the 255-character buffer, never overrun.
TEST(PacketDiagnostics, OverLongReportIsTruncatedNotOverrun)
{
	HookGuard guard;
	PacketDiagnostics::setBugReportHook(&Capture);
	const std::string longText(600, 'x');
	PacketDiagnostics::reportBug("%s", longText.c_str());
	CHECK_EQ(1, s_Reports);
	CHECK_EQ(255, s_LastReport.size());
}

TEST(PacketDiagnostics, NullFormatIsIgnored)
{
	HookGuard guard;
	PacketDiagnostics::setBugReportHook(&Capture);
	PacketDiagnostics::reportBug(NULL);
	CHECK_EQ(0, s_Reports);
}
