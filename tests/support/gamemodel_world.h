//----------------------------------------------------------------------
// gamemodel_world.h - the globals every gamemodel test reads
//----------------------------------------------------------------------
//
// The item core reads six globals the executable creates at start-up:
// the item and option tables, the game string table, the user's
// settings, the client configuration and the timed-item register.
// GameModelWorld creates them empty, resets MItem's host, and tears
// everything down in reverse; a test file's own fixture derives from
// it and fills what its tests read, so no test sees another's tables.
//
//----------------------------------------------------------------------

#ifndef __GAMEMODEL_WORLD_H__
#define __GAMEMODEL_WORLD_H__

#include "MItem.h"				// first: it carries the platform types the other headers use
#include "MItemTable.h"
#include "MItemOptionTable.h"
#include "MGameStringTable.h"
#include "MStringArray.h"
#include "UserInformation.h"
#include "ClientConfig.h"
#include "MTimeItemManager.h"
#include "ItemClassDef.h"

struct GameModelWorld
{
	// optionRows sizes the option table; row 0 is the "no option" row.
	explicit GameModelWorld(int optionRows = 1)
	{
		g_pItemTable = new ITEMCLASS_TABLE;
		g_pItemTable->Init(MAX_ITEM_CLASS);
		g_pItemOptionTable = new ITEMOPTION_TABLE;
		g_pItemOptionTable->Init(optionRows);
		g_pGameStringTable = new MStringArray;
		g_pUserInformation = new UserInformation;
		g_pClientConfig = new ClientConfig;
		// The executable creates the register at start-up, before any item.
		g_pTimeItemManager = new MTimeItemManager;
		MItem::SetHost(NULL);
	}

	~GameModelWorld()
	{
		MItem::SetHost(NULL);
		delete g_pTimeItemManager;	g_pTimeItemManager = NULL;
		delete g_pClientConfig;		g_pClientConfig = NULL;
		delete g_pUserInformation;	g_pUserInformation = NULL;
		delete g_pGameStringTable;	g_pGameStringTable = NULL;
		delete g_pItemOptionTable;	g_pItemOptionTable = NULL;
		delete g_pItemTable;		g_pItemTable = NULL;
	}

private:
	// Copying one would delete the six globals twice.
	GameModelWorld(const GameModelWorld&);
	void	operator=(const GameModelWorld&);
};

#endif
