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

#include <cstddef>

class CSpriteSurface;

CSpriteSurface*		g_pLast = NULL;
