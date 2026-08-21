//////////////////////////////////////////////////////////////////////////////
/// \file LeakMemoryDumper.h
/// \author sonee
/// \date 2004.12.30
//////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _DEBUG

void AddTrack(DWORD addr,  DWORD asize,  const char *fname, DWORD lnum);
void DumpUnfreed();
void RemoveTrack(DWORD addr);

__inline void * __cdecl operator new(size_t size,
								   const char *file, int line)
{
	void *ptr = (void *)malloc(size);
	AddTrack((DWORD)(uintptr_t)ptr, (DWORD)size, file, line);
	return(ptr);
}

__inline void __cdecl operator delete(void *p)
{
	RemoveTrack((DWORD)(uintptr_t)p);
	free(p);
}

#endif

#ifdef _DEBUG
	#define DEBUG_NEW new(__FILE__, __LINE__)
#else
	#define DEBUG_NEW new
#endif

#define new DEBUG_NEW

#pragma warning(disable:4291)