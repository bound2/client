//----------------------------------------------------------------------
// MObjectScreen.cpp - MObject's screen rectangles, executable side
//----------------------------------------------------------------------
// Split from MObject.cpp when the object base moved into gamemodel
// (docs/RESTRUCTURING.md task 4.4): these two members read the draw
// interpolation state the game update owns (g_bInterpolateDraw, the
// camera gaps), so they compile here, into the executable, while the
// constructor and the file round trip stay in the library.
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "MObject.h"

//----------------------------------------------------------------------
// Screen rects in exact tick space
//----------------------------------------------------------------------
// The rects are computed by the draw path, which since the 60 fps change
// renders positions interpolated between logic ticks against a camera
// carrying the player's interpolation bias (g_DrawCamGapX/Y). The per-tick
// input code hit-tests these rects (IsPointInScreenRect) against per-tick
// state, and a pick that misses because rect and logic disagree drops the
// held-button attack repeat. Shifting the rect by minus the object's own
// gap and minus the camera gap converts it back to exact tick space -
// identical to pre-interpolation behaviour. For the player both gaps cancel
// to zero, matching the pinned on-screen position. Outside the draw phase
// (g_bInterpolateDraw false) both gaps read as zero and this is a no-op.
//----------------------------------------------------------------------
extern bool		g_bInterpolateDraw;
extern int		g_DrawCamGapX;
extern int		g_DrawCamGapY;

//----------------------------------------------------------------------
// Add ScreenRect
//----------------------------------------------------------------------
// Grows the object's on-screen hit rectangle by a sprite's rectangle.
//----------------------------------------------------------------------
void
MObject::AddScreenRect(RECT* pRect)
{
	RECT rect = *pRect;

	if (g_bInterpolateDraw)
	{
		int shiftX = -GetDrawGapX() - g_DrawCamGapX;
		int shiftY = -GetDrawGapY() - g_DrawCamGapY;
		rect.left += shiftX;	rect.right += shiftX;
		rect.top += shiftY;		rect.bottom += shiftY;
	}

	if (rect.left < m_ScreenRect.left)		m_ScreenRect.left	= rect.left;
	if (rect.top < m_ScreenRect.top)		m_ScreenRect.top	= rect.top;
	if (rect.right > m_ScreenRect.right)	m_ScreenRect.right	= rect.right;
	if (rect.bottom > m_ScreenRect.bottom)	m_ScreenRect.bottom	= rect.bottom;
}

//----------------------------------------------------------------------
// Set ScreenRect
//----------------------------------------------------------------------
void
MObject::SetScreenRect(RECT* pRect)
{
	m_ScreenRect = *pRect;

	if (g_bInterpolateDraw)
	{
		int shiftX = -GetDrawGapX() - g_DrawCamGapX;
		int shiftY = -GetDrawGapY() - g_DrawCamGapY;
		m_ScreenRect.left += shiftX;	m_ScreenRect.right += shiftX;
		m_ScreenRect.top += shiftY;		m_ScreenRect.bottom += shiftY;
	}
}

