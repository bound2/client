//----------------------------------------------------------------------
// CSpriteSetManager.cpp
//----------------------------------------------------------------------		
#include "client_PCH.h"
#include "CSpriteSetManager.h"
//----------------------------------------------------------------------		
//
// member functions
//
//----------------------------------------------------------------------		
CSpriteSetManager::CSpriteSetManager()
{
}

CSpriteSetManager::~CSpriteSetManager()
{
}

//----------------------------------------------------------------------		
//
// member functions
//
//----------------------------------------------------------------------		

//----------------------------------------------------------------------
// Save FilePosition
//----------------------------------------------------------------------				
// SpritePack IndexFile�κ��� SpriteSet IndexFile�� �����Ѵ�.
//
// spkIndex�� SpritePack IndexFile�̰�,
// SpritePack�� �ִ� ������ Sprite�� ���� File Position�� �����ϰ� �ִ�.
//
// setIndex�� SpriteSet IndexFile�̰�, m_List���� SpriteID�� �о
// �� �Լ����� �����ϴµ� �̿��� ���̴�.
//----------------------------------------------------------------------
bool		
CSpriteSetManager::SaveSpriteSetIndex(std::ofstream& setIndex, std::ifstream& spkIndex)
{
	// m_List�� �ƹ��͵� ������..
	if (m_List.size() == 0)
		return false;


	TYPE_SPRITEID	count;

	//---------------------------------------------------------------
	// SpritePack IndexFile�� Sprite������ �д´�.
	//---------------------------------------------------------------
	spkIndex.read((char*)&count, SIZE_SPRITEID);

	// SpritePack Index�� �����ص� memory���
	long* pIndex = new long [count];

	//---------------------------------------------------------------
	// ��� SpritePack IndexFile�� Load�Ѵ�.
	//---------------------------------------------------------------
	for (TYPE_SPRITEID i=0; i<count; i++)
	{
		spkIndex.read((char*)&pIndex[i], 4);
	}

	//---------------------------------------------------------------
	// m_List�� ������� SpritePack Index���� 
	// �ش��ϴ� SpriteID�� File Position�� �о �����Ѵ�.
	//---------------------------------------------------------------
	DATA_LIST::iterator iData = m_List.begin();

	// SpriteSet�� Sprite���� ����
	count = m_List.size();
	setIndex.write((const char*)&count, SIZE_SPRITEID);

	// List�� ��� node�� ���ؼ�..
	while (iData != m_List.end())
	{
		// Sprite ID�� ���� SpritePack File������ File Position
		setIndex.write((const char*)&pIndex[(*iData)], 4);

		iData ++;
	}

	delete [] pIndex;

	return true;
}

