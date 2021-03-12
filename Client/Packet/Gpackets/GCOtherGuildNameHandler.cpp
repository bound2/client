//////////////////////////////////////////////////////////////////////////////
// Filename    : GCOtherGuildName.cc
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Client_PCH.h"
#include "GCOtherGuildName.h"
#include "ClientDef.h"
#include "UIFunction.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void GCOtherGuildNameHandler::execute(GCOtherGuildName* pPacket, Player* pPlayer) 
	 throw ( Error )
{
	__BEGIN_TRY
	// Other Info �˻��Ŀ�-_- ��Ƽ �Ŵ����� �˻��ؼ� ����̸��� �־��ش�.

	UI_SetGuildName(pPacket->getGuildID(), pPacket->getGuildName());
	
	__END_CATCH
}