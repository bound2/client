//----------------------------------------------------------------------
// CSpritePackList.h
//----------------------------------------------------------------------
// ���������� stl�� list�� ����ߴ�.
//----------------------------------------------------------------------

#ifndef	__CSPRITEPACKLIST_H__
#define	__CSPRITEPACKLIST_H__

#pragma warning(disable:4786)

#include <list>
#include <fstream>
#include "DrawTypeDef.h"
#include "CSprite.h"

typedef	std::list<CSprite*>		SPRITE_LIST;
typedef std::list<CSpritePal*>	SPRITEPAL_LIST;


class CSpritePackList {
	public :
		CSpritePackList();
		virtual ~CSpritePackList();

		//---------------------------------------------------
		// Init/Release		
		//---------------------------------------------------
		void		Release();

		//---------------------------------------------------
		// add / remove
		//---------------------------------------------------
		void		AddSprite(TYPE_SPRITEID n, CSprite* pSprite);
		void		AddSprite(CSprite* pSprite);
		void		RemoveSprite(TYPE_SPRITEID n);
		void		ChangeSprite(TYPE_SPRITEID n, TYPE_SPRITEID m);

		//---------------------------------------------------
		// get
		//---------------------------------------------------
		CSprite*	GetSprite(TYPE_SPRITEID n) const;

		//---------------------------------------------------
		// ù��° ��ġ�� List Iterater�� �Ѱ��ش�.
		//---------------------------------------------------
		SPRITE_LIST::const_iterator	GetIterator() const	{ return m_listSprite.begin(); }
		SPRITE_LIST* GetListPtr() { return &m_listSprite; }

		//---------------------------------------------------
		// get size
		//---------------------------------------------------
		TYPE_SPRITEID	GetSize() const	{ return m_listSprite.size(); }

		//---------------------------------------------------
		// file I/O
		//---------------------------------------------------
		bool				SaveToFile(std::ofstream& spkFile, std::ofstream& indexFile);
		virtual void		LoadFromFile(std::ifstream& file) = 0;

	protected :		
		SPRITE_LIST		m_listSprite;		// CSprite���� �����صд�.
};

class CSpritePalPackList {
	public :
		CSpritePalPackList();
		~CSpritePalPackList();

		//---------------------------------------------------
		// Init/Release		
		//---------------------------------------------------
		void		Release();

		//---------------------------------------------------
		// add / remove
		//---------------------------------------------------
		void		AddSprite(TYPE_SPRITEID n, CSpritePal* pSprite);
		void		AddSprite(CSpritePal* pSprite);
		void		RemoveSprite(TYPE_SPRITEID n);
		void		ChangeSprite(TYPE_SPRITEID n, TYPE_SPRITEID m);

		//---------------------------------------------------
		// get
		//---------------------------------------------------
		CSpritePal*	GetSprite(TYPE_SPRITEID n) const;

		//---------------------------------------------------
		// ù��° ��ġ�� List Iterater�� �Ѱ��ش�.
		//---------------------------------------------------
		SPRITEPAL_LIST::const_iterator	GetIterator() const	{ return m_listSprite.begin(); }
		SPRITEPAL_LIST* GetListPtr() { return &m_listSprite; }

		//---------------------------------------------------
		// get size
		//---------------------------------------------------
		TYPE_SPRITEID	GetSize() const	{ return m_listSprite.size(); }

		//---------------------------------------------------
		// file I/O
		//---------------------------------------------------
		bool		SaveToFile(std::ofstream& spkFile, std::ofstream& indexFile);
		void		LoadFromFile(std::ifstream& file);

	protected :		
		SPRITEPAL_LIST		m_listSprite;		// CSprite���� �����صд�.
};
#endif



