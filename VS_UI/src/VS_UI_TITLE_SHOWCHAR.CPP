#include "client_PCH.h"
#include "VS_UI.h"

#include "VS_UI_filepath.h"
#include "VS_UI_Message.h"
#include "VS_UI_Mouse_pointer.h"
#include "VS_UI_GameCommon.h"
#include "UserOption.h"
#include "ClientConfig.h"

//#include "ExperienceTable.h"	//char_infoçæèçè·èµæ¯è²è¦ çæèçè·èµéèè
#include "MGuildMarkManager.h"
#include "KeyAccelerator.h"
#include "UserInformation.h"
#include <time.h>
#include "MItemOptionTable.H"
#include "mgamestringtable.H"
#include "SkinManager.h"
#include "MTypeDef.h"

int	g_ACAddonOrder[ DIRECTION_MAX ][ AC_ADDON_MAX ] =
{
	{ AC_ADDON_MOTOR, AC_ADDON_BODY, AC_ADDON_HELM, AC_ADDON_SHOULDER, AC_ADDON_RIGHTHAND, AC_ADDON_LEFTHAND },
	{ AC_ADDON_MOTOR, AC_ADDON_BODY, AC_ADDON_HELM, AC_ADDON_SHOULDER, AC_ADDON_RIGHTHAND, AC_ADDON_LEFTHAND },
	{ AC_ADDON_MOTOR, AC_ADDON_BODY, AC_ADDON_HELM, AC_ADDON_SHOULDER, AC_ADDON_LEFTHAND, AC_ADDON_RIGHTHAND },
	{ AC_ADDON_MOTOR, AC_ADDON_BODY, AC_ADDON_HELM, AC_ADDON_SHOULDER, AC_ADDON_LEFTHAND, AC_ADDON_RIGHTHAND },
	{ AC_ADDON_MOTOR, AC_ADDON_BODY, AC_ADDON_HELM, AC_ADDON_SHOULDER, AC_ADDON_LEFTHAND, AC_ADDON_RIGHTHAND },
	{ AC_ADDON_MOTOR, AC_ADDON_BODY, AC_ADDON_HELM, AC_ADDON_SHOULDER, AC_ADDON_LEFTHAND, AC_ADDON_RIGHTHAND },
	{ AC_ADDON_MOTOR, AC_ADDON_BODY, AC_ADDON_HELM, AC_ADDON_SHOULDER, AC_ADDON_RIGHTHAND, AC_ADDON_LEFTHAND },
	{ AC_ADDON_MOTOR, AC_ADDON_BODY, AC_ADDON_HELM, AC_ADDON_SHOULDER, AC_ADDON_RIGHTHAND, AC_ADDON_LEFTHAND },
};

int GetWeaponPartFromType( int type )
{
	switch( type )
	{
	case M_SWORD :
		return AC_SWORD;

	case M_BLADE :
		return AC_BLADE;

	case M_AR :
		return AC_GUN_AR;

	case M_SR :
	case M_SMG :
	case M_SG :
		return AC_GUN_SR;

	case M_MACE :
		return AC_MACE;

	case M_CROSS :
		return AC_CROSS;
	}
	return -1;
}			

int GetAdvancementSlayerMoveActionFromType( int type )
{
	switch( type )
	{
	case M_SWORD :
		return ACTION_ADVANCEMENT_SLAYER_MOVE_SWORD;

	case M_BLADE :
		return ACTION_ADVANCEMENT_SLAYER_MOVE_BLADE;

	case M_SR :
	case M_SMG :
	case M_SG :
	case M_AR :
		return ACTION_ADVANCEMENT_SLAYER_MOVE_GUN;
	}

	return ACTION_ADVANCEMENT_SLAYER_MOVE_MACE_AND_CROSS;
}

int	GetAdvancementSlayerStandActionFromType( int type )
{
	switch( type )
	{
	case M_SWORD :
		return ACTION_ADVANCEMENT_SLAYER_STOP_SWORD;

	case M_BLADE :
		return ACTION_ADVANCEMENT_SLAYER_STOP_BLADE;

	case M_SR :
	case M_SMG :
	case M_SG :
	case M_AR :
		return ACTION_ADVANCEMENT_SLAYER_STOP_GUN;		
	}

	return ACTION_ADVANCEMENT_SLAYER_STOP_MACE_AND_CROSS;
}

