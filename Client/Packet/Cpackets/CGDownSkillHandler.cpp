//////////////////////////////////////////////////////////////////////////////
// Filename    : CGDownSkillHandler.cpp
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////
#include "Client_PCH.h"
#include "CGDownSkill.h"

#ifdef __GAME_SERVER__
	#include "GamePlayer.h"
	#include "SkillParentInfo.h"
	#include "SkillInfo.h"
	#include "SkillDomainInfoManager.h"
#endif	


#ifndef __GAME_CLIENT__
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGDownSkillHandler::execute (CGDownSkill* pPacket , Player* pPlayer)
	 throw (ProtocolException, Error)
{
	__BEGIN_TRY 

	__END_CATCH
}

#endif