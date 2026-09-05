//----------------------------------------------------------------------
// ProfileManager.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#ifdef PLATFORM_WINDOWS
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/dir.h>
#endif
#include "CDirectDraw.h"
#include "CSpritePack.h"
#include "UserInformation.h"
//#include "MFileDef.h"

#ifdef __GAME_CLIENT__
	#include "Properties.h"
#else
	#include "../Client/Packet/Properties.h"
#endif

#include "UtilityFunction.h"
#include "ProfileManager.h"

// std::filesystem directory enumeration, in place of the _findfirst /
// _findnext walks these two functions used to run
// (docs/cpp17-cpp20-compatibility-assessment-2026-09-04.md, priority 6).
#include "DirectoryListing.h"

#include <string>
#include <vector>

#ifdef __GAME_CLIENT__
	#include "RequestUserManager.h"

	#include "Packet/Cpackets/CGRequestIP.h"

	#include "ServerInfo.h"
	#include "RequestClientPlayerManager.h"
#endif

#include "DebugInfo.h"

//----------------------------------------------------------------------
// Global
//----------------------------------------------------------------------
ProfileManager*		g_pProfileManager = NULL;

#define	PROFILE_NULL	"NULL"

//----------------------------------------------------------------------
//
// constructor / destructor
//
//----------------------------------------------------------------------
ProfileManager::ProfileManager()
{
	InitializeCriticalSection(&m_Lock);
}

ProfileManager::~ProfileManager()
{
	Release();

	DeleteCriticalSection(&m_Lock);
}

//----------------------------------------------------------------------
// Release
//----------------------------------------------------------------------
void
ProfileManager::Release()
{
	m_Profiles.clear();

	ReleaseRequire();
}

//----------------------------------------------------------------------
// Release Require
//----------------------------------------------------------------------
void
ProfileManager::ReleaseRequire()
{
	// Debug output before locking
	DEBUG_ADD("[ProfileManager] ReleaseRequire: attempting to lock...\n");

	Lock();

	DEBUG_ADD("[ProfileManager] ReleaseRequire: lock acquired, clearing...\n");

	m_Requires.clear();

	DEBUG_ADD("[ProfileManager] ReleaseRequire: done, releasing lock...\n");

	Unlock();
}

//----------------------------------------------------------------------
// Has Profile
//----------------------------------------------------------------------
bool
ProfileManager::HasProfile(const char* pName) const
{
	PROFILE_MAP::const_iterator iProfile = m_Profiles.find( std::string(pName) );

	if (iProfile!=m_Profiles.end())
	{
		if (iProfile->second==PROFILE_NULL)
		{
			return false;
		}

		return true;
	}

	return false;
}

//----------------------------------------------------------------------
// Add Profile
//----------------------------------------------------------------------
// 이미 있거나 말거나 관계없다.
//----------------------------------------------------------------------
void
ProfileManager::AddProfile(const char* pName, const char* pFilename)
{
	m_Profiles[std::string(pName)] = std::string(pFilename);
}

//----------------------------------------------------------------------
// Add ProfileNULL
//----------------------------------------------------------------------
// 상대방이 아예 Profile이 없는 경우
//----------------------------------------------------------------------
void
ProfileManager::AddProfileNULL(const char* pName)
{
	m_Profiles[std::string(pName)] = PROFILE_NULL;
}

//----------------------------------------------------------------------
// Has ProfileNULL
//----------------------------------------------------------------------
// 상대방이 아예 Profile이 없는가?
//----------------------------------------------------------------------
bool			
ProfileManager::HasProfileNULL(const char* pName) const
{
	PROFILE_MAP::const_iterator iProfile = m_Profiles.find( std::string(pName) );

	if (iProfile!=m_Profiles.end())
	{
		if (iProfile->second==PROFILE_NULL)
		{
			return true;
		}
	}

	return false;	
}

//----------------------------------------------------------------------
// Remove Profile
//----------------------------------------------------------------------
bool
ProfileManager::RemoveProfile(const char* pName)
{
	PROFILE_MAP::iterator iProfile = m_Profiles.find( std::string(pName) );

	if (iProfile!=m_Profiles.end())
	{
		m_Profiles.erase( iProfile );

		return true;
	}

	return false;
}

//----------------------------------------------------------------------
// Get UserInfo
//----------------------------------------------------------------------
const char*
ProfileManager::GetFilename(const char* pName) const
{
	PROFILE_MAP::const_iterator iProfile = m_Profiles.find( std::string(pName) );

	if (iProfile!=m_Profiles.end())
	{
		// NULL로 설정된 거는 아예 Profile이 없는 경우이다.
		if (iProfile->second==PROFILE_NULL)
		{
			return NULL;
		}

		return iProfile->second.c_str();
	}

	return NULL;
}

