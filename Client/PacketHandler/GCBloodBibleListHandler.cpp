//////////////////////////////////////////////////////////////////////
//
// Filename    : GCBloodBibleListHandler.cc
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////
#include "Client_PCH.h"
#pragma warning(disable:4786)
// include files
#include "Gpackets/GCBloodBibleList.h"


#include "ClientDef.h"
#include "TalkBox.h"
#include "UIDialog.h"
#include "SystemAvailabilities.h"
#include "MGameStringTable.h"
#include "SafeFormat.h"
#include "TempInformation.h"

//////////////////////////////////////////////////////////////////////
//
// 클라이언트에서 서버로부터 메시지를 받았을때 실행되는 메쏘드이다.
//
//////////////////////////////////////////////////////////////////////
void GCBloodBibleListHandler::execute ( GCBloodBibleList * pPacket , Player * pPlayer )
	 
throw ( ProtocolException , Error )
{
	__BEGIN_TRY 
	
#ifdef __GAME_CLIENT__
	if (g_pPlayer==NULL
		|| g_pZone==NULL
		|| g_pUIDialog==NULL
		|| g_pPCTalkBox==NULL)
	{
		DEBUG_ADD("[Error] Some Object is NULL");
		return;
	}

	g_pPlayer->SetWaitVerifyNULL();

	g_pPCTalkBox->Release();		// 

	std::vector<ItemType_t>	BloodBibleList = pPacket->getList();

	g_pPCTalkBox->SetType( PCTalkBox::BLOOD_BIBLE_SIGN );

	char str[192];
	char str2[192];
	for(int i = 0; i< BloodBibleList.size(); i++)
	{
		// The two arguments are table lookups indexed by a packet field.
		// CTypeTable::operator[] only bounds-checks under _DEBUG, so
		// outside Debug a list entry the server made large enough read
		// past the end of the table; GetGameString() range-checks the id
		// in every build and answers "" for one it cannot resolve.
		SafeFormat::Format(str2, GetGameString(UI_STRING_MESSAGE_RENT_BLOOD_BIBLE2),
			GetGameString(UI_STRING_MESSAGE_BLOOD_BIBLE_ARMEGA+BloodBibleList[i]),
			GetGameString(STRING_MESSAGE_BLOOD_BIBLE_BONUS_ARMEGA+BloodBibleList[i]));
		sprintf(str, "%3d %s", BloodBibleList[i], str2);
		g_pPCTalkBox->AddString( str );
	}
	
	// 끝내기 추가
	std::string szMsg;
	szMsg += "999";
	szMsg += (*g_pGameStringTable)[UI_STRING_MESSAGE_RENT_LATER_BLOOD_BIBLE].GetString();
	g_pPCTalkBox->AddString( szMsg.c_str() );


	strcpy(str, (*g_pGameStringTable)[UI_STRING_MESSAGE_RENT_BLOOD_BIBLE].GetString());

	g_pPCTalkBox->SetContent( str );

	g_pUIDialog->PopupPCTalkDlg();
	g_pTempInformation->SetMode(TempInformation::MODE_SKILL_LEARN);

#endif

	__END_CATCH
}
