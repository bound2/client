//----------------------------------------------------------------------
// MInteractionObject.h
//----------------------------------------------------------------------
//
// Action�� �ִ� ImageObject
// 
//----------------------------------------------------------------------
//
// ��, ������ Action�� ���ؼ��� Animation�� ����.
// �ϳ��� Action�� �ϳ��� Frame�� �����Ѵ�.
//
//----------------------------------------------------------------------

#ifndef	__MINTERACTIONOBJECT_H__
#define	__MINTERACTIONOBJECT_H__

#include "MTypeDef.h"
#include "MAnimationObject.h"
#include <fstream>

//----------------------------------------------------------------------
//
// InteractionObject class
//
//----------------------------------------------------------------------
class MInteractionObject : public MAnimationObject {
	public :
		MInteractionObject();
		MInteractionObject(TYPE_INTERACTIONOBJECTTYPE iaotype, TYPE_OBJECTID id, TYPE_OBJECTID ImageObjectID, TYPE_SPRITEID nSprite, int pX, int pY, TYPE_SECTORPOSITION viewpoint, bool trans, BYTE type=BLT_NORMAL);
		~MInteractionObject();

		//--------------------------------------------------------
		// Action�� ���õ� �Լ�
		//--------------------------------------------------------		
		void			SetAction(BYTE action);
		BYTE			GetAction() const			{ return m_CurrentFrame; }

		//--------------------------------------------------------		
		// action�� ���� action�� �����Ѵ�.
		//--------------------------------------------------------		
		void			SetNextAction(BYTE action);

		//--------------------------------------------------------		
		// InteractionObject type
		//--------------------------------------------------------		
		BYTE			GetInteractionObjectType() const	{ return m_InteractionObjectType; }

		//-------------------------------------------------------
		// File I/O
		//-------------------------------------------------------
		virtual void	SaveToFile(std::ofstream& file);
		virtual void	LoadFromFile(std::ifstream& file);

	public :
		enum INTERACTIONOBJECT_TYPE 
		{
			TYPE_DOOR = 0,
			TYPE_TRAP,
			TYPE_SWITCH,
			TYPE_MAX
		};
		
	protected :
		// Action�� �ٲ۴�.
		void			ChangeActionDoor();
		void			ChangeActionTrap();
		void			ChangeActionSwitch();

	protected :
		BYTE			m_InteractionObjectType;
};


#endif