//----------------------------------------------------------------------
// RequestProfile
//----------------------------------------------------------------------
void			
ProfileManager::RequestProfile(const char* pName)
{
	if(pName == NULL)
		return;

	if (!HasRequire(pName))
	{
		AddRequire( pName );
	}

	/*
	// Update에서 처리한다.
#ifdef __GAME_CLIENT__
	//-------------------------------------------------------
	// 접속중이거나 접속 시도 중인 경우..
	//-------------------------------------------------------
	if (g_pRequestClientPlayerManager->HasConnection(pName)
		|| g_pRequestClientPlayerManager->HasTryingConnection(pName))
	{
	}
	//-------------------------------------------------------
	// 접속중이 아닌 경우
	//-------------------------------------------------------
	else
	{
		RequestUserInfo* pUserInfo = g_pRequestUserManager->GetUserInfo(pName);

		//-------------------------------------------------------
		// 사용자 정보가 있다면 접속 시도를 한다.
		//-------------------------------------------------------
		if (pUserInfo!=NULL)
		{
			g_pRequestClientPlayerManager->Connect(pUserInfo->IP.c_str(), 
													pName, 
													REQUEST_CLIENT_MODE_PROFILE);
		}
		//-------------------------------------------------------
		// 사용자 정보가 없다면 ... 서버에 IP를 요청한다.
		//-------------------------------------------------------
		else
		{
			if (!g_pRequestUserManager->HasRequestingUser( pName ))
			{
				#ifdef CONNECT_SERVER
					// 서버에 IP를 요청한다.
					CGRequestIP _CGRequestIP;
					_CGRequestIP.setName( pName );

					g_pSocket->sendPacket( &_CGRequestIP );			
				#endif
	
				// 요청해두면 IP를 받을 때, ProfileManager를 체크하게 된다.
				g_pRequestUserManager->AddRequestingUser( pName, RequestUserManager::REQUESTING_FOR_PROFILE );
			}
		}
	}	
#endif
	*/
}

//----------------------------------------------------------------------
// Add Require
//----------------------------------------------------------------------
void			
ProfileManager::AddRequire(const char* pName)
{
	Lock();

	if (!HasProfile(pName)
		&& !HasProfileNULL(pName))
	{
		m_Requires[std::string(pName)] = 0;
	}

	Unlock();
}

