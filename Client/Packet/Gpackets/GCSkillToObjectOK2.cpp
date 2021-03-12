//////////////////////////////////////////////////////////////////////
// 
// Filename    : GCSkillToObjectOK2.cpp 
// Written By  : elca@ewestsoft.com
// Description : �ڽſ��� ���� ����� ������ �˸��� ���� ��Ŷ Ŭ������
//               ��� ����.
// 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////

#include "Client_PCH.h"
#include "GCSkillToObjectOK2.h"



//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCSkillToObjectOK2::GCSkillToObjectOK2 () 
     throw ()
{
	__BEGIN_TRY


	__END_CATCH
}

	
//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCSkillToObjectOK2::~GCSkillToObjectOK2 () 
    throw ()
{
	__BEGIN_TRY
	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
// �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ�� �ʱ�ȭ�Ѵ�.
//////////////////////////////////////////////////////////////////////
void GCSkillToObjectOK2::read ( SocketInputStream & iStream ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
		
	// ����ȭ �۾��� ���� ũ�⸦ �����ϵ��� �Ѵ�.
	iStream.read( m_ObjectID );
	iStream.read( m_SkillType );
	iStream.read( m_Duration );
	iStream.read( m_Grade );
	ModifyInfo::read( iStream );


	__END_CATCH
}

		    
//////////////////////////////////////////////////////////////////////
// ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
//////////////////////////////////////////////////////////////////////
void GCSkillToObjectOK2::write ( SocketOutputStream & oStream ) 
     const throw ( ProtocolException , Error )
{
	__BEGIN_TRY
		
	// ����ȭ �۾��� ���� ũ�⸦ �����ϵ��� �Ѵ�.
	oStream.write( m_ObjectID );
	oStream.write( m_SkillType );
	oStream.write( m_Duration );
	oStream.write( m_Grade );

	ModifyInfo::write( oStream );
	
	__END_CATCH
}
//////////////////////////////////////////////////////////////////////
//
// execute packet's handler
//
//////////////////////////////////////////////////////////////////////
void GCSkillToObjectOK2::execute ( Player * pPlayer ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
		
	GCSkillToObjectOK2Handler::execute( this , pPlayer );
		
	__END_CATCH
}


#ifdef __DEBUG_OUTPUT__
//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
std::string GCSkillToObjectOK2::toString () 
	const throw ()
{
	__BEGIN_TRY

	StringStream msg;
	msg << "GCSkillToObjectOK2("
		<< "SkillType:" << (int)m_SkillType 
		<< ",ObjectID:" << (int)m_ObjectID 
		<< ",Duration:" << (int)m_Duration
		<< ",Grade:" << (int)m_Grade;
	msg << ModifyInfo::toString();
	msg << ")";
	return msg.toString();

	__END_CATCH
}


#endif