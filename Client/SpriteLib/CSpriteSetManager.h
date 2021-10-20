//----------------------------------------------------------------------
// CSpriteSetManager.h
//----------------------------------------------------------------------
//
// SpritePack���� Ư�� Sprite�� Load�� �� �ֵ��� ������ �����.
//
// <SpriteID�� ������ List���>
//
// SpritePack File�� �Ϻ� Sprite ID�� �����Ѵ�. 
// 
// SpriteSet���� load�Ҷ� ����� index file�� ������ �ϴµ�,
// ����� Sprite ID�� �ش��ϴ� Sprite�� File Position�� 
// SpritePack IndexFile���� ã�Ƽ� 
// SpriteSet index file�� �����ؾ� �Ѵ�.
//
//----------------------------------------------------------------------
//
// Sprite ID�� set�ϰ�
// File Position�� �����ؾ� �Ѵ�.
//
//----------------------------------------------------------------------

#ifndef	__CSPRITESETMANAGER_H__
#define	__CSPRITESETMANAGER_H__

#include "DrawTypeDef.h"
#include "CSetManager.h"
#include <fstream>


class CSpriteSetManager : public CSetManager<TYPE_SPRITEID, TYPE_SPRITEID> {
	public :
		CSpriteSetManager();
		~CSpriteSetManager();

		//--------------------------------------------------------
		// SpritePack IndexFile�κ��� SpriteSet IndexFile�� �����Ѵ�.
		//--------------------------------------------------------
		bool		SaveSpriteSetIndex(std::ofstream& setIndex, std::ifstream& spkIndex);
		

	protected :
		
};


#endif
