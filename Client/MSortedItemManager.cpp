//----------------------------------------------------------------------
// MSortedItemManager.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "MSortedItemManager.h"
#include "MItem.h"

//----------------------------------------------------------------------
//
// constructor / destructor
//
//----------------------------------------------------------------------
MSortedItemManager::MSortedItemManager()
{
}

MSortedItemManager::~MSortedItemManager()
{
	Release();
}

//----------------------------------------------------------------------
//
// member functions
//
//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Release
//----------------------------------------------------------------------
// Delete every item and empty the map.
//----------------------------------------------------------------------
void		
MSortedItemManager::Release()
{
	ITEM_MAP::iterator iItem = begin();

	while (iItem != end())
	{
		MItem* pItem = iItem->second;

		if (pItem!=NULL)
		{
			delete pItem;
		}

		iItem++;
	}

	clear();
}

//----------------------------------------------------------------------
// Add Item
//----------------------------------------------------------------------
bool		
MSortedItemManager::AddItem(MItem* pItem)
{
	ULONGLONG key = GetKey( pItem );

	// Not held yet: take it.
	if (!contains( key ))
	{
		insert( ITEM_MAP::value_type( key, pItem ) );

		return true;
	}

	return false;
}

//----------------------------------------------------------------------
// Get Key
//----------------------------------------------------------------------
// Eight bytes, high to low: 0xFF minus the item's grid area (so the
// biggest footprint sorts first), then the item's object id (so equal
// footprints sort by id).
//----------------------------------------------------------------------
ULONGLONG
MSortedItemManager::GetKey(MItem* pItem)
{
	TYPE_OBJECTID itemObjectID= pItem->GetID();

	int gridWidth		= pItem->GetGridWidth();
	int gridHeight		= pItem->GetGridHeight();
	int gridSize		= 0xFF - gridWidth*gridHeight;

	ULONGLONG key = gridSize;

	key = (key << 32) | itemObjectID;

	return key;
}