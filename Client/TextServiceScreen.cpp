//----------------------------------------------------------------------
// TextServiceScreen.cpp
//----------------------------------------------------------------------
//
// TextService::RenderText, the one member of that class the TextSystem
// library does not define (docs/RESTRUCTURING.md task 5.3).
//
// Everything else in TextService draws through a RenderTarget the
// caller supplies, which is what makes the library linkable on its own.
// This one draws through g_pLast - the back buffer the executable owns
// and creates - so while it lived in TextService.cpp a test binary had
// to define that global to link TextSystem at all. It did, in
// tests/stubs/client_globals.cpp, which existed for this and nothing
// else once 5.1 moved SendBugReport into packetwire.
//
// So the split is the same one MItem/MItemUse and MSkillManager/
// MSkillAvailable took: the library half stays, the half that reaches
// for the program around it compiles into the executable. TextService
// has no virtual members, so the class costs no vtable for being
// defined across two targets.
//
// The body is the one that was in TextService.cpp, unchanged.
//
//----------------------------------------------------------------------

#include "Client_PCH.h"

#include "TextSystem/TextService.h"
#include "TextSystem/RenderTargetSpriteSurface.h"
#include "SpriteLib/CSpriteSurface.h"

// The back buffer, defined in the executable (Client/Client.cpp).
extern CSpriteSurface* g_pLast;

namespace TextSystem {

void TextService::RenderText(int x, int y, const std::string& text)
{
	// Simple text rendering API for compatibility with SDL_RenderText
	// Renders white text at the specified position using the global surface

	auto& service = Get();

	// Get global surface reference (from SpriteLib)
	if (!::g_pLast || !::g_pLast->GetBackendSurface())
		return;

	// Create render target from global surface
	SpriteSurfaceRenderTarget target(::g_pLast);

	// Use default style with white color
	TextStyle style = service.GetDefaultStyle();
	// Override color to white
	style.color.r = 255;
	style.color.g = 255;
	style.color.b = 255;
	style.color.a = 255;

	// Draw the text
	service.DrawLine(target, text, x, y, 0, style);
}

} // namespace TextSystem
