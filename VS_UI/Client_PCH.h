////////////////////////////////////////////////////////////////////////////////
//	created:	2004/12/22
//	file base:	client_pch.h
//
//	Modified for cross-platform support (macOS/Linux)
////////////////////////////////////////////////////////////////////////////////

#pragma once

/* Establish PLATFORM_WINDOWS before the #ifdef checks below run: whenever
   this PCH is the first header included in a translation unit, the macro
   is still undefined here (only basic/Platform.h defines it), so the
   POSIX branches (<unistd.h>, <SDL2/SDL.h>, ...) would get taken even on
   a native Windows build. */
#if defined(_WIN32) || defined(_WIN64)
	#ifndef PLATFORM_WINDOWS
		#define PLATFORM_WINDOWS
	#endif
#endif

#ifdef PLATFORM_WINDOWS
	#pragma warning(disable:4290)
	#pragma warning(disable:4018)
	#pragma warning(disable:4244)
	#pragma warning(disable:4018)
	#pragma warning(disable:4786)
	#pragma warning(push)
#endif

#include <string>
#include <cassert>
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdarg>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef PLATFORM_WINDOWS
	/* Do not include the real <MMSystem.h>/<Digitalv.h>/<DDraw.h> here:
	   this project no longer uses real DirectX/DirectSound - basic/AudioTypes.h
	   and Client/DXLib/CDirectDraw.h define SDL-backed stand-ins for the
	   same type names (HMMIO, DDPIXELFORMAT, LPDIRECTDRAW7, ...), and
	   including the real headers redefines them with incompatible types.
	   WIN32_LEAN_AND_MEAN keeps <windows.h> itself from pulling those same
	   headers in automatically (via lzexpand.h -> mmsystem.h -> ...). */
	#ifndef _WINDOWS_
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#endif
	#include <io.h>
	#include <fcntl.h>
	#pragma warning(pop)
#else
	/* Use platform abstraction layer */
	#include "../basic/Platform.h"
	#include <SDL2/SDL.h>
	#include <unistd.h>
#endif

//#include "GAME1024.h"

using std::string;
using std::vector;
using std::map;
using std::list;
using std::deque;
using std::bitset;

extern BOOL g_MyFull;
extern RECT g_GameRect;
//extern GAME1024 g_NewMode;
extern	LONG g_SECTOR_WIDTH;
extern	LONG g_SECTOR_HEIGHT;
extern	LONG g_SECTOR_WIDTH_HALF;
extern	LONG g_SECTOR_HEIGHT_HALF;
extern	LONG g_SECTOR_SKIP_PLAYER_LEFT;
extern	LONG g_SECTOR_SKIP_PLAYER_UP;

extern	LONG g_TILESURFACE_SECTOR_WIDTH;
extern	LONG g_TILESURFACE_SECTOR_HEIGHT;
extern	LONG g_TILESURFACE_SECTOR_OUTLINE_RIGHT;
extern	LONG g_TILESURFACE_SECTOR_OUTLINE_DOWN;
extern	LONG g_TILESURFACE_WIDTH;
extern	LONG g_TILESURFACE_HEIGHT;
extern	LONG g_TILESURFACE_OUTLINE_RIGHT;
extern	LONG g_TILESURFACE_OUTLINE_DOWN;
extern	LONG g_TILE_X_HALF;
extern	LONG g_TILE_Y_HALF;
