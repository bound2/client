//////////////////////////////////////////////////////////////////////
//
// Filename    : PacketDiagnostics.h
// Description : The wire layer's one outbound diagnostic seam.
//
//               Datagram::read used to call SendBugReport() directly - a
//               function defined in the executable that formats the text
//               and sends it to the game server as a "*bug_report" chat
//               line. A library may not reach into the executable (rule
//               W1, and the packet factory test links packetwire on its
//               own), so the call goes through a hook instead: the
//               executable installs SendBugReport at its composition root
//               (PacketHandlerRegistry.cpp) and the library reports
//               through it. With no hook installed - a test binary - a
//               report is a no-op. Same text, same recipient, no link.
//
//////////////////////////////////////////////////////////////////////

#ifndef __PACKET_DIAGNOSTICS_H__
#define __PACKET_DIAGNOSTICS_H__

namespace PacketDiagnostics {

	// Receives one fully formatted report.
	typedef void (*BugReportFn)(const char* message);

	// Install (or, with NULL, remove) the executable's reporter. Written
	// only at startup, like the dispatcher table.
	void setBugReportHook(BugReportFn fn);
	BugReportFn getBugReportHook();

	// printf-style; formats into a bounded buffer (the same 256 bytes
	// SendBugReport uses) and hands the text to the hook, if any.
	void reportBug(const char* format, ...);
}

#endif // __PACKET_DIAGNOSTICS_H__
