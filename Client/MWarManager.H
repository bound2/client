//-----------------------------------------------------------------------------------------------
// MWarManager
//  Date : 2003.2
//  
//  by sonee
// 
//  çè½èçè¼è çè½è²æ¯é²éºæ¯æ¥¼å½ éæ³èçæèéé²è½ çè»è¢æ¯è¤è°æ¯è¥é.
//  çè½èçè¼è çè½è²æ¯é²éºçè´è¬ æ¯è°æ³æ¯èºå½ UI çè´è¬çèè¹ éè¶è²çèºèµ éè²è½ çè¢èµ çè»è¢æ¯è«èæ¯éèº Push éè²éºçéèæ¯è¥é.
//-----------------------------------------------------------------------------------------------

#ifndef __MWARMANAGER_H__
#define __MWARMANAGER_H__

//#include "types.h"
#include "WarInfo.h"

#include <map>
#include <list>
#include <vector>

enum WAR_TYPE_ID // 챕혷혦책혻짢챈혬짖
{
	WAR_TYPE_HOLYLAND,					// çè²èæ¯è¥éºçèºèµ çèå¤çæè
	WAR_TYPE_GUILD,						// ì±ì¨ì¨ì±í«íµì±í»í 
};

typedef			std::map<ZoneID_t, WarInfo*>			WarInfoMap;
typedef			WarInfoMap::iterator			WarInfoMapItr;


class MWarManager
{
private :

// 	list<ZoneID_t>		HolyLandZone;							// çè²èæ¯è¥éºçèºèµ çèå¤çæè éè²èæ¯è¯è¹ çééº ID List	
	WarInfoMap			m_WarInfo;								// 챕혷혶책혢쨀챌혙쨌 챕혷혶챈혴짚챌혗혪 Map<ZoneID>

public :	
	MWarManager();
	~MWarManager();	

	//-------------------------------------------------------------------
	// SetData
	//-------------------------------------------------------------------
	void			SetWar(WarInfo *info);						// WarList Add
	void			RemoveWar(ZoneID_t id);						// WarList Remove
	
	void			ClearWar();									// Clear
	void			ClearRaceWar();

	//-------------------------------------------------------------------
	// Get Data
	//-------------------------------------------------------------------
	WarInfo*						GetWarInfo(ZoneID_t id);
	const WarInfoMap&				getWarInfoList() { return m_WarInfo; }
	int								getSize() { return m_WarInfo.size(); }
	
	//-------------------------------------------------------------------
	// Check
	//-------------------------------------------------------------------
	bool			IsExist(ZoneID_t id);						// çééºçè²èçèºéºæ¯è°è°éæ³è çè½èçè¼èçè´è¬ éè«å¢éè²ç¯æ¯è¬èµçè³éºçè»è¢æ¯è¤è°éæ³è
// 	bool			IsHolyLand(ZoneID_t id);					// çèå¤çèºéº çè²èæ¯è¥è¢æ¯èºå½ çè²èæ¯è¥éºçèºèµ çèå¤çæè éè²èæ¯è¯è¹çèºèµéæ³è
	
	void			Update();	
};

extern MWarManager	*g_pWarManager;

#endif
