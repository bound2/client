//----------------------------------------------------------------------
// CIndexSprite565.h
//----------------------------------------------------------------------
// 5:6:5 video card�� ���� sprite class
//----------------------------------------------------------------------

#ifndef	__CINDEXSPRITE565_H__
#define	__CINDEXSPRITE565_H__

#include <Windows.h>
#include <fstream>
#include "CIndexSprite.h"


class CIndexSprite565 : public CIndexSprite {
	public :
		//---------------------------------------------------------
		// fstream���� save/load�� �Ѵ�.
		//---------------------------------------------------------
		bool		SaveToFile(std::ofstream& file);
		bool		LoadFromFile(std::ifstream& file);

};


#endif