void	C_VS_UI_NEWCHAR::_ShowCharacterOusters( int _x, int _y, S_SLOT* p_slot, int index, int enable, int dark )
{
	const int DARK_BIT = 1;
	POINT point;
	int coat = p_slot->man_info.coat;
	int weapon = p_slot->man_info.right;
	
	int action = 0;
	if(enable)	// 莽聯炉茅聙垄猫碌麓
	{
		if(weapon == 0)	// 챈째혥챈혨혫챔쨉쨈 챕혷혨책혰혻책짬혺
			action = ACTION_OUSTERS_MOVE;
		else
			action = ACTION_MOVE;
	}
	else
	{
		if(weapon == 0)	// 챈째혥챈혨혫챔쨉쨈 챕혷혨책혰혻책짬혺
			action = ACTION_OUSTERS_STAND;
		else
			action = ACTION_STAND;
	}
	
	index %= m_ousters_cfpk[coat][action][2].GetSize();
	
	int coat_color = p_slot->coat_color;
	if(coat_color == QUEST_ITEM_COLOR )
		coat_color = MItem::GetQuestItemColorset();
	else
	{
		if(coat_color == UNIQUE_ITEM_COLOR)
			coat_color = MItem::GetUniqueItemColorset();
	}
	
	int weapon_color = p_slot->skin_color;
	if(weapon_color == QUEST_ITEM_COLOR )
		weapon_color = MItem::GetQuestItemColorset();
	else
	{
		if(weapon_color == UNIQUE_ITEM_COLOR)
			weapon_color = MItem::GetUniqueItemColorset();
	}
	
	int hair_color = p_slot->hair_color;
	if(hair_color == QUEST_ITEM_COLOR )
		hair_color = MItem::GetQuestItemColorset();
	else
	{
		if(hair_color == UNIQUE_ITEM_COLOR)
			hair_color = MItem::GetUniqueItemColorset();
	}
	
	int trouser_color = p_slot->trouser_color;
	
	if(trouser_color == QUEST_ITEM_COLOR )
		trouser_color = MItem::GetQuestItemColorset();
	else
	{
		if(trouser_color == UNIQUE_ITEM_COLOR)
			trouser_color = MItem::GetUniqueItemColorset();
	}
	
	
	
	CIndexSprite::SetUsingColorSet(hair_color, coat_color); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
	if(weapon)
		CIndexSprite::SetUsingColorSetOnly(2, 377);
	else
		CIndexSprite::SetUsingColorSetOnly(2, weapon_color);
	CIndexSprite::SetUsingColorSetOnly(3, trouser_color);
	
	point.x = _x+m_ousters_cfpk[coat][action][2][index].GetCX();
	point.y = _y+m_ousters_cfpk[coat][action][2][index].GetCY();
	
	const int sprite_id = m_ousters_cfpk[coat][action][2][index].GetSpriteID();
	
	if(dark)
		gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &m_ousters_ispk[sprite_id], DARK_BIT);
	else
		gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &m_ousters_ispk[sprite_id]);
	
	if(weapon)	// ì±ì§¹í»ì± íì§­ì±í·í£ ì±ì§¸í¥ì±ì¨ì§¸
	{
		index %= m_ousters_cfpk[0][action][2].GetSize();
		
		point.x = _x+m_ousters_cfpk[0][action][2][index].GetCX();
		point.y = _y+m_ousters_cfpk[0][action][2][index].GetCY();
		
		const int sprite_id = m_ousters_cfpk[0][action][2][index].GetSpriteID();
		
		CIndexSprite::SetUsingColorSet(weapon_color, weapon_color); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
		if(dark)
			gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &m_ousters_ispk[sprite_id], DARK_BIT);
		else
			gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &m_ousters_ispk[sprite_id]);
	}
}

