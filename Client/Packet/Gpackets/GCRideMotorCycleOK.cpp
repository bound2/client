//////////////////////////////////////////////////////////////////////
// 
// Filename    : GCRideMotorCycleOK.cpp 
// Written By  : elca@ewestsoft.com
// Description : 자신에게 쓰는 기술의 성공을 알리기 위한 패킷 클래스의
//               멤버 정의.
// 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////

#include "Client_PCH.h"
#include "GCRideMotorCycleOK.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCRideMotorCycleOK::GCRideMotorCycleOK () 
     throw ()
{
	__BEGIN_TRY
	__END_CATCH
}

	
//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCRideMotorCycleOK::~GCRideMotorCycleOK () 
    throw ()
{
	__BEGIN_TRY
	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCRideMotorCycleOK::read ( SocketInputStream & iStream ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
		
	// 최적화 작업시 실제 크기를 명시하도록 한다.
	iStream.read( m_ObjectID );

	__END_CATCH
}

		    
//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCRideMotorCycleOK::write ( SocketOutputStream & oStream ) 
     const throw ( ProtocolException , Error )
{
	__BEGIN_TRY
		
	// 최적화 작업시 실제 크기를 명시하도록 한다.
	oStream.write( m_ObjectID );

	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// execute packet's handler
//
//////////////////////////////////////////////////////////////////////
void GCRideMotorCycleOK::execute ( Player * pPlayer ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
		
	GCRideMotorCycleOKHandler::execute( this , pPlayer );
		
	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug std::string
//
//////////////////////////////////////////////////////////////////////
#ifdef __DEBUG_OUTPUT__
	std::string GCRideMotorCycleOK::toString () 
		const throw ()
	{
		__BEGIN_TRY

		StringStream msg;

		msg << "GCRideMotorCycleOK( ObjectID:" << (int)m_ObjectID 
			<< " )";

		return msg.toString();

		__END_CATCH
	}
#endif
