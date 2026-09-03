//----------------------------------------------------------------------
// client_globals.cpp
//----------------------------------------------------------------------
//
// Definitions for globals that a static library references but that live in
// the DarkEden executable.
//
// TextService.cpp holds both NormalizeText, a pure string function, and the
// on-screen drawing entry point, which reaches for the global back buffer
// g_pLast. Linking the object for the sake of the former drags in the
// reference from the latter, so a test binary has nothing to resolve against
// and fails with LNK2001.
//
// A null is the correct value here rather than merely a convenient one:
// nothing under test calls the drawing path, and if something ever does it
// takes that function's own `if (!g_pLast)` early-out instead of writing into
// a surface that was never created.
//
// The real fix is to separate the pure text utilities in TextService.cpp from
// the code that draws through client globals, which would let the whole
// TextSystem library be tested without a stub. That is a larger change than
// the one this file supports.
//
//----------------------------------------------------------------------

// The same shape a second time, for a function rather than a global.
// Player::processCommand reports an oversized packet upstream through
// SendBugReport, which builds a CGSay - a packetwire class - and sends
// it through g_pSocket, the executable's ClientPlayer. A test binary
// has no server to report to, so the report goes nowhere; nothing
// under test takes that path, which needs a socket carrying a packet
// whose declared size exceeds its own factory's maximum.
//
// The real fix is the one the plan records for task 5.1's next slice:
// SendBugReport itself reaches for exactly one thing outside the wire
// layer, so it can move into packetwire behind a Player* rather than
// stay a symbol the library expects the executable to supply. Doing it
// here would mean moving a file this slice does not touch.
//
// Note what this stub costs: the failed link was the only thing that
// found this seam - W1/W2 in check_includes.pl read includes and R4
// greps for g_p* globals, so neither can see a library file calling an
// executable-side function - and defining the symbol here means the
// next caller links quietly instead. R6 in tests/ratchet/ratchets.sh
// replaces the signal: it counts packetwire members calling
// SendBugReport, and its baseline is the one there is.
//
//----------------------------------------------------------------------

#include <cstddef>

class CSpriteSurface;

CSpriteSurface*		g_pLast = NULL;

void	SendBugReport(const char* bug, ...);
void	SendBugReport(const char* /*bug*/, ...) {}
