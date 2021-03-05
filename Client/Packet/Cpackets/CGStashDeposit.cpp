////////////////////////////////////////////////////////////////////////////////
// 
// Filename    : CGStashDeposit.cpp 
// Written By  : 김성민
// Description : 
// 
////////////////////////////////////////////////////////////////////////////////

// include files
#include "Client_PCH.h"
#include "CGStashDeposit.h"

////////////////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
////////////////////////////////////////////////////////////////////////////////
void CGStashDeposit::read ( SocketInputStream & iStream ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	__BEGIN_DEBUG
		
	iStream.read( m_Amount );

	__END_DEBUG
	__END_CATCH
}

		    
////////////////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
////////////////////////////////////////////////////////////////////////////////
void CGStashDeposit::write ( SocketOutputStream & oStream ) const 
     throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	__BEGIN_DEBUG

	oStream.write( m_Amount );

	__END_DEBUG
	__END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// execute packet's handler
////////////////////////////////////////////////////////////////////////////////
void CGStashDeposit::execute ( Player * pPlayer ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	__BEGIN_DEBUG

	#ifndef __GAME_CLIENT__
		CGStashDepositHandler::execute ( this , pPlayer );
	#endif
		
	__END_DEBUG
	__END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#ifdef __DEBUG_OUTPUT__
	std::string CGStashDeposit::toString () 
		const throw ()
	{
		__BEGIN_TRY
		__BEGIN_DEBUG
			
		StringStream msg;

		msg << "CGStashDeposit("
			<< "Amount : " << (int)m_Amount
			<< ")";

		return msg.toString();

		__END_DEBUG
		__END_CATCH
	}
#endif