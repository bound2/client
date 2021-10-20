//----------------------------------------------------------------------
// CFrame.h
//----------------------------------------------------------------------
//
// CFrame�� �ϳ��� Sprite�� ���� ������ ������.
//
//----------------------------------------------------------------------
//
// m_ID�� ���Ƿ� ������ SpritePack/SpriteSet������ Sprite ID�̴�.
//
// m_cX, m_cY�� ��ǥ ���������μ� Sprite�� ��µɶ� �����¿�� 
//           ��ȭ�ϴ� ���̴�. 
//           (��) ĳ���Ϳ� �Ȱ��� ���� ��,
//                ĳ���� Sprite�� m_cX=0, m_cY=0���� �Ѵٸ�
//                �Ȱ� Sprite�� m_cX=20, m_cY=10���� �� �� �ְ���.
//
// m_fEffect�� ��¹���� ���� �����̴�.
//        Mirror��� ����.
//----------------------------------------------------------------------

#ifndef	__CFRAME_H__
#define	__CFRAME_H__

#include <Windows.h>
#include "fstream"
#include "DrawTypeDef.h"
#include "TArray.h"


//----------------------------------------------------------------------
// Frame�� ����ϴµ� �־ ȿ���� �ִ� FLAG
//----------------------------------------------------------------------
//#define	FLAG_FRAME_MIRROR		0x01



class CFrame {
	public :
		CFrame(TYPE_SPRITEID spriteID=0, short cx=0, short cy=0)
		{
			Set(spriteID, cx, cy);
		}

		//~CFrame() {}

		//---------------------------------------------------------------
		// set
		//---------------------------------------------------------------
		void	Set(TYPE_SPRITEID spriteID, short cx, short cy);		

		//---------------------------------------------------------------
		// file I/O
		//---------------------------------------------------------------
		void	SaveToFile(std::ofstream& file);
		void	LoadFromFile(std::ifstream& file);

		//---------------------------------------------------------------
		// get		
		//---------------------------------------------------------------
		TYPE_SPRITEID	GetSpriteID()	const	{ return m_SpriteID; }
		short	GetCX()		const		{ return m_cX; }
		short	GetCY()		const		{ return m_cY; }

		//---------------------------------------------------------------
		// assign
		//---------------------------------------------------------------
		void	operator = (const CFrame& frame);


		//---------------------------------------------------------------
		// flag
		//---------------------------------------------------------------
		//void	UnSetAll()			{ m_fEffect = 0; }
		//BYTE	GetEffectFlag()		{ return m_fEffect; }

		// set flag
		//void	SetMirror()			{ m_fEffect |= FLAG_FRAME_MIRROR; }

		// unset flag
		//void	UnSetMirror()		{ m_fEffect &= ~FLAG_FRAME_MIRROR; }

		// is flag set?
		//bool	IsMirror()			{ return m_fEffect & FLAG_FRAME_MIRROR; }


	protected :
		TYPE_SPRITEID	m_SpriteID;		// SpriteSurface�� ��ȣ(0~65535)		

		// ��ǥ ������
		short	m_cX;			
		short	m_cY;

		//BYTE			m_fEffect;		// ��¹���� ���� flag
};


//----------------------------------------------------------------------
// Effect�� ���� Frame
//----------------------------------------------------------------------
class CEffectFrame : public CFrame {
	public :
		CEffectFrame(TYPE_SPRITEID spriteID=0, short cx=0, short cy=0, char light=0, bool bBack = false)
		{
//			m_bBackground = bBack;
			Set(spriteID, cx, cy, light, bBack);
		};

		void	Set(TYPE_SPRITEID spriteID, short cx, short cy, char light, bool bBack)
		{
			CFrame::Set(spriteID, cx, cy);
			m_Light = light;
			m_bBackground = bBack;
		}

		void	SetBackground()				{ m_bBackground = true; }
		void	UnSetBackground()			{ m_bBackground = false; }

		//---------------------------------------------------------------
		// File I/O
		//---------------------------------------------------------------
		void	SaveToFile(std::ofstream& file);
		void	LoadFromFile(std::ifstream& file);

		// Get
		char	GetLight() const		{ return m_Light; }		
		bool	IsBackground() const	{ return m_bBackground; }

		//---------------------------------------------------------------
		// assign
		//---------------------------------------------------------------
		void	operator = (const CEffectFrame& frame);


	
	protected :
		char		m_Light;		// �þ��� ũ��(���� ���)
		bool		m_bBackground;
};


//----------------------------------------------------------------------
//
// Frame Array  data type ����
//
//----------------------------------------------------------------------

// FrameArray
typedef	TArray<CFrame, WORD>					FRAME_ARRAY;

// Direction FrameArray	
typedef	TArray<FRAME_ARRAY, BYTE>				DIRECTION_FRAME_ARRAY;

// Action FrameArray
typedef	TArray<DIRECTION_FRAME_ARRAY, BYTE>		ACTION_FRAME_ARRAY;


//----------------------------------------------------------------------
// Effect Frame
//----------------------------------------------------------------------
// FrameArray
typedef	TArray<CEffectFrame, WORD>					EFFECTFRAME_ARRAY;

// Direction FrameArray	
typedef	TArray<EFFECTFRAME_ARRAY, BYTE>				DIRECTION_EFFECTFRAME_ARRAY;


#endif
