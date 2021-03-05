//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMiniGameScoresHandler.cc
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "Client_PCH.h"
#include "GCMiniGameScores.h"
#include "ClientDef.h"
#include "UIFunction.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void GCMiniGameScoresHandler::execute ( GCMiniGameScores * pGCMiniGameScores , Player * pPlayer )
	 throw ( Error )
{	
	UI_MiniGameScores( pGCMiniGameScores );
}
