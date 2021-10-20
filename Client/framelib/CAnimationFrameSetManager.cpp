//----------------------------------------------------------------------
// CAnimationFrameSetManager.cpp
//----------------------------------------------------------------------
#include "Frame_PCH.h"
#include "CFrame.h"
#include "CAnimationFrameSetManager.h"
#include "CFramePack.h"
#include "CSpriteSetManager.h"
#include "fstream"

//----------------------------------------------------------------------
//
// constructor/destructor
//
//----------------------------------------------------------------------

CAnimationFrameSetManager::CAnimationFrameSetManager()
{
}

CAnimationFrameSetManager::~CAnimationFrameSetManager()
{
}

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Save SpriteSet Index
//----------------------------------------------------------------------
// AnimationFrameSet�� ���õ� SpriteID�� �����ؼ�
// SpritePack IndexFile�κ��� SpriteSet IndexFile�� �����Ѵ�.
//----------------------------------------------------------------------
bool		
CAnimationFrameSetManager::SaveSpriteSetIndex(CAnimationFramePack* pAnimationFramePack, 
										  std::ofstream& setIndex, std::ifstream& packIndex)
{
	CSpriteSetManager ssm;

	
	FRAME_ARRAY*	pFrameArray;

	DATA_LIST::iterator iData = m_List.begin();

	//------------------------------------------------------------------
	// ���õ� FrameID�� ���õ� ��� SpriteID�� �˾Ƴ��� �Ѵ�.	
	//------------------------------------------------------------------
	while (iData != m_List.end())
	{
		// FRAME_ARRAY�� �о�´�.
		pFrameArray = &((*pAnimationFramePack)[*iData]);

		//--------------------------------------------------------------
		// ������ FRAME_ARRAY�� ���� Frame�� SpriteID�� 
		// CSpriteSetManager�� �����Ѵ�.
		//--------------------------------------------------------------
		for (int i=0; i<pFrameArray->GetSize(); i++)
		{
			ssm.Add( (*pFrameArray)[i].GetSpriteID() );
		}

		iData++;
	}

	//------------------------------------------------------------------
	// AnimationFrameSet�� ���õ� ��� SpriteID�� 
	// SpriteSetManager�� ���������Ƿ� 
	// SpriteSetManager�� �̿��� SpriteSetIndex�� �����ϸ�ȴ�.
	//------------------------------------------------------------------
	return ssm.SaveSpriteSetIndex(setIndex, packIndex);
}

