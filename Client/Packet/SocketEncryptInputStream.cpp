//////////////////////////////////////////////////////////////////////
// 
// SocketEncryptInputStream.cpp
// 
// by Reiot
// 
//////////////////////////////////////////////////////////////////////

#include "Client_PCH.h"
#include "SocketEncryptInputStream.h"
#include "Assert.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
SocketEncryptInputStream::SocketEncryptInputStream ( Socket * sock , uint BufferLen ) 
	throw ( Error )
: SocketInputStream(sock, BufferLen)
{
	__BEGIN_TRY
	__END_CATCH
}