void	C_VS_UI_NEWCHAR::_ShowCharacterSlayer( int _x, int _y, S_SLOT* p_slot, int index, int enable, int dark )
{
	try{
	const int DARK_BIT = 1;
	POINT point;

	MAN_INFO *info;
	if(p_slot->bl_female)
		info = (MAN_INFO *)&p_slot->woman_info;
	else
		info = &p_slot->man_info;
	
	// éè·¯èµæ¯èæ¥¼ éè¨èµéè²èµéèè¦ æ¯è­èµæ¯è¥é æ¯æ³è°çæèçèè£çè¹å½æ¯éè¹ çèéçè½è².. - -; by sigi
	int item_color = p_slot->trouser_color;
	if(item_color == QUEST_ITEM_COLOR )
		item_color = MItem::GetQuestItemColorset();
	else
	{
		if(item_color == UNIQUE_ITEM_COLOR)
			item_color = MItem::GetUniqueItemColorset();
	}
	
	CIndexSprite::SetUsingColorSet(p_slot->skin_color, item_color); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².		
	
	const int female_plus = p_slot->bl_female*m_slayer_cfpk.GetSize()/2;
	
	if(p_slot->bl_female && enable == 0)
		index = index%(m_slayer_cfpk[female_plus][enable][2].GetSize()-20)+20;
	else
		index %= m_slayer_cfpk[female_plus][enable][2].GetSize();
	
	
	int sprite_id = 0, part_id = 0;
	
	part_id = info->trouser +female_plus;
	sprite_id = m_slayer_cfpk[part_id][enable][2][index].GetSpriteID();
	
	// 				AssertSlayerSprite(sprite_id);
	
	point.x = _x+m_slayer_cfpk[part_id][enable][2][index].GetCX();
	point.y = _y+m_slayer_cfpk[part_id][enable][2][index].GetCY();
	
	if(dark)
		gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &m_slayer_ispk[sprite_id], DARK_BIT);
	else
		gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &m_slayer_ispk[sprite_id]);
	
	item_color=p_slot->coat_color;
	if(item_color == QUEST_ITEM_COLOR )
		item_color = MItem::GetQuestItemColorset();
	else
	{
		if(item_color == UNIQUE_ITEM_COLOR)
			item_color = MItem::GetUniqueItemColorset();
	}
	CIndexSprite::SetUsingColorSet(p_slot->skin_color, item_color); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
	
	part_id = info->coat +female_plus;
	sprite_id = m_slayer_cfpk[part_id][enable][2][index].GetSpriteID();
	
	// 				AssertSlayerSprite(sprite_id);
	
	point.x = _x+m_slayer_cfpk[part_id][enable][2][index].GetCX();
	point.y = _y+m_slayer_cfpk[part_id][enable][2][index].GetCY();
	
	if(dark)
		gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &m_slayer_ispk[sprite_id], DARK_BIT);
	else
		gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &m_slayer_ispk[sprite_id]);
	
	CIndexSprite::SetUsingColorSet(p_slot->skin_color, 0); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
	
	CIndexSprite::SetUsingColorSet(p_slot->hair_color, 0); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
	
	// éè¢å¢éç¢å¢éæ³è çè»è¢çèºè æ¯è²è¦æ¯è¤è° æ¯ç¯èµæ¯å¨å¢çé¹¿éºæ¯èºéçèºè çæ³è§çæèçè²è¤æ¯è¤è°æ¯è¥é.
	if (info->helmet == M_NO_WEAR )//|| info->helmet == M_HELMET1)
	{
		part_id = info->face +female_plus;
		sprite_id = m_slayer_cfpk[part_id][enable][2][index].GetSpriteID();
		
		// 					AssertSlayerSprite(sprite_id);
		
		point.x = _x+m_slayer_cfpk[part_id][enable][2][index].GetCX();
		point.y = _y+m_slayer_cfpk[part_id][enable][2][index].GetCY();
		
		if(dark)
			gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &m_slayer_ispk[sprite_id], DARK_BIT);
		else
			gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &m_slayer_ispk[sprite_id]);
	}
	else
	{
		item_color=p_slot->helmet_color;
		if(item_color == QUEST_ITEM_COLOR)
			item_color = MItem::GetQuestItemColorset();
		else
			if(item_color == UNIQUE_ITEM_COLOR)
				item_color = MItem::GetUniqueItemColorset();
			CIndexSprite::SetUsingColorSet(item_color, 0); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
			
			part_id = info->helmet +female_plus;
			sprite_id = m_slayer_cfpk[part_id][enable][2][index].GetSpriteID();
			
			// 						AssertSlayerSprite(sprite_id);
			
			point.x = _x+m_slayer_cfpk[part_id][enable][2][index].GetCX();
			point.y = _y+m_slayer_cfpk[part_id][enable][2][index].GetCY();
			
			if(dark)
				gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &m_slayer_ispk[sprite_id], DARK_BIT);
			else
				gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &m_slayer_ispk[sprite_id]);
	}
	
	// ì±ì§­ì¨ì±ì¨ì§¸ & ì±ì§¸ì§¤ì±í¯ì¨ì±ì¨ì§­
	if (info->right != M_NO_WEAR)//p_slot->bl_female && p_slot->woman_info.right != M_NO_WEAR || !p_slot->bl_female && p_slot->man_info.right != M_NO_WEAR)
	{
		item_color=p_slot->right_color;
		if(item_color == QUEST_ITEM_COLOR )				
			item_color = MItem::GetQuestItemColorset();
		else
			if(item_color == UNIQUE_ITEM_COLOR)
				item_color = MItem::GetUniqueItemColorset();
			
			CIndexSprite::SetUsingColorSet(item_color, 0); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
			
			part_id = info->right +female_plus;
			sprite_id = m_slayer_cfpk[part_id][enable][2][index].GetSpriteID();
			
			// 						AssertSlayerSprite(sprite_id);
			
			point.x = _x+m_slayer_cfpk[part_id][enable][2][index].GetCX();
			point.y = _y+m_slayer_cfpk[part_id][enable][2][index].GetCY();
			
			if(dark)
				gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &m_slayer_ispk[sprite_id], DARK_BIT);
			else
				gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &m_slayer_ispk[sprite_id]);
	}
	
	if (/*p_slot->bl_female && */info->left != M_NO_WEAR)
	{
		item_color=p_slot->left_color;
		if(item_color == QUEST_ITEM_COLOR )
			item_color = MItem::GetQuestItemColorset();
		else
			if(item_color == UNIQUE_ITEM_COLOR)
				item_color = MItem::GetUniqueItemColorset();
			
			CIndexSprite::SetUsingColorSet(item_color, 0); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
			
			part_id = info->left +female_plus;
			sprite_id = m_slayer_cfpk[part_id][enable][2][index].GetSpriteID();
			
			// 						AssertSlayerSprite(sprite_id);
			
			point.x = _x+m_slayer_cfpk[part_id][enable][2][index].GetCX();
			point.y = _y+m_slayer_cfpk[part_id][enable][2][index].GetCY();
			
			if(dark)
				gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &m_slayer_ispk[sprite_id], DARK_BIT);
			else
				gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &m_slayer_ispk[sprite_id]);
	}
	}catch(...){}
}
			

