//-----------------------------------------------------------------------------
// CSoundPartManager.h
//-----------------------------------------------------------------------------

#ifndef	__CSOUNDPARTMANAGER_H__
#define __CSOUNDPARTMANAGER_H__

#include "CPartManager.h"

/* This project no longer uses real DirectSound - basic/Platform.h already
   forward-declares LPDIRECTSOUNDBUFFER as an SDL-backed stand-in. Including
   the real <DSound.h> here redefines _DSBPOSITIONNOTIFY (from basic/AudioTypes.h)
   with an incompatible duplicate, matching the same category of conflict
   already fixed for CDirectDraw/MMSystem elsewhere. */
#include "../../basic/Platform.h"

class CSoundPartManager : public CPartManager<WORD, BYTE, LPDIRECTSOUNDBUFFER> {
	public :
		CSoundPartManager()		{}
		~CSoundPartManager()	{ Release(); }

		void			Release();

		void			Stop();

	protected :

};

#endif
