//----------------------------------------------------------------------
// CAlphaSprite555.h
//----------------------------------------------------------------------
// 5:5:5 video card�� ���� sprite class
//----------------------------------------------------------------------

#ifndef	__CALPHASPRITE555_H__
#define	__CALPHASPRITE555_H__

using namespace std;

#include <Windows.h>
#include <fstream>
#include "CAlphaSprite.h"


class CAlphaSprite555 : public CAlphaSprite {
	public :
	
		//---------------------------------------------------------
		// fstream���� save/load�� �Ѵ�.
		//---------------------------------------------------------
		bool		SaveToFile(std::ofstream& file);
		bool		LoadFromFile(std::ifstream& file);
};


#endif



