//--------------------------------------------------------------------------
// WavePackFileManager.cpp
//--------------------------------------------------------------------------
#include "Client_PCH.h"
#include "WavePackFileManager.h"
// This file needs the real MMIO API (mmioOpen/mmioRead/...) to parse WAV
// files, unlike every other file that pulls in <MMSystem.h> just for
// timeGetTime()/GetTickCount(). basic/Platform.h has already #defined those
// two as platform_get_ticks() by this point (via Client_PCH.h), which
// corrupts <MMSystem.h>'s own timeGetTime() declaration (C2375) - undef it
// first so the real header parses cleanly. Safe because this file never
// calls timeGetTime()/GetTickCount() itself.
#undef timeGetTime
#undef GetTickCount
#include <MMSystem.h>
#include "CDirectSound.h"
#include "Profiler.h"

//--------------------------------------------------------------------------
// Global
//--------------------------------------------------------------------------
WavePackFileManager*	g_pWavePackFileManager = NULL;

//--------------------------------------------------------------------------
//
//					WavePackFileInfo
//
//--------------------------------------------------------------------------
bool		
WavePackFileInfo::SaveToFileData(std::ofstream& file)
{
	if (m_Filename.c_str()==NULL)
	{
		return false;
	}

	const int wavefmtSize = sizeof(WAVEFORMATEX);

	// open a wav file
	HMMIO wavefile;
	char filename[256];
	strcpy(filename, m_Filename.c_str());
	wavefile = mmioOpen(filename, 0, MMIO_READ|MMIO_ALLOCBUF);
	if(wavefile == NULL)
	{
		//DirectSoundFailed("Direct Sound mmioOpen Error!");		
		return false;
	}
	
	// find wave data
	MMCKINFO parent;
	memset(&parent, 0, sizeof(MMCKINFO));
	parent.fccType = mmioFOURCC('W','A','V','E');
	mmioDescend(wavefile, &parent, 0, MMIO_FINDRIFF);

	// find fmt data
	MMCKINFO child;
	memset(&child, 0, sizeof(MMCKINFO));
	child.fccType = mmioFOURCC('f','m','t',' ');
	mmioDescend(wavefile, &child, &parent,0);

	// read the format
	WAVEFORMATEX wavefmt;
	mmioRead(wavefile, (char*)&wavefmt, wavefmtSize);
	if(wavefmt.wFormatTag != WAVE_FORMAT_PCM)
	{
		return false;
	}

	// find the wave data chunk
	mmioAscend(wavefile, &child, 0);
	child.ckid = mmioFOURCC('d','a','t','a');
	mmioDescend(wavefile, &child, &parent, MMIO_FINDCHUNK);

	DWORD cksize = child.cksize;

	char* pBuffer = new char [cksize];
	mmioRead(wavefile, (char*)pBuffer, cksize);
	mmioClose(wavefile, 0);

	//--------------------------------------------------------------
	// file에 저장한다.
	//--------------------------------------------------------------
	file.write((const char*)&cksize, 4);
	file.write((const char*)&wavefmt, wavefmtSize);
	file.write((const char*)pBuffer, cksize);

	delete [] pBuffer;
	
	return true;
}

//--------------------------------------------------------------------------
// Load From File Data
//--------------------------------------------------------------------------
/* CSDLAudio::GetDS() (Client/DXLib/CDirectSound.cpp) always returns the
   stub's m_pDS, which is initialized to NULL and never reassigned - actual
   playback goes through SDL_mixer instead. So the original body here
   (g_SDLAudio.GetDS()->CreateSoundBuffer(...)) always dereferenced a null
   pointer if it was ever reached, i.e. it was already dead/crashing code
   before this stub, same category as CSDLGraphics::GetDD() elsewhere (see
   참고자료/작업필요stub.md). Stubbed to return NULL directly instead of
   building a DSBUFFERDESC/calling the real DirectSound API, which also
   avoids pulling in the real <DSound.h> in this translation unit (see the
   comment on the LPDIRECTSOUNDBUFFER typedef in WavePackFileManager.h). */
LPDIRECTSOUNDBUFFER
WavePackFileInfo::LoadFromFileData(std::ifstream& file)
{
	(void)file;
	return NULL;
}

//--------------------------------------------------------------------------
//
//					WavePackFileManager
//
//--------------------------------------------------------------------------
WavePackFileManager::WavePackFileManager()
{
}

WavePackFileManager::~WavePackFileManager()
{
}

//--------------------------------------------------------------------------
// Load From File Data
//--------------------------------------------------------------------------
LPDIRECTSOUNDBUFFER		
WavePackFileManager::LoadFromFileData(TYPE_SOUNDID id)
{
	__BEGIN_PROFILE("WavePackFileManager-Load")

	if (m_DataFilename.c_str()==NULL)
	{
		__END_PROFILE("WavePackFileManager-Load")
		return NULL;
	}

	WavePackFileInfo* pInfo = GetInfo(id);

	if (pInfo!=NULL)
	{
		std::ifstream file(m_DataFilename.c_str(), ios::binary);
		file.seekg( pInfo->GetFilePosition() );
		
		LPDIRECTSOUNDBUFFER pBuffer = pInfo->LoadFromFileData(file);

		file.close();

		__END_PROFILE("WavePackFileManager-Load")

		return pBuffer;
	}

	__END_PROFILE("WavePackFileManager-Load")
	return NULL;
}
