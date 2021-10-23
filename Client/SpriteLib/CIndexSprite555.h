//----------------------------------------------------------------------
// CIndexSprite555.h
//----------------------------------------------------------------------
// 5:5:5 video card에 대한 sprite class
//----------------------------------------------------------------------

#ifndef	__CINDEXSPRITE555_H__
#define	__CINDEXSPRITE555_H__

#include <Windows.h>
#include <fstream>
#include "CIndexSprite.h"


class CIndexSprite555 : public CIndexSprite {
	public :
		//---------------------------------------------------------
		// fstream에서 save/load를 한다.
		//---------------------------------------------------------
		bool		SaveToFile(std::ofstream& file);
		bool		LoadFromFile(std::ifstream& file);		

};


#endif