void	C_VS_UI_NEWCHAR::_ShowCharacterVampire( int _x, int _y, S_SLOT* p_slot, int index, int enable, int dark  )
{
	const int DARK_BIT = 1;
	POINT point;

	int coat = 0;
	if(p_slot->bl_female)
		coat = p_slot->woman_info.coat;
	else
		coat = p_slot->man_info.coat;
	
	index %= m_vampire_cfpk[coat][enable][2].GetSize();
	
	int coat_color = p_slot->coat_color;
	if(coat_color == QUEST_ITEM_COLOR )
		coat_color = MItem::GetQuestItemColorset();
	else
	{
		if(coat_color == UNIQUE_ITEM_COLOR)
			coat_color = MItem::GetUniqueItemColorset();
	}
	
	
	CIndexSprite::SetUsingColorSet(p_slot->skin_color, coat_color); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
	
	point.x = _x+m_vampire_cfpk[coat][enable][2][index].GetCX();
	point.y = _y+m_vampire_cfpk[coat][enable][2][index].GetCY();
	
	const int sprite_id = m_vampire_cfpk[coat][enable][2][index].GetSpriteID();
	
	// 				AssertVampireSprite(sprite_id);
	
	if(dark)
		gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &m_vampire_ispk[sprite_id], DARK_BIT);
	else
		gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &m_vampire_ispk[sprite_id]);
}

