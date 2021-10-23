//----------------------------------------------------------------------
// CSpriteSet.cpp
//----------------------------------------------------------------------
#include "client_PCH.h"
#include "CSpriteSurface.h"
#include "CSprite565.h"
#include "CSprite555.h"
#include "CSpritePack.h"
#include "CSpriteSet.h"

//----------------------------------------------------------------------
//
// constructor/destructor
//
//----------------------------------------------------------------------

CSpriteSet::CSpriteSet()
{
	m_nSprites		= 0;
	m_pSprites		= NULL;
}

CSpriteSet::~CSpriteSet()
{
	// array�� �޸𸮿��� �����Ѵ�.
	Release();
}

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Init
//----------------------------------------------------------------------
void
CSpriteSet::Init(TYPE_SPRITEID count)
{
	// ������ ���� ��� 
	if (count==0) 
		return;

	// �ϴ� ����
	Release();

	// �޸� ���
	m_nSprites = count;

	if (CDirectDraw::Is565())
	{
		m_pSprites = new CSprite565 [m_nSprites];
	}
	else
	{
		m_pSprites = new CSprite555 [m_nSprites];
	}
}

//----------------------------------------------------------------------
// Release
//----------------------------------------------------------------------
void
CSpriteSet::Release()
{
	if (m_pSprites != NULL)
	{
		// ��� MSprite�� �����.
		delete [] m_pSprites;
		m_pSprites = NULL;

		m_nSprites = 0;
	}
}

//----------------------------------------------------------------------
// Load From File
//----------------------------------------------------------------------
// SpriteSet IndexFile�� �̿��ؼ� SpritePack File���� 
// Ư�� ��ġ�� Sprite���� Load�Ѵ�.
//----------------------------------------------------------------------
bool		
CSpriteSet::LoadFromFile(std::ifstream& indexFile, std::ifstream& packFile)
{
	TYPE_SPRITEID	count;
	
	//------------------------------------------------------
	// SpriteSet�� Sprite������ �о���δ�.
	//------------------------------------------------------
	indexFile.read((char*)&count, SIZE_SPRITEID);


	long* pIndex = new long [count];	// file position

	//------------------------------------------------------
	// SpriteSet IndexFile�� ��� �о���δ�.
	//------------------------------------------------------
	for (TYPE_SPRITEID i=0; i<count; i++)
	{		
		indexFile.read((char*)&pIndex[i], 4);
	}

	// Loop�� ���� ����ϴ� ������ 
	// �ƹ����� �ΰ��� file�� ���ÿ� access�ϸ�
	// ������ �� ���Ƽ�... �����ϱ�? - -;;


	//------------------------------------------------------
	// Sprite�� Load�� memory�� ��´�.
	//------------------------------------------------------
	Init( count );

	//------------------------------------------------------
	// Index(File Position)�� �̿��ؼ� SpritePack����
	// Ư�� Sprite���� Load�Ѵ�.
	//------------------------------------------------------
	register int i;
	for (i=0; i<count; i++)
	{
		packFile.seekg(pIndex[i], std::ios::beg);
		m_pSprites[i].LoadFromFile( packFile );
	}

	delete [] pIndex;

	return true;
}

