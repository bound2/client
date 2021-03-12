//////////////////////////////////////////////////////////////////////
// 
// Filename    : GCUnburrowOK.cc 
// Written By  : crazydog
// Description : CGUnburrow�� ���� ������ �ڱ� �ڽſ��� OK ������ ������
//               ���� ��Ŷ Ŭ���� �Լ� ����
// 
//////////////////////////////////////////////////////////////////////

// include files
#include "Client_PCH.h"
#include "GCUnburrowOK.h"


//////////////////////////////////////////////////////////////////////
// �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ�� �ʱ�ȭ�Ѵ�.
//////////////////////////////////////////////////////////////////////
void GCUnburrowOK::read ( SocketInputStream & iStream ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY

	iStream.read( m_X );
	iStream.read( m_Y );
	iStream.read( m_Dir );

	__END_CATCH
}

		    
//////////////////////////////////////////////////////////////////////
// ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
//////////////////////////////////////////////////////////////////////
void GCUnburrowOK::write ( SocketOutputStream & oStream ) const 
     throw ( ProtocolException , Error )
{
	__BEGIN_TRY

	oStream.write( m_X );
	oStream.write( m_Y );
	oStream.write( m_Dir );

	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
// execute packet's handler
//////////////////////////////////////////////////////////////////////
void GCUnburrowOK::execute ( Player * pPlayer ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
		
	GCUnburrowOKHandler::execute( this , pPlayer );
		
	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
#ifdef __DEBUG_OUTPUT__
	std::string GCUnburrowOK::toString () const
		   throw ()
	{
		__BEGIN_TRY

		StringStream msg;
		msg << "GCUnburrowOK("
			<< "X:" << (int)m_X 
			<< ",Y:" << (int) m_Y 
			<< ",Dir:" << Dir2String[m_Dir] 
			<< ")";
		return msg.toString();

		__END_CATCH
	}
#endif
