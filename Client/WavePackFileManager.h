//--------------------------------------------------------------------------
// WavePackFileManager.h
//--------------------------------------------------------------------------
/*
	
		// WaveFileData

		WAVEFORMATEX	wavefmt;
		DWORD			size;
		char*			pBuffer;

*/
//--------------------------------------------------------------------------


#ifndef __WAVE_PACK_FILE_MANAGER__
#define __WAVE_PACK_FILE_MANAGER__

#include "PackFileManager.h"
#include "MTypeDef.h"

/* Including the real <DSound.h> here redefines _DSBPOSITIONNOTIFY (from
   basic/AudioTypes.h) with an incompatible duplicate, matching the same
   category of conflict already fixed for CDirectDraw/MMSystem elsewhere
   (see CSoundPartManager.h/MZoneSoundManager.h). basic/Platform.h only
   forward-declares LPDIRECTSOUNDBUFFER on non-Windows (it expects the real
   header to supply it on Windows), so declare the opaque pointer type here
   directly for all platforms instead - this header only ever names the
   type as an opaque pointer. WavePackFileInfo::LoadFromFileData() (the one
   place that used to need the real DSBUFFERDESC/CreateSoundBuffer API) is
   stubbed out in the .cpp; see 참고자료/작업필요stub.md. */
#ifndef LPDIRECTSOUNDBUFFER
typedef struct IDirectSoundBuffer* LPDIRECTSOUNDBUFFER;
#endif

//--------------------------------------------------------------------------
// WavePackFileInfo
//--------------------------------------------------------------------------
class WavePackFileInfo : public PackFileInfo {
	public :
		WavePackFileInfo() {}
		virtual ~WavePackFileInfo() {}

		virtual bool			SaveToFileData(std::ofstream& file);
		LPDIRECTSOUNDBUFFER		LoadFromFileData(std::ifstream& file);
};

//--------------------------------------------------------------------------
// WavePackFileManager
//--------------------------------------------------------------------------
class WavePackFileManager : public PackFileManager<WavePackFileInfo> {
	public :
		WavePackFileManager();
		virtual ~WavePackFileManager();

		LPDIRECTSOUNDBUFFER		LoadFromFileData(TYPE_SOUNDID id);
};

extern WavePackFileManager*	g_pWavePackFileManager;

#endif