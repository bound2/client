//----------------------------------------------------------------------
// CAnimationFrame.h
//----------------------------------------------------------------------
// Frame�� �ٲ�鼭 Animation�� �Ǵ� �Ϳ� ���� class
//----------------------------------------------------------------------
//
// � FramePack������ Frame ID�� 
// CurrentFrame/MaxFrame�� ������ ���´�.
//
//----------------------------------------------------------------------
//
// [ File I/O ]
//
// Frame ID�� ����ȴ�. MaxFrame�� �ٲ� �� �����Ƿ� ���� �ÿ� ���� �����Ѵ�.
// CurrentFrame�� �� 0���� ���۵ȴ�.
// Frame ID�� ������ MaxFrame�� ���� ������ �� �� �ִ�.
// �׷���, � CAnimationFrame�� ���õ� FramePack�� ������ �� ���� ����.
// ��¶�ų�.. �ܺο��� �� ������ �������شٰ� ����.. - -;;
//
//----------------------------------------------------------------------

#ifndef	__CANIMATIONFRAME_H__
#define	__CANIMATIONFRAME_H__

#include <Windows.h>
#include "DrawTypeDef.h"
#include "fstream"


class CAnimationFrame {
	public :
		CAnimationFrame(BYTE bltType=BLT_NORMAL);
		~CAnimationFrame();

		//--------------------------------------------------------
		// �⺻ Frame
		//--------------------------------------------------------
		void			SetFrameID(TYPE_FRAMEID FrameID, BYTE max)	{ m_FrameID = FrameID; m_MaxFrame=max; m_CurrentFrame=0; }
		TYPE_FRAMEID	GetFrameID() const		{ return m_FrameID; }
		BYTE			GetFrame() const		{ return m_CurrentFrame; }
		BYTE			GetMaxFrame() const		{ return m_MaxFrame; }

		// 
		void			NextFrame()		 		{ if (++m_CurrentFrame==m_MaxFrame) m_CurrentFrame=0; }

		//--------------------------------------------------------
		// file I/O
		//--------------------------------------------------------
		void	SaveToFile(std::ofstream& file);
		void	LoadFromFile(std::ifstream& file);

		//-------------------------------------------------------
		// ��� ���
		//-------------------------------------------------------
		void	SetBltType(BYTE bltType)	{ m_BltType = bltType; }
		BYTE	GetBltType() const			{ return m_BltType; }
		bool	IsBltTypeNormal() const		{ return m_BltType==BLT_NORMAL; }
		bool	IsBltTypeEffect() const		{ return m_BltType==BLT_EFFECT; }
		bool	IsBltTypeShadow() const		{ return m_BltType==BLT_SHADOW; }
		bool	IsBltTypeScreen() const		{ return m_BltType==BLT_SCREEN; }


	protected :		
		// ���� frame�� ���� ���� : CThingFramePack�� ���� ���̴�.	
		TYPE_FRAMEID		m_FrameID;

		// Animation Frame�� ���� ����
		BYTE				m_CurrentFrame;	// Current Frame
		BYTE				m_MaxFrame;		// Max Frame
	
		// ��� ���
		BYTE				m_BltType;
};

#endif

