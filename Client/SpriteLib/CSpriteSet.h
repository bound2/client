//----------------------------------------------------------------------
// CSpriteSet.h
//----------------------------------------------------------------------
//
// SpritePack���� Ư�� Sprite�� Load�� �� �ְ� �Ѵ�.
//
// SpriteSet file(index��)���� ������ �о 
// SpritePack���� Ư���� Sprite�� �о���δ�.
//
// SpriteSet IndexFile�� ������ �̿��ؼ� SpritePack���� 
// Ư�� ��ġ(File Position)�� Sprite�� Load�Ѵ�.
//
//----------------------------------------------------------------------

#ifndef	__CSPRITESET_H__
#define	__CSPRITESET_H__


#include "DrawTypeDef.h"
#include "CSpritePack.h"


class CSpriteSet {
	public :
		CSpriteSet();
		~CSpriteSet();

		//--------------------------------------------------------
		// Init/Release
		//--------------------------------------------------------
		void		Init(TYPE_SPRITEID count);		
		void		Release();

		//--------------------------------------------------------
		// file I/O		
		//--------------------------------------------------------
		// SpritePack File���� Sprite�� Load�Ѵ�.
		// spkFile = SpritePack File, indexFile = FilePointer File
		bool		LoadFromFile(std::ifstream& indexFile, std::ifstream& packFile);
		

		//--------------------------------------------------------
		// operator
		//--------------------------------------------------------
		CSprite&	operator [] (TYPE_SPRITEID n) { return m_pSprites[n]; }


	protected :
		TYPE_SPRITEID		m_nSprites;			// Sprite ID�� ����
		CSprite*			m_pSprites;			// MSpritePack�� CSprite���� Load�Ѵ�.
};

#endif

