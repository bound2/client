//////////////////////////////////////////////////////////////////////
//
// Filename    : GCShowGuildInfoHandler.cpp
// Written By  : 
//
//////////////////////////////////////////////////////////////////////

// include files
#include "Client_PCH.h"
#include "GCShowGuildInfo.h"
#include "ClientDef.h"
#include "UIFunction.h"
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void GCShowGuildInfoHandler::execute ( GCShowGuildInfo * pPacket , Player * pPlayer )
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	
#ifdef __GAME_CLIENT__
	//cout << pPacket->toString() << endl;
	UI_ShowGuildInfo(pPacket);
#endif

	__END_CATCH
}
