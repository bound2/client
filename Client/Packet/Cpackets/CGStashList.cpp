////////////////////////////////////////////////////////////////////////////////
// 
// Filename    : CGStashList.cpp 
// Written By  : 김성민
// Description : 
// 
////////////////////////////////////////////////////////////////////////////////

// include files
#include "Client_PCH.h"
#include "CGStashList.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

////////////////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
////////////////////////////////////////////////////////////////////////////////
void CGStashList::read ( SocketInputStream & iStream ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	__BEGIN_DEBUG
	
	iStream.read(m_ObjectID);

	__END_DEBUG
	__END_CATCH
}

		    
////////////////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
////////////////////////////////////////////////////////////////////////////////
void CGStashList::write ( SocketOutputStream & oStream ) const 
     throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	__BEGIN_DEBUG

	oStream.write(m_ObjectID);

	__END_DEBUG
	__END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// execute packet's handler
////////////////////////////////////////////////////////////////////////////////
void CGStashList::execute ( Player * pPlayer ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	__BEGIN_DEBUG

	#ifndef __GAME_CLIENT__
		CGStashListHandler::execute ( this , pPlayer );
	#endif
		
	__END_DEBUG
	__END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// get debug std::string
////////////////////////////////////////////////////////////////////////////////
#ifdef __DEBUG_OUTPUT__
	std::string CGStashList::toString () 
		const throw ()
	{
		__BEGIN_TRY
		__BEGIN_DEBUG

		StringStream msg;
		msg << "CGStashList(" 
			<< "ObjectID : " << std::to_string(m_ObjectID)
			<< ")";
		return msg.toString();

		__END_DEBUG
		__END_CATCH
	}
#endif