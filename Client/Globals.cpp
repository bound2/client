//-----------------------------------------------------------------------------
// Globals.cpp
//-----------------------------------------------------------------------------
// Global variables and utility functions needed by the game
// These came out of a duplicate of PacketFunction.cpp that the build
// excluded and task 5.2 deleted; this file is the one that compiles.
//-----------------------------------------------------------------------------

#include "Client_PCH.h"
#include "VS_UI/src/hangul/Fl2.h"  // For PrintInfo definition

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

// Wave pack file manager
class CWavePackFileManager;
CWavePackFileManager* g_pWavePackFileManager = NULL;

// Info show globals
extern "C" {
	bool gbl_info_show = true;
	bool gbl_show_item = true;
}

//-----------------------------------------------------------------------------
// Print Functions
//-----------------------------------------------------------------------------
// Implemented in RenderingFunctions.cpp for SDL builds.

//-----------------------------------------------------------------------------
// Platform-Specific Functions (Windows-specific)
//-----------------------------------------------------------------------------

// CheckMacScreenMode - PacketFunction.cpp already provides a real
// implementation under #ifdef PLATFORM_WINDOWS; this stub is only needed
// as a fallback on the platforms where that one is excluded. Defining both
// on Windows was a duplicate symbol (LNK2005).
#ifndef PLATFORM_WINDOWS
void CheckMacScreenMode()
{
	// Stub implementation - Windows-specific function
}
#endif

// GetNMClipData - Get Netmarble clipboard data (Windows-specific)
bool GetNMClipData(char* pBuffer, unsigned int bufferSize, const char* pURL, bool bUseHTML)
{
	(void)pBuffer; (void)bufferSize; (void)pURL; (void)bUseHTML;
	// Stub implementation - Windows-specific function
	return false;
}

// SendUserIDToGameMonA - Send user ID to GameMon anti-cheat (Windows-specific)
void SendUserIDToGameMonA(const char* pUserID)
{
	(void)pUserID;
	// Stub implementation - Windows GameMon anti-cheat not available on macOS
}