void	C_VS_UI_NEWCHAR::_ShowCharacterACOusters( int _x, int _y, S_SLOT* p_slot, int index, int enable, int dark )
{
	const int DARK_BIT = 1;
	POINT point;
	int coat = 1;//p_slot->man_info.coat;
	int weapon = p_slot->man_info.right;
	
	int action = 0;

	if(enable)	// 莽聯炉茅聙垄猫碌麓
	{
		if(weapon == 0)	// 챈째혥챈혨혫챔쨉쨈 챕혷혨책혰혻책짬혺
		{
			bool bFirstAction = index < m_AdvancementOustersCfpk[coat][ACTION_ADVANCEMENT_OUSTERS_MAGIC-ADVANCEMENT_ACTION_START][2].GetSize();
	
			if( bFirstAction )
				action = ACTION_ADVANCEMENT_OUSTERS_MAGIC;
			else
				action = ACTION_ADVANCEMENT_OUSTERS_WRISTLET_MOVE;
		}
		else
		{
			bool bFirstAction = index < m_AdvancementOustersCfpk[coat][ACTION_ADVANCEMENT_OUSTERS_SKILL_FAST-ADVANCEMENT_ACTION_START][2].GetSize();

			if( bFirstAction )
				action = ACTION_ADVANCEMENT_OUSTERS_SKILL_FAST;
			else
				action = ACTION_ADVANCEMENT_OUSTERS_CHAKRAM_MOVE;
		}
	}
	else
	{
		if(weapon == 0)	// 챈째혥챈혨혫챔쨉쨈 챕혷혨책혰혻책짬혺
		{
			action = ACTION_ADVANCEMENT_OUSTERS_WRISTLET_STOP;
		}
		else
			action = ACTION_ADVANCEMENT_OUSTERS_CHAKRAM_STOP;
	}

	action-=ADVANCEMENT_ACTION_START;
	index %= m_AdvancementOustersCfpk[coat][action][2].GetSize();
	
	
	int coat_color = p_slot->coat_color;
	if(coat_color == QUEST_ITEM_COLOR )
		coat_color = MItem::GetQuestItemColorset();
	else
	{
		if(coat_color == UNIQUE_ITEM_COLOR)
			coat_color = MItem::GetUniqueItemColorset();
	}
	
	int weapon_color = p_slot->skin_color;
	if(weapon_color == QUEST_ITEM_COLOR )
		weapon_color = MItem::GetQuestItemColorset();
	else
	{
		if(weapon_color == UNIQUE_ITEM_COLOR)
			weapon_color = MItem::GetUniqueItemColorset();
	}
	
	int hair_color = p_slot->hair_color;
	if(hair_color == QUEST_ITEM_COLOR )
		hair_color = MItem::GetQuestItemColorset();
	else
	{
		if(hair_color == UNIQUE_ITEM_COLOR)
			hair_color = MItem::GetUniqueItemColorset();
	}
	
	int trouser_color = p_slot->trouser_color;
	
	if(trouser_color == QUEST_ITEM_COLOR )
		trouser_color = MItem::GetQuestItemColorset();
	else
	{
		if(trouser_color == UNIQUE_ITEM_COLOR)
			trouser_color = MItem::GetUniqueItemColorset();
	}
	
	
	CIndexSprite::SetUsingColorSet(hair_color, coat_color); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
	if(weapon)
		CIndexSprite::SetUsingColorSetOnly(2, 377);
	else
		CIndexSprite::SetUsingColorSetOnly(2, weapon_color);
	CIndexSprite::SetUsingColorSetOnly(3, trouser_color);
	
	point.x = _x+m_AdvancementOustersCfpk[coat][action][2][index].GetCX();
	point.y = _y+m_AdvancementOustersCfpk[coat][action][2][index].GetCY();
	
	const int sprite_id = m_AdvancementOustersCfpk[coat][action][2][index].GetSpriteID();
	
	if(dark)
		gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &m_AdvancementOustersIspk[sprite_id], DARK_BIT);
	else
		gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &m_AdvancementOustersIspk[sprite_id]);
	
	if(weapon)	// ì±ì§¹í»ì± íì§­ì±í·í£ ì±ì§¸í¥ì±ì¨ì§¸
	{
		index %= m_AdvancementOustersCfpk[0][action][2].GetSize();
		
		point.x = _x+m_AdvancementOustersCfpk[0][action][2][index].GetCX();
		point.y = _y+m_AdvancementOustersCfpk[0][action][2][index].GetCY();
		
		const int sprite_id = m_AdvancementOustersCfpk[0][action][2][index].GetSpriteID();
		
		CIndexSprite::SetUsingColorSet(weapon_color, weapon_color); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
		if(dark)
			gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &m_AdvancementOustersIspk[sprite_id], DARK_BIT);
		else
			gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &m_AdvancementOustersIspk[sprite_id]);
	}
}

