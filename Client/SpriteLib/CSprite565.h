//----------------------------------------------------------------------
// CSprite565.h
//----------------------------------------------------------------------
// 5:6:5 video card�� ���� sprite class
//----------------------------------------------------------------------

#ifndef	__CSPRITE565_H__
#define	__CSPRITE565_H__

#include <Windows.h>

#include "CSprite.h"


class CSprite565 : public CSprite {
	public :
		//---------------------------------------------------------
		// fstream���� save/load�� �Ѵ�.
		//---------------------------------------------------------
		bool		SaveToFile(ofstream& file);
		bool		LoadFromFile(ifstream& file);
		//bool	LoadFromFileToBuffer(ifstream& file);
};


#endif


