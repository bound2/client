//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddInjuriousCreatureHandler.cc
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////
#include "Client_PCH.h"
#include "GCAddInjuriousCreature.h"
#include "MJusticeAttackManager.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void GCAddInjuriousCreatureHandler::execute (GCAddInjuriousCreature* pPacket , Player* pPlayer)
	 throw (ProtocolException , Error)
{
	__BEGIN_TRY

	if (g_pJusticeAttackManager!=NULL)
	{
		g_pJusticeAttackManager->AddCreature( pPacket->getName().c_str() );
	}

	__END_CATCH
}
