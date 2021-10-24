//----------------------------------------------------------------------
// CFramePack.h
//----------------------------------------------------------------------
// �Ʒ��� FramePack���� �����ؼ� ����Ѵ�.
//
// typedef CFramePack<FRAME_ARRAY>				CThingFramePack;
// typedef	CFramePack<DIRECTION_FRAME_ARRAY>	CEffectFramePack;
// typedef	CFramePack<ACTION_FRAME_ARRAY>		CCreatureFramePack;
//
//
// - ThingFramePack    : ���⼺�� ���� Thing�� ǥ��
// - EffectFramePack   : ���⼺�� �ִ� Effect�� ǥ��
// - CreatureFramePack : Action�� ���⼺�� �ִ� Creature�� ǥ��
//
//----------------------------------------------------------------------

#ifndef	__CFRAMEPACK_H__
#define	__CFRAMEPACK_H__


#include "DrawTypeDef.h"
#include "CFrame.h"
#include <fstream>

template <class Type>
class CFramePack : public TArray<Type, TYPE_FRAMEID> {
	public :
		CFramePack();
		~CFramePack();

		
		//--------------------------------------------------------
		// File I/O
		//--------------------------------------------------------
		bool		SaveToFile(std::ofstream& packFile, std::ofstream& indexFile);		

	protected :

};


//----------------------------------------------------------------------
// CFramePack<Type>.cpp
//----------------------------------------------------------------------

//#include "CFramePack.h"

//----------------------------------------------------------------------
//
// constructor/destructor
//
//----------------------------------------------------------------------
template <class Type>
CFramePack<Type>::CFramePack<Type>()
{
}

template <class Type>
CFramePack<Type>::~CFramePack<Type>()
{
}

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Save To File
//----------------------------------------------------------------------
// CreatureFramePack File�� �����ϰ�
// CreatureFramePack IndexFile�� �����ؾ� �Ѵ�.
//----------------------------------------------------------------------
template <class Type>
bool
CFramePack<Type>::SaveToFile(std::ofstream& packFile, std::ofstream& indexFile)
{
	//--------------------------------------------------
	// Size���� : 0�̶� ������ �����Ѵ�.
	//--------------------------------------------------
	packFile.write((const char*)&m_Size, s_SIZEOF_SizeType);
	indexFile.write((const char *)&m_Size, s_SIZEOF_SizeType); 

	// �ƹ��͵� ������..
	if (m_pData==NULL || m_Size==0) 
		return false;

	//--------------------------------------------------
	// index file�� �����ϱ� ���� ����
	//--------------------------------------------------
	long*	pIndex = new long [m_Size];

	//--------------------------------------------------
	//
	// CreatureFramePack ��� ����
	//
	//--------------------------------------------------
	for (TYPE_FRAMEID i=0; i<m_Size; i++)
	{
		// CreatureFrame�� file�� �������� index�� ����
		pIndex[i] = packFile.tellp();

		// CreatureFrame����
		m_pData[i].SaveToFile(packFile);
	}

	//--------------------------------------------------
	// index ����
	//--------------------------------------------------
	for (int i=0; i<m_Size; i++)
	{
		indexFile.write((const char*)&pIndex[i], 4);
	}

	delete [] pIndex;

	return true;
}

//----------------------------------------------------------------------
// FramePack�� define�Ѵ�.
//----------------------------------------------------------------------
typedef CFramePack<CFrame>							CImageFramePack;
typedef CFramePack<FRAME_ARRAY>						CAnimationFramePack;
typedef	CFramePack<DIRECTION_FRAME_ARRAY>			CDirectionFramePack;
typedef	CFramePack<DIRECTION_EFFECTFRAME_ARRAY>		CEffectFramePack;
//typedef	CFramePack<ACTION_FRAME_ARRAY>				CCreatureFramePack;

/*
class CEffectFramePack : public CFramePack<DIRECTION_EFFECTFRAME_ARRAY> {
	public :
		void		InfoToFile(const char* filename);
};
*/

class CCreatureFramePack : public CFramePack<ACTION_FRAME_ARRAY> {
	public :
		void		InfoToFile(const char* filename);
};

#endif

