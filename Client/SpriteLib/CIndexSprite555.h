//----------------------------------------------------------------------
// CIndexSprite555.h
//----------------------------------------------------------------------
// 5:5:5 video card�� ���� sprite class
//----------------------------------------------------------------------

#ifndef	__CINDEXSPRITE555_H__
#define	__CINDEXSPRITE555_H__

#include <Windows.h>
#include <fstream>
#include "CIndexSprite.h"


class CIndexSprite555 : public CIndexSprite {
	public :
		//---------------------------------------------------------
		// fstream���� save/load�� �Ѵ�.
		//---------------------------------------------------------
		bool		SaveToFile(std::ofstream& file);
		bool		LoadFromFile(std::ifstream& file);		

};


#endif



