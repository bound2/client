//////////////////////////////////////////////////////////////////////////////
// Filename    : CGRequestGuildListHandler.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////
#include "Client_PCH.h"
#include "CGRequestGuildList.h"
#ifdef __GAME_SERVER__
	#include "SystemAvailabilitiesManager.h"
	#include "Assert.h"
#endif	// __GAME_SERVER__

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifndef __GAME_CLIENT__	
void CGRequestGuildListHandler::execute (CGRequestGuildList* pPacket , Player* pPlayer)
	 throw (Error)
{
	__BEGIN_TRY 
		
#ifdef __GAME_SERVER__

	
#endif	// __GAME_SERVER__
		
	__END_CATCH
}
#endif
