//----------------------------------------------------------------------
// CCreatureFrameSetManager.cpp
//----------------------------------------------------------------------
#include "Frame_PCH.h"
#include "CFrame.h"
#include "CCreatureFrameSetManager.h"
#include "CFramePack.h"
#include "CSpriteSetManager.h"
#include "fstream"

//----------------------------------------------------------------------
//
// constructor/destructor
//
//----------------------------------------------------------------------

CCreatureFrameSetManager::CCreatureFrameSetManager()
{
}

CCreatureFrameSetManager::~CCreatureFrameSetManager()
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
// CreatureFrameSet�� ���õ� SpriteID�� �����ؼ�
// SpritePack IndexFile�κ��� SpriteSet IndexFile�� �����Ѵ�.
//----------------------------------------------------------------------
bool		
CCreatureFrameSetManager::SaveSpriteSetIndex(CCreatureFramePack* pCreatureFramePack, 
											 std::ofstream& setIndex, std::ifstream& packIndex)
{
	CSpriteSetManager ssm;

	
	ACTION_FRAME_ARRAY*		pActionArray;
	DIRECTION_FRAME_ARRAY*	pDirectionArray ;
	FRAME_ARRAY*			pFrameArray;

	DATA_LIST::iterator iData = m_List.begin();

	//------------------------------------------------------------------
	// ���õ� FrameID�� ���õ� ��� SpriteID�� �˾Ƴ��� �Ѵ�.	
	//------------------------------------------------------------------
	while (iData != m_List.end())
	{
		// FRAME_ARRAY�� �о�´�.
		pActionArray = &((*pCreatureFramePack)[*iData]);

		//--------------------------------------------------------------
		// ������ ACTION_FRAME_ARRAY�� ���� ..
		// DIRECTION_FRAME_ARRAY�� ����..
		// FRAME_ARRAY�� ���� Frame�� SpriteID�� 
		// CSpriteSetManager�� �����Ѵ�. - -;;
		//--------------------------------------------------------------
		for (int action=0; action<pActionArray->GetSize(); action++)
		{
			pDirectionArray = &((*pActionArray)[action]);
			for (int direction=0; direction<pDirectionArray->GetSize(); direction++)
			{
				pFrameArray = &((*pDirectionArray)[direction]);
				for (int frame=0; frame<pFrameArray->GetSize(); frame++)
				{
					ssm.Add( (*pActionArray)[action][direction][frame].GetSpriteID() );
				}
			}
		}

		iData++;
	}

	//------------------------------------------------------------------
	// ThingFrameSet�� ���õ� ��� SpriteID�� 
	// SpriteSetManager�� ���������Ƿ� 
	// SpriteSetManager�� �̿��� SpriteSetIndex�� �����ϸ�ȴ�.
	//------------------------------------------------------------------
	return ssm.SaveSpriteSetIndex(setIndex, packIndex);
}