void	C_VS_UI_NEWCHAR::_ShowCharacterACVampire( int _x, int _y, S_SLOT* p_slot, int index, int enable, int dark )
{
	const int DARK_BIT = 1;
	POINT point;
	int coat = 0;//p_slot->man_info.coat;
	int weapon = p_slot->man_info.right;

	CCreatureFramePack* pCFPK = NULL;
	CIndexSpritePack* pISPK = NULL;
	
	if( p_slot->bl_female )
	{
		pCFPK = &m_AdvancementVampireWomanCfpk;
		pISPK = &m_AdvancementVampireWomanIspk;
	}
	else
	{
		pCFPK = &m_AdvancementVampireManCfpk;
		pISPK = &m_AdvancementVampireManIspk;
	}
	
	int action = 0;

	if(enable)	// 莽聯炉茅聙垄猫碌麓
	{
		if(weapon == 0)	// 챈째혥챈혨혫챔쨉쨈 챕혷혨책혰혻책짬혺
		{
			bool bFirstAction = index < (*pCFPK)[coat][ACTION_ADVANCEMENT_SPECIAL-ADVANCEMENT_ACTION_START][2].GetSize();
	
			if( bFirstAction )
				action = ACTION_ADVANCEMENT_SPECIAL;
			else
				action = ACTION_ADVANCEMENT_MOVE;
		}
		else
		{
			bool bFirstAction = index < (*pCFPK)[coat][ACTION_ADVANCEMENT_SKILL_FAST-ADVANCEMENT_ACTION_START][2].GetSize();

			if( bFirstAction )
				action = ACTION_ADVANCEMENT_SKILL_FAST;
			else
				action = ACTION_ADVANCEMENT_MOVE;
		}
	}
	else
	{
		if(weapon == 0)	// 챈째혥챈혨혫챔쨉쨈 챕혷혨책혰혻책짬혺
		{
			action = ACTION_ADVANCEMENT_STOP;
		}
		else
			action = ACTION_ADVANCEMENT_STOP;
	}

	action-=ADVANCEMENT_ACTION_START;
	index %= (*pCFPK)[coat][action][2].GetSize();
		
	int coat_color = p_slot->coat_color;
	if(coat_color == QUEST_ITEM_COLOR )
		coat_color = MItem::GetQuestItemColorset();
	else
	{
		if(coat_color == UNIQUE_ITEM_COLOR)
			coat_color = MItem::GetUniqueItemColorset();
	}
		
	point.x = _x+(*pCFPK)[coat][action][2][index].GetCX();
	point.y = _y+(*pCFPK)[coat][action][2][index].GetCY();
	
	const int sprite_id = (*pCFPK)[coat][action][2][index].GetSpriteID();
	
	if(dark)
		gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &(*pISPK)[sprite_id], DARK_BIT);
	else
		gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &(*pISPK)[sprite_id]);
	
	if(weapon)	// ì±ì§¹í»ì± íì§­ì±í·í£ ì±ì§¸í¥ì±ì¨ì§¸
	{
		index %= (*pCFPK)[1][action][2].GetSize();
		
		point.x = _x+(*pCFPK)[1][action][2][index].GetCX();
		point.y = _y+(*pCFPK)[1][action][2][index].GetCY();
		
		const int sprite_id = (*pCFPK)[1][action][2][index].GetSpriteID();
		
		if(dark)
			gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness(&point, &(*pISPK)[sprite_id], DARK_BIT);
		else
			gpC_base->m_p_DDSurface_back->BltIndexSprite(&point, &(*pISPK)[sprite_id]);
	}
}

