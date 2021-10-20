//----------------------------------------------------------------------
// CFrameSetManager.cpp
//----------------------------------------------------------------------
#include "Frame_PCH.h"
#include "CFrameSetManager.h"
#include "fstream"

//----------------------------------------------------------------------
//
// constructor/destructor
//
//----------------------------------------------------------------------

CFrameSetManager::CFrameSetManager()
{
}

CFrameSetManager::~CFrameSetManager()
{
}

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Save FrameSet Index
//----------------------------------------------------------------------
// FramePack IndexFile�κ��� FrameSet IndexFile�� �����Ѵ�.
//----------------------------------------------------------------------
bool		
CFrameSetManager::SaveFrameSetIndex(std::ofstream& setIndex, std::ifstream& packIndex)
{
	// m_List�� �ƹ��͵� ������..
	if (m_List.size() == 0)
		return false;


	TYPE_FRAMEID	count;

	//---------------------------------------------------------------
	// FramePack IndexFile�� Frame������ �д´�.
	//---------------------------------------------------------------
	packIndex.read((char*)&count, SIZE_FRAMEID);

	// FramePack Index�� �����ص� memory���
	long* pIndex = new long [count];

	//---------------------------------------------------------------
	// ��� FramePack IndexFile�� Load�Ѵ�.
	//---------------------------------------------------------------
	for (TYPE_FRAMEID i=0; i<count; i++)
	{
		packIndex.read((char*)&pIndex[i], 4);
	}

	//---------------------------------------------------------------
	// m_List�� ������� FramePack Index���� 
	// �ش��ϴ� FrameID�� File Position�� �о �����Ѵ�.
	//---------------------------------------------------------------
	DATA_LIST::iterator iData = m_List.begin();

	// FrameSet�� Frame���� ����
	count = m_List.size();
	setIndex.write((const char*)&count, SIZE_FRAMEID);

	// List�� ��� node�� ���ؼ�..
	while (iData != m_List.end())
	{
		// Frame ID�� ���� FramePack File������ File Position
		setIndex.write((const char*)&pIndex[(*iData)], 4);

		iData ++;
	}

	delete [] pIndex;

	return true;
}

