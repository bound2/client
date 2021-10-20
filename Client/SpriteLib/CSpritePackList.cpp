//----------------------------------------------------------------------
// CSpritePackList.cpp
//----------------------------------------------------------------------

#include "CSprite.h"
#include "CSpritePackList.h"
#include <fstream>

//----------------------------------------------------------------------
//
// constructor/destructor
//
//----------------------------------------------------------------------

CSpritePackList::CSpritePackList()
{
}

CSpritePackList::~CSpritePackList()
{
	// list�� �޸𸮿��� �����Ѵ�.
	Release();
}

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Release
//----------------------------------------------------------------------
void
CSpritePackList::Release()
{
	if (!m_listSprite.empty())
	{
		// ��� CSprite�� �����.
		SPRITE_LIST::iterator iSprite = m_listSprite.begin();

		// ������ CSprite�� �޸𸮸� delete�Ѵ�.
		while (iSprite != m_listSprite.end())
		{
			CSprite* pSprite = *iSprite;
			//pSprite->Release();
			delete pSprite;
			iSprite++;
		}

		m_listSprite.clear();
	}
}

//----------------------------------------------------------------------
// Add Sprite to List
//----------------------------------------------------------------------
// List�� ���� CSprite*�� �߰��Ѵ�.
// pSprite�� �ܺο��� new�� �ؾ��ϰ� delete�� CSpritePackList ���ο��� �Ѵ�.
//----------------------------------------------------------------------
void		
CSpritePackList::AddSprite(CSprite* pSprite)
{
	// list�� �߰��ϰ�
	m_listSprite.insert(m_listSprite.end(), pSprite);
}

//----------------------------------------------------------------------
// Remove Sprite
//----------------------------------------------------------------------
// List�� Sprite�߿��� n��°�� Sprite�� �����Ѵ�.
// index�� 0���� nSprite-1�����̰� delete����� �Ѵ�.
//----------------------------------------------------------------------
void		
CSpritePackList::RemoveSprite(TYPE_SPRITEID n)
{
	// ���� �Ұ�
	if ( n >= m_listSprite.size() )
		return;

	SPRITE_LIST::iterator	iSprite = m_listSprite.begin();

	// n��° sprite�� �����Ѵ�.
	for (TYPE_SPRITEID i=0; i<n; i++)
		iSprite++;

	// �޸𸮿��� �����
	delete (*iSprite);

	// list���� �����.
	m_listSprite.erase(iSprite);
}

//----------------------------------------------------------------------
// Add Sprite
//----------------------------------------------------------------------
// n��° �ڿ� pSprite�� �߰��Ѵ�.
//
// ex)  0,1,2,3,4....,9  = 10��
//----------------------------------------------------------------------
void		
CSpritePackList::AddSprite(TYPE_SPRITEID n, CSprite* pSprite)
{	
	// n�� �ʹ� Ŭ��
	if ( n > m_listSprite.size() )
		n = m_listSprite.size();

	SPRITE_LIST::iterator	iSprite = m_listSprite.begin();

	// n��° sprite�� �����Ѵ�.
	for (TYPE_SPRITEID i=0; i<n; i++)
		iSprite++;

	// �߰��Ѵ�.
	m_listSprite.insert(iSprite, pSprite);
}

//----------------------------------------------------------------------
// Get Sprite
//----------------------------------------------------------------------
// List�� Sprite�߿��� n��°�� Sprite�� �����Ѵ�.
// index�� 0���� nSprite-1�����̰� delete����� �Ѵ�.
//----------------------------------------------------------------------
CSprite*
CSpritePackList::GetSprite(TYPE_SPRITEID n) const
{
	// get �Ұ�
	if ( n >= m_listSprite.size() )
		return NULL;

	SPRITE_LIST::const_iterator	iSprite = m_listSprite.begin();

	// n��° sprite�� �����Ѵ�.
	for (TYPE_SPRITEID i=0; i<n; i++)
		iSprite++;

	// n��° Sprite�� �Ѱ��ش�.
	return (*iSprite);
}

//----------------------------------------------------------------------
// Change Sprite
//----------------------------------------------------------------------
// n��°�� m��° Sprite�� �ٲ۴�.
//----------------------------------------------------------------------
void		
CSpritePackList::ChangeSprite(TYPE_SPRITEID n, TYPE_SPRITEID m)
{
	// ���� ��쿣 �ǹ̰� ������..
	if (m==n) return;

	// get �Ұ�
	if ( n >= m_listSprite.size() || m >= m_listSprite.size())
		return;

	TYPE_SPRITEID temp;

	//------------------------------------------------------
	// n�� ���� ���� �����Ѵ�.
	//------------------------------------------------------
	if (n > m) 
	{
		temp = n;
		n = m;
		m = temp;
	}

	SPRITE_LIST::iterator	iFirstSprite = m_listSprite.begin();	

	//------------------------------------------------------
	// n��° sprite�� �����Ѵ�.
	//------------------------------------------------------
	for (TYPE_SPRITEID i=0; i<n; i++)
		iFirstSprite++;

	//------------------------------------------------------
	// m��° sprite�� �����Ѵ�.
	//------------------------------------------------------
	temp = m-n;
	SPRITE_LIST::iterator	iSecondSprite = iFirstSprite;
	for (short i=n; i<temp; i++)
		iSecondSprite++;
	
	//------------------------------------------------------
	// iFirstSprite�� iSecondSprite�� �ٲ��ش�.
	//------------------------------------------------------
	CSprite*	pSprite = *iFirstSprite;
	*iFirstSprite = *iSecondSprite;
	*iSecondSprite = pSprite;
}

//----------------------------------------------------------------------
// Save To File
//----------------------------------------------------------------------
// list�� ���󰡸鼭 file�� �����ؾ��Ѵ�.
//----------------------------------------------------------------------
bool		
CSpritePackList::SaveToFile(std::ofstream& spkFile, std::ofstream& indexFile)
{
	// �ʱ�ȭ���� �ʾ����� 
	if (m_listSprite.empty())
		return FALSE;	

	// Size ����
	TYPE_SPRITEID size = m_listSprite.size();
	spkFile.write((const char *)&size, SIZE_SPRITEID); 
	indexFile.write((const char *)&size, SIZE_SPRITEID);

	//--------------------------------------------------
	// index file�� �����ϱ� ���� ����
	//--------------------------------------------------
	long*	pIndex = new long [size];

	// List�� ��� Sprite�� �����Ѵ�.
	SPRITE_LIST::iterator iSprite = m_listSprite.begin();

	//--------------------------------------------------
	// ��� CSprite�� ����
	//--------------------------------------------------
	int i = 0;
	while (iSprite != m_listSprite.end())
	{
		// SpritePack file�� �������� index�� ����
		pIndex[i++] = spkFile.tellp();

		// Sprite ����
		(*iSprite)->SaveToFile(spkFile);		// CSprite����	
		iSprite ++;
	}
	
	//--------------------------------------------------
	// index ����
	//--------------------------------------------------
	for (i=0; i<size; i++)
	{
		indexFile.write((const char*)&pIndex[i], 4);
	}

	// �޸𸮿��� ����
	delete [] pIndex;

	return true;
}
