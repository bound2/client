////////////////////////////////////////////////////////////////////////////////
// Filename    : GCTradeMoney.cpp 
// Written By  : �輺��
// Description :
////////////////////////////////////////////////////////////////////////////////

// include files
#include "Client_PCH.h"
#include "GCTradeMoney.h"

////////////////////////////////////////////////////////////////////////////////
// �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ�� �ʱ�ȭ�Ѵ�.
////////////////////////////////////////////////////////////////////////////////
void GCTradeMoney::read ( SocketInputStream & iStream ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY

	iStream.read( m_TargetObjectID );
	iStream.read( m_Gold );
	iStream.read( m_Code );
		
	__END_CATCH
}

		    
////////////////////////////////////////////////////////////////////////////////
// ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
////////////////////////////////////////////////////////////////////////////////
void GCTradeMoney::write ( SocketOutputStream & oStream ) const 
     throw ( ProtocolException , Error )
{
	__BEGIN_TRY

	oStream.write( m_TargetObjectID );
	oStream.write( m_Gold );
	oStream.write( m_Code );
	
	__END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
// execute packet's handler
////////////////////////////////////////////////////////////////////////////////
void GCTradeMoney::execute ( Player * pPlayer ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
		
	GCTradeMoneyHandler::execute( this , pPlayer );

	__END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
// get packet's debug string
////////////////////////////////////////////////////////////////////////////////
#ifdef __DEBUG_OUTPUT__
	std::string GCTradeMoney::toString () const
		   throw ()
	{
		__BEGIN_TRY
			
		StringStream msg;
		msg << "GCTradeMoney("
			<< "TargetObjectID : " << (int)m_TargetObjectID << ","
			<< "Gold : "           << (int)m_Gold           << ","
			<< "CODE : "           << (int)m_Code
			<< ")";
		return msg.toString();
			
		__END_CATCH
	}

#endif
