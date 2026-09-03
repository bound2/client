//----------------------------------------------------------------------
// MSortedItemManager.h
//----------------------------------------------------------------------

#ifndef __MSORTEDITEMMANAGER_H__
#define __MSORTEDITEMMANAGER_H__

#pragma warning(disable:4786)

#include <map>

class MItem;

class MSortedItemManager : public std::map<ULONGLONG, MItem*> {
	public :
		typedef std::map<ULONGLONG, MItem*>		ITEM_MAP;
		typedef ITEM_MAP::iterator				iterator;
		typedef ITEM_MAP::const_iterator		const_iterator;

	public :
		MSortedItemManager();
		~MSortedItemManager();

		void		Release();						// delete every item and empty the map
		void		Clear()		{ clear(); }		// empty the map, keep the items

		//----------------------------------------------------------
		// Add an item; false when its key is already held.
		//----------------------------------------------------------
		bool		AddItem(MItem* pItem);

	protected :
		ULONGLONG	GetKey(MItem* pItem);
};

#endif


