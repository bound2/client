#ifndef __SOUND_SETTING__
#define __SOUND_SETTING__

//#define __USE_MP3__


#ifdef __USE_MP3__
#else
#include "COGGSTREAM.h"
extern COGGSTREAM*			g_pOGG;
extern CDirectSoundBuffer*	g_pSoundBufferForOGG;
extern FILE	*				g_oggfile;
#endif //				__USE_MP3__

#endif	// __SOUND_SETTING