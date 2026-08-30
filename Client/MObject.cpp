//----------------------------------------------------------------------
// MObject.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "MObject.h"

//----------------------------------------------------------------------
//
// constructor/destructor
//
//----------------------------------------------------------------------
MObject::MObject()	
{ 
	m_ObjectType	= TYPE_OBJECT; 
	m_ID			= OBJECTID_NULL;
	m_X	 			= SECTORPOSITION_NULL;
	m_Y				= SECTORPOSITION_NULL;
	ClearScreenRect();
}

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

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
// 화면에서의 Sprite충돌 사각 영역을 추가한다.
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

//----------------------------------------------------------------------
// Save to File
//----------------------------------------------------------------------
void	
MObject::SaveToFile(ofstream& file)
{
	file.write((const char*)&m_ObjectType, 1);
	file.write((const char*)&m_ID, SIZE_OBJECTID);
	file.write((const char*)&m_X, SIZE_SECTORPOSITION);
	file.write((const char*)&m_Y, SIZE_SECTORPOSITION);
}

//----------------------------------------------------------------------
// Load from File
//----------------------------------------------------------------------
void	
MObject::LoadFromFile(ifstream& file)
{
	file.read((char*)&m_ObjectType, 1);
	file.read((char*)&m_ID, SIZE_OBJECTID);	
	file.read((char*)&m_X, SIZE_SECTORPOSITION);
	file.read((char*)&m_Y, SIZE_SECTORPOSITION);
}

