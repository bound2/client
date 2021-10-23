//----------------------------------------------------------------------
// MMusicTable.h
//----------------------------------------------------------------------
// Music Loading�ÿ� Music���� �ʿ��� ������ �����ϴ� class
//----------------------------------------------------------------------
//
// [ Music���� Loadind�� �ʿ��� ���� ] 
//
// - file�̸�
// 
//----------------------------------------------------------------------

#ifndef	__MMUSICTABLE_H__
#define	__MMUSICTABLE_H__

#include <fstream>
#include "MTypeDef.h"
#include "CTypeTable.h"
#include "MString.h"

//----------------------------------------------------------------------
//
// �� ������ Music�� ���� ����
//
//----------------------------------------------------------------------
class MUSICTABLE_INFO {
	public :
		MString					Filename;		// Music File�̸�(MID)
		MString					FilenameWav;	// wave Music File�̸�

	public :
		//-------------------------------------------------------
		// File I/O
		//-------------------------------------------------------
		void			SaveToFile(std::ofstream& file);		
		void			LoadFromFile(std::ifstream& file);		
};

typedef CTypeTable<MUSICTABLE_INFO>	MUSIC_TABLE;
extern 	MUSIC_TABLE*	g_pMusicTable;

#endif
