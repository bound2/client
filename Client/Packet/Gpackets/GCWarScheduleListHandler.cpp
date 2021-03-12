//////////////////////////////////////////////////////////////////////
//
// Filename    : GCWarScheduleListHandler.cpp
// Written By  : 
//
//////////////////////////////////////////////////////////////////////

// include files
#include "Client_PCH.h"
#include "GCWarScheduleList.h"
#include "clientdef.h"
#include "UIFunction.h"


//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void GCWarScheduleListHandler::execute ( GCWarScheduleList * pPacket , Player * pPlayer )
	 throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	
#ifdef __GAME_CLIENT__
	
	UI_RunWarList(pPacket);

#endif

	__END_CATCH
}