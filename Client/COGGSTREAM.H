#ifndef __COGGSTREAM_H__
#define __COGGSTREAM_H__

#include <stdio.h>

class CDirectSoundBuffer;

struct tag_position_set {
	int position;
	// Add other fields as needed
};

// Legacy sound-library-style constants kept for call-site compatibility.
// streamLoad()/streamPlay() now return bool directly (true = success), and
// the SDL-backend stub implementation (COGGSTREAM.cpp) ignores ulFlags
// entirely, so SOUND_PLAY_ONCE's value doesn't matter beyond being a valid
// unsigned long.
const bool SOUND_ERR_OK = true;
const unsigned long SOUND_PLAY_ONCE = 0;

class COGGSTREAM
{
public:
	COGGSTREAM(void* pDirectSound, CDirectSoundBuffer* pBuffer,
	           unsigned long ulMin, unsigned long ulMax,
	           unsigned long ulFlags, unsigned long ulBufferSize);
	~COGGSTREAM();

	bool streamLoad(FILE* file, tag_position_set* pos);
	void streamClose();
	bool streamPlay(unsigned long ulFlags);
	bool streamUpdate(void* pBuffer);
	long streamVolume(long lVolume);

private:
	CDirectSoundBuffer* m_pBuffer;
	bool m_bPlaying;
};

#endif /* __COGGSTREAM_H__ */