void	C_VS_UI_NEWCHAR::_ShowCharacterACSlayer( int _x, int _y, S_SLOT* p_slot, int index, int enable, int dark )
{
	const int DARK_BIT = 1;
	POINT point;

	MAN_INFO *info;
	if(p_slot->bl_female)
		info = (MAN_INFO *)&p_slot->woman_info;
	else
		info = &p_slot->man_info;
	
	// éè·¯èµæ¯èæ¥¼ éè¨èµéè²èµéèè¦ æ¯è­èµæ¯è¥é æ¯æ³è°çæèçèè£çè¹å½æ¯éè¹ çèéçè½è².. - -; by sigi
	int item_color = p_slot->trouser_color;
	if(item_color == QUEST_ITEM_COLOR )
		item_color = MItem::GetQuestItemColorset();
	else
	{
		if(item_color == UNIQUE_ITEM_COLOR)
			item_color = MItem::GetUniqueItemColorset();
	}	
	CIndexSprite::SetUsingColorSet(p_slot->skin_color, item_color); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².

	CCreatureFramePack& slayerFPK = !p_slot->bl_female ? m_AdvancementSlayerManCfpk : m_AdvancementSlayerWomanCfpk;
	CIndexSpritePack& addonISPK = !p_slot->bl_female ? m_AdvancementSlayerManIspk : m_AdvancementSlayerWomanIspk;

	int action = 0;
	if(enable)	// 莽聯炉茅聙垄猫碌麓
	{
		action = GetAdvancementSlayerMoveActionFromType( info->right );
	}
	else
	{
		action = GetAdvancementSlayerStandActionFromType( info->right );
	}

	action -= ADVANCEMENT_ACTION_START;

	for( int i = 0; i < AC_ADDON_MAX; ++i )
	{
		int clothes = g_ACAddonOrder[ 2 ][ i ];
		if( clothes == AC_ADDON_MOTOR )
			continue;

		switch( clothes )
		{
		case AC_ADDON_BODY :
			item_color=p_slot->coat_color;
			if(item_color == QUEST_ITEM_COLOR )
				item_color = MItem::GetQuestItemColorset();
			else
			{
				if(item_color == UNIQUE_ITEM_COLOR)
					item_color = MItem::GetUniqueItemColorset();
			}
			CIndexSprite::SetUsingColorSet(p_slot->skin_color, item_color); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
			clothes = AC_BODY;
			break;

		case AC_ADDON_HELM :
			if( info->helmet == M_NO_WEAR )
				continue;

			item_color=p_slot->helmet_color;
			if(item_color == QUEST_ITEM_COLOR)
				item_color = MItem::GetQuestItemColorset();
			else
				if(item_color == UNIQUE_ITEM_COLOR)
					item_color = MItem::GetUniqueItemColorset();
			CIndexSprite::SetUsingColorSet(item_color, 0); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².
			clothes = AC_HELMET;
			break;

		case AC_ADDON_LEFTHAND :
			if( info->left == M_NO_WEAR )
				continue;
		case AC_ADDON_RIGHTHAND :
			if( clothes == AC_ADDON_RIGHTHAND && info->right == M_NO_WEAR )
				continue;

			if( clothes == AC_ADDON_RIGHTHAND )
			{
				item_color=p_slot->right_color;
				clothes = GetWeaponPartFromType( info->right );
				if( clothes == -1 )
					continue;
			}
			else 
			{
				item_color=p_slot->left_color;
				clothes = AC_SHIELD;
			}

			if(item_color == QUEST_ITEM_COLOR )				
				item_color = MItem::GetQuestItemColorset();
			else
				if(item_color == UNIQUE_ITEM_COLOR)
					item_color = MItem::GetUniqueItemColorset();
				
			CIndexSprite::SetUsingColorSet(item_color, 0); // bltçè¶è éè²è¹çè¦è§çèºè çèºéºæ¯éèçè²å½ éè²ç¯. çé²è¹æ¯è½æ¥¼çæèçè½èçè´è¬ çèéçè½è².

			break;

		case AC_ADDON_SHOULDER :
			clothes = AC_SHOULDER;
			break;
		}
		try{
		//by viva
		if(clothes>slayerFPK.GetSize()-1) continue;
		if(action>slayerFPK[clothes].GetSize()-1) continue;

		index %= slayerFPK[ clothes ][ action ][ 2 ].GetSize();

		int sprite_id = slayerFPK[ clothes ][ action ][ 2 ][ index ].GetSpriteID();
		point.x = _x + slayerFPK[ clothes ][ action ][ 2 ][ index ].GetCX();
		point.y = _y + slayerFPK[ clothes ][ action ][ 2 ][ index ].GetCY();

		if( dark )
			gpC_base->m_p_DDSurface_back->BltIndexSpriteDarkness( &point, &addonISPK[ sprite_id ], DARK_BIT );
		else
			gpC_base->m_p_DDSurface_back->BltIndexSprite( &point, &addonISPK[ sprite_id ] );
		}catch(...){}//by viva
	}
}
