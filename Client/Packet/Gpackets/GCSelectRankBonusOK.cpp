//////////////////////////////////////////////////////////////////////
// 
// Filename    : GCSelectRankBonusOK.cc 
// Written By  : elca@ewestsoft.com
// Description : Skill이 실패 했을때 날려주는 패킷의 멤버 정의
// 
//////////////////////////////////////////////////////////////////////

// include files
#include "Client_PCH.h"
#include "GCSelectRankBonusOK.h"



//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCSelectRankBonusOK::GCSelectRankBonusOK () 
     throw ()
{
	__BEGIN_TRY
	__END_CATCH
}

	
//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCSelectRankBonusOK::~GCSelectRankBonusOK () 
    throw ()
{
	__BEGIN_TRY
	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCSelectRankBonusOK::read ( SocketInputStream & iStream ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	iStream.read( m_RankBonusType );
	__END_CATCH
}

		    
//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCSelectRankBonusOK::write ( SocketOutputStream & oStream ) const 
     throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	oStream.write( m_RankBonusType );
	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
// execute packet's handler
//////////////////////////////////////////////////////////////////////
void GCSelectRankBonusOK::execute ( Player * pPlayer ) 
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
		
	GCSelectRankBonusOKHandler::execute( this , pPlayer );
		
	__END_CATCH
}

#ifdef __DEBUG_OUTPUT__
//////////////////////////////////////////////////////////////////////
//
// get packet's debug std::string
//
//////////////////////////////////////////////////////////////////////
std::string GCSelectRankBonusOK::toString () const
       throw ()
{
	__BEGIN_TRY

	StringStream msg;
	msg << "GCSelectRankBonusOK("
		<< "RankBonusType:" << m_RankBonusType
		<< ")" ;
	return msg.toString();

	__END_CATCH
}
#endif