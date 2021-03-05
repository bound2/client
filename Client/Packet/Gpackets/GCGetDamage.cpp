//////////////////////////////////////////////////////////////////////
// 
// Filename    : GCGetDamage.cc 
// Written By  : elca@ewestsoft.com
// Description : CGMove가 날아 왓을때 자기 자신에게 OK 사인을 날리기
//               위한 패킷 클래스 함수 정의
// 
//////////////////////////////////////////////////////////////////////

// include files
#include "Client_PCH.h"
#include "GCGetDamage.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCGetDamage::GCGetDamage () 
     throw ()
{
	__BEGIN_TRY
	__END_CATCH
}

	
//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCGetDamage::~GCGetDamage () 
    throw ()
{
	__BEGIN_TRY
	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCGetDamage::read ( SocketInputStream & iStream ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	iStream.read( (char*)&m_ObjectID , szObjectID );
	iStream.read( (char*)&m_GetDamage , szWORD );
	__END_CATCH
}

		    
//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCGetDamage::write ( SocketOutputStream & oStream ) const 
     throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	oStream.write( (char*)&m_ObjectID , szObjectID );
	oStream.write( (char*)&m_GetDamage , szWORD );
	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
// execute packet's handler
//////////////////////////////////////////////////////////////////////
void GCGetDamage::execute ( Player * pPlayer ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
		
	GCGetDamageHandler::execute( this , pPlayer );
		
	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug std::string
//
//////////////////////////////////////////////////////////////////////
#ifdef __DEBUG_OUTPUT__
	std::string GCGetDamage::toString () const
		   throw ()
	{
		__BEGIN_TRY

		StringStream msg;
		msg << "GCGetDamage ( ObjectID : " << (int)m_ObjectID << "  Damage : " << (int)m_GetDamage << " )";
		return msg.toString();

		__END_CATCH
	}

#endif
