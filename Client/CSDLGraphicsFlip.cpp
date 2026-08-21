//----------------------------------------------------------------------
// CSDLGraphicsFlip.cpp
//----------------------------------------------------------------------
// CSDLGraphics::Flip()의 실제 구현.
//
// CSDLGraphics 자체는 Client/DXLib/CDirectDraw.cpp(dxlib 라이브러리)에
// 있지만, Flip()은 g_pBack(CSpriteSurface)의 전체 정의가 필요하다.
// dxlib 프로젝트는 SPRITELIB_BACKEND_SDL 없이, /IClient 없이 빌드되어
// SpriteLib/CSpriteSurface.h를 안전하게 끌어올 수 없으므로, 여기
// DarkEden.exe 쪽(Client_MAIN_SOURCES)에 따로 둔다.
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "DXLib/CDirectDraw.h"
#include "SpriteLib/CSpriteSurface.h"

extern CSpriteSurface* g_pBack;

//----------------------------------------------------------------------
// Flip
//----------------------------------------------------------------------
// 한 프레임을 화면에 보여준다. g_pBack(게임이 매 프레임 그려넣는 백버퍼)을
// SDL2 렌더러에 올린 뒤 present한다.
//----------------------------------------------------------------------
void CSDLGraphics::Flip()
{
	if (m_pSDLRenderer == NULL)
	{
		return;
	}

	SDL_RenderClear(m_pSDLRenderer);

	if (g_pBack != NULL)
	{
		spritectl_surface_t backendSurface = g_pBack->GetBackendSurface();
		if (backendSurface != SPRITECTL_INVALID_SURFACE)
		{
			spritectl_present_surface(backendSurface, m_pSDLRenderer);
		}
	}

	SDL_RenderPresent(m_pSDLRenderer);
}
