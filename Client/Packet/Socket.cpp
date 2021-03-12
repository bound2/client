//////////////////////////////////////////////////////////////////////
// 
// Socket.cpp
// 
// by Reiot
// 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////
#include "Client_PCH.h"
#include "Socket.h"
#include "Assert.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
Socket::Socket () 
	throw () 
: m_pSocketImpl(NULL) 
{ 
	__BEGIN_TRY

	m_pSocketImpl = new SocketImpl(); 
	m_pSocketImpl->create();
	
	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
Socket::Socket ( const std::string & host , uint port ) 
	throw () 
: m_pSocketImpl(NULL) 
{ 
	__BEGIN_TRY

	m_pSocketImpl = new SocketImpl(host,port); 
	m_pSocketImpl->create();
	
	__END_CATCH
}
	

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
Socket::Socket ( SocketImpl * impl ) 
	throw () 
: m_pSocketImpl(NULL) 
{ 
	__BEGIN_TRY

	Assert( impl != NULL );

	m_pSocketImpl = impl;
	
	__END_CATCH
}
	

//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
Socket::~Socket () 
	throw ( Error ) 
{ 
	__BEGIN_TRY

	if ( m_pSocketImpl != NULL ) { 
		delete m_pSocketImpl; 
		m_pSocketImpl = NULL; 
	} 
	
	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
// close previous connection and connect to another server socket
//////////////////////////////////////////////////////////////////////
void Socket::reconnect ( const std::string & host , uint port )
	throw ( ConnectException , Error )
{
	__BEGIN_TRY

	// delete old socket impl object
	delete m_pSocketImpl;

	// create new socket impl object
	m_pSocketImpl = new SocketImpl( host , port );
	m_pSocketImpl->create();

	// try to connect
	m_pSocketImpl->connect();	

	__END_CATCH
}


//////////////////////////////////////////////////////////////////////
// return debug string
//////////////////////////////////////////////////////////////////////
std::string Socket::toString () const
       throw ()
{
	__BEGIN_TRY
		
	StringStream msg;

	msg << "Socket(FD:" << getSOCKET() << ",HOST:" << getHost() << ",PORT:" << getPort() << ")" ;
	
	return msg.toString();

	__END_CATCH
}