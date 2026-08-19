//-----------------------------------------------------------------------------
// CSoundPartManager.h
//-----------------------------------------------------------------------------

#ifndef	__CSOUNDPARTMANAGER_H__
#define __CSOUNDPARTMANAGER_H__

#include "CPartManager.h"

/* This project no longer uses real DirectSound - including the real
   <DSound.h> here redefines _DSBPOSITIONNOTIFY (from basic/AudioTypes.h)
   with an incompatible duplicate, matching the same category of conflict
   already fixed for CDirectDraw/MMSystem elsewhere. basic/Platform.h only
   forward-declares LPDIRECTSOUNDBUFFER on non-Windows (it expects the real
   header to supply it on Windows), so declare the opaque pointer type here
   directly for all platforms instead. */
#include "../../basic/Platform.h"

#ifndef LPDIRECTSOUNDBUFFER
typedef struct IDirectSoundBuffer* LPDIRECTSOUNDBUFFER;
#endif

class CSoundPartManager : public CPartManager<WORD, BYTE, LPDIRECTSOUNDBUFFER> {
	public :
		CSoundPartManager()		{}
		~CSoundPartManager()	{ Release(); }

		void			Release();

		void			Stop();

	protected :

};

#endif
