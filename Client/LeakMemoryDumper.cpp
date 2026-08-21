//////////////////////////////////////////////////////////////////////////////
/// \file LeakMemoryDumper.cpp
/// \author sonee
/// \date 2004.12.30
//////////////////////////////////////////////////////////////////////////////

#include "Client_PCH.h"
#include "LeakMemoryDumper.h"

#ifdef _DEBUG

typedef struct {
	uintptr_t	address;
	size_t	size;
	char	file[64];
	DWORD	line;
} ALLOC_INFO;

typedef std::list<ALLOC_INFO*> AllocList;
AllocList allocList;

void AddTrack(uintptr_t addr, size_t asize, const char *fname, DWORD lnum)
{
	ALLOC_INFO *info;
	// Must bypass the tracked new(__FILE__, __LINE__) here (see
	// LeakMemoryDumper.h's #define new DEBUG_NEW) - `new (ALLOC_INFO)`
	// would macro-expand to that same tracked placement new, which calls
	// AddTrack() again to track *this* allocation, infinitely recursing
	// (unconditional stack overflow the first time anything anywhere
	// calls `new`). Allocate the bookkeeping node with a plain malloc()
	// instead.
	info = (ALLOC_INFO*)malloc(sizeof(ALLOC_INFO));
	info->address = addr;
	strncpy(info->file, fname, 63);
	info->line = lnum;
	info->size = asize;
	allocList.insert(allocList.begin(), info);
}

void RemoveTrack(uintptr_t addr)
{
	AllocList::iterator i;

	for(i = allocList.begin(); i != allocList.end(); i++)
	{
		if((*i)->address == addr)
		{
			allocList.remove((*i));
			break;
		}
	}
}

void DumpUnfreed()
{
	AllocList::iterator i;
	size_t totalSize = 0;
	char buf[1024];

	for(i = allocList.begin(); i != allocList.end(); i++)
	{
		sprintf(buf, "%-50s:\t\tLINE %d,\t\tADDRESS 0x%p\t%zu unfreed\n",
			(*i)->file, (*i)->line, (void*)(*i)->address, (*i)->size);
		OutputDebugString(buf);
		totalSize += (*i)->size;
	}
	sprintf(buf, "-----------------------------------------------------------\n");
	OutputDebugString(buf);
	sprintf(buf, "Total Unfreed: %zu bytes\n", totalSize);
	OutputDebugString(buf);
}

#endif