//----------------------------------------------------------------------
// Has Require
//----------------------------------------------------------------------
bool			
ProfileManager::HasRequire(const char* pName) const
{
	if (m_Requires.find( std::string(pName) )==m_Requires.end())
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------
// Remove Require
//----------------------------------------------------------------------
bool			
ProfileManager::RemoveRequire(const char* pName)
{
	Lock();

	REQUIRE_MAP::iterator iRequire = m_Requires.find( std::string(pName) );

	if (iRequire==m_Requires.end())
	{
		Unlock();
		return false;
	}

	m_Requires.erase( iRequire );

	Unlock();
	return true;
}

//----------------------------------------------------------------------
// Update
//----------------------------------------------------------------------
void			
ProfileManager::Update()
{
#ifdef __GAME_CLIENT__
	if (g_pRequestClientPlayerManager==NULL 
		|| g_pRequestUserManager==NULL)
	{				
		m_Requires.clear();
		return;
	}
#endif

	Lock();

	//--------------------------------------------------------------
	// request
	//--------------------------------------------------------------
	REQUIRE_MAP::iterator iRequire = m_Requires.begin();

	while (iRequire != m_Requires.end())
	{
		const char* pName = iRequire->first.c_str();

		//-------------------------------------------------------
		// profile을 받은 경우
		//-------------------------------------------------------
		if (HasProfile(pName)
			|| HasProfileNULL(pName))
		{
			REQUIRE_MAP::iterator iTemp = iRequire;
			iRequire ++;
			m_Requires.erase( iTemp );
			
			continue;
		}

		#ifdef __GAME_CLIENT__
			//-------------------------------------------------------
			// 접속중이거나 접속 시도 중인 경우..
			//-------------------------------------------------------
			if (g_pRequestClientPlayerManager->HasConnection(pName)
				|| g_pRequestClientPlayerManager->HasTryingConnection(pName)
				|| g_pRequestUserManager->HasRequestingUser( pName ))
			{				
			}
			//-------------------------------------------------------
			// 접속중이 아닌 경우
			//-------------------------------------------------------
			else
			{
				RequestUserInfo* pUserInfo = g_pRequestUserManager->GetUserInfo(pName);

				//-------------------------------------------------------
				// 사용자 정보가 있다면 접속 시도를 한다.
				//-------------------------------------------------------
				if (pUserInfo!=NULL)
				{
					if( g_pUserInformation->bKorean == true )	// 한국 버전만 p2p
					{
						g_pRequestClientPlayerManager->Connect(pUserInfo->IP.c_str(), 
																pName, 
																REQUEST_CLIENT_MODE_PROFILE);				
					}
				}
				//-------------------------------------------------------
				// 사용자 정보가 없다면 ... 서버에 IP를 요청한다.
				//-------------------------------------------------------
				else
				{
					if (!g_pRequestUserManager->HasRequestingUser( pName ))
					{
						if( g_pUserInformation->bKorean == true )
						{
							// 서버에 IP를 요청한다.
							CGRequestIP _CGRequestIP;
							_CGRequestIP.setName( pName );

							g_pSocket->sendPacket( &_CGRequestIP );			
			
						// 요청해두면 IP를 받을 때, ProfileManager를 체크하게 된다.
						g_pRequestUserManager->AddRequestingUser( pName, RequestUserManager::REQUESTING_FOR_PROFILE );
						}
					}
				}
			}	
		#endif

		iRequire ++;
	}

	Unlock();
}


//----------------------------------------------------------------------
// Init Profiles
//----------------------------------------------------------------------
// 프로그램이 실행될 때 한번 실행시켜주면 된다.
//
// Profile/*.bmp 를 읽어서 Profile/*.spr로 바꿔주면 된다.
//----------------------------------------------------------------------
void		
ProfileManager::InitProfiles()
{
	//-----------------------------------------------------------------
	// Profile Directory가 없으면 생성한다.
	//-----------------------------------------------------------------
	char CWD[_MAX_PATH];

	std::string sTest = g_pFileDef->getProperty("DIR_PROFILE");

	if (_getcwd( CWD, _MAX_PATH )!=NULL)
	{	
		if (_chdir( g_pFileDef->getProperty("DIR_PROFILE").c_str()) == 0)
		{
			// 있다면.. 다시 원래 DIR로..
			_chdir( CWD );
		}
		else
		{
			// DIR_PROFILE이 없다면.. 생성..
#ifdef PLATFORM_WINDOWS
			_mkdir( g_pFileDef->getProperty("DIR_PROFILE").c_str() );
#else
			mkdir( g_pFileDef->getProperty("DIR_PROFILE").c_str(), 0755 );
#endif
		}		
	}

	const std::string sProfileDir = g_pFileDef->getProperty("DIR_PROFILE");

	char bmpFilename[256];

	//-----------------------------------------------------------------
	// Every entry in the profile directory, which is what the legacy
	// "<dir>\*.*" _findfirst walk saw: on Win32 that pattern matches
	// every name - a name with no dot included - and subdirectories with
	// it, so the listing is asked for the same set. The only entries the
	// old walk produced that this one cannot are "." and "..", and both
	// are shorter than the eight characters the loop below requires.
	//-----------------------------------------------------------------
	std::vector<Basic::SDirectoryEntry>	vProfileFiles;

	if ( Basic::ListDirectory( sProfileDir.c_str(), "*", vProfileFiles,
			Basic::LIST_FILES_AND_DIRECTORIES ) )
	{
		CSpritePack SPK;

		// [0]은 작은거 (30, 38)
		// [1]은 큰거 (110, 139)
		SPK.Init( 2);

		// A CDirectDrawSurface-based branch used to run here on Windows,
		// with a CSpriteSurface `surface` used to Blt/Lock the loaded BMP
		// into SPK[]. With SPRITELIB_BACKEND_SDL (the only backend this
		// project builds, Windows included) CSpriteSurface no longer
		// inherits from CDirectDrawSurface and that branch never
		// type-checked; it's now a stub (see the notes further below), so
		// that intermediate surface isn't needed here any more either.
		const POINT bigSize = { 55, 70 };
		const POINT smallSize = { 30, 38 };

		// Note: SDL backend doesn't have InitOffsurface, surface will be created when needed

		for (size_t iFile=0; iFile<vProfileFiles.size(); iFile++)
		{
			const std::string&	sFilename = vProfileFiles[iFile].sName;

			//---------------------------------------------------------
			// _finddata_t::name capped a name at 259 bytes; a listing
			// entry carries no such cap, and the .spki path built below
			// is one byte longer than the path built here. An entry
			// that would not leave room for both is skipped rather than
			// written past the end of these 256-byte buffers.
			//---------------------------------------------------------
			if (sProfileDir.size() + 1 + sFilename.size() + 1 >= sizeof(bmpFilename))
			{
				continue;
			}

			sprintf(bmpFilename, "%s\\%s", sProfileDir.c_str(), sFilename.c_str());

			//---------------------------------------------------------
			// Read the bmp and turn it into a sprite.
			//---------------------------------------------------------
			char charName[256], spkFilename[256], spkiFilename[256];
			int lenFilename = (int)sFilename.size();

			// "name.bmp"
			if (lenFilename< 8)
			{
				continue;
			}

			strncpy( charName, sFilename.c_str(), lenFilename-4 );	// cut the .bmp off
			charName[lenFilename-4] = '\0';

			// CDirectDrawSurface-based loading used to run here on Windows,
			// but with SPRITELIB_BACKEND_SDL CSpriteSurface no longer
			// inherits from CDirectDrawSurface (LoadImageToSurface() has no
			// overload for it), so that branch never type-checked - see the
			// stub notes further below. Profile image loading is not yet
			// implemented on the SDL backend (Windows included); this is a
			// non-critical feature (profile character portraits).
			// TODO: Implement SDL_image based loading
			WORD* lpSurface;
			unsigned short pitch;

			// Create temporary surfaces for the profile
			RECT bmpRect = { 0, 0, smallSize.x, smallSize.y };
			RECT bmpRectBig = { 0, 0, bigSize.x, bigSize.y };

			// For now, just initialize empty sprites
			// The profile will load but without character portrait image
			lpSurface = new WORD[smallSize.x * smallSize.y];
			memset(lpSurface, 0, smallSize.x * smallSize.y * 2);
			pitch = smallSize.x * 2;
			SPK[0].SetPixelNoColorkey(lpSurface, pitch, smallSize.x, smallSize.y);
			delete[] lpSurface;

			lpSurface = new WORD[bigSize.x * bigSize.y];
			memset(lpSurface, 0, bigSize.x * bigSize.y * 2);
			pitch = bigSize.x * 2;
			SPK[1].SetPixelNoColorkey(lpSurface, pitch, bigSize.x, bigSize.y);
			delete[] lpSurface;

			// filename.spk
			int lenBmpFilename = strlen(bmpFilename);
			strncpy(spkFilename, bmpFilename, lenBmpFilename-3);
			spkFilename[lenBmpFilename-3] = '\0';
			strcat(spkFilename, "spk");

			// filename.spki
			strcpy(spkiFilename, spkFilename);
			strcat(spkiFilename, "i");

			std::ofstream	spkFile(spkFilename, ios::binary);
			std::ofstream	spkiFile(spkiFilename, ios::binary);
			SPK.SaveToFile( spkFile, spkiFile );
			spkFile.close();
			spkiFile.close();

			g_pProfileManager->AddProfile( charName, spkFilename );
		}
	}
}

//----------------------------------------------------------------------
// Delete Profiles
//----------------------------------------------------------------------
// 프로그램이 실행될 때 한번 실행시켜주면 된다.
//
// Profile/*.spr 화일을 모두 지우면 된다.
//----------------------------------------------------------------------
void		
ProfileManager::DeleteProfiles()
{
	const std::string sProfileDir = g_pFileDef->getProperty("DIR_PROFILE");

	char spkFilename[256];

	//-----------------------------------------------------------------
	// Both walks below list files only, where the legacy _findfirst
	// patterns also matched a subdirectory of the same name. remove()
	// cannot delete a directory on Windows, so the entries that are no
	// longer listed are exactly the ones the loop body could never have
	// acted on.
	//-----------------------------------------------------------------
	std::vector<Basic::SDirectoryEntry>	vProfileFiles;

	//-----------------------------------------------------------------
	// Find the *.spk files.
	//-----------------------------------------------------------------
	if ( Basic::ListDirectory( sProfileDir.c_str(), "*.spk*", vProfileFiles ) )
	{
		for (size_t iFile=0; iFile<vProfileFiles.size(); iFile++)
		{
			const std::string&	sFilename = vProfileFiles[iFile].sName;

			if (sProfileDir.size() + 1 + sFilename.size() >= sizeof(spkFilename))
			{
				continue;
			}

			sprintf(spkFilename, "%s\\%s", sProfileDir.c_str(), sFilename.c_str());
			remove(spkFilename);
		}
	}

	//-----------------------------------------------------------------
	// Find the *-spk temporary files.
	//-----------------------------------------------------------------
	if ( Basic::ListDirectory( sProfileDir.c_str(), "*-spk*", vProfileFiles ) )
	{
		for (size_t iFile=0; iFile<vProfileFiles.size(); iFile++)
		{
			const std::string&	sFilename = vProfileFiles[iFile].sName;

			if (sProfileDir.size() + 1 + sFilename.size() >= sizeof(spkFilename))
			{
				continue;
			}

			sprintf(spkFilename, "%s\\%s", sProfileDir.c_str(), sFilename.c_str());
			remove(spkFilename);
		}
	}
}

