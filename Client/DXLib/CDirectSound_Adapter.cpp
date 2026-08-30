/*-----------------------------------------------------------------------------

	CDirectSound_Adapter.cpp

	CSDLAudio on top of the DXLibBackend sound interface (SDL_mixer).

	This is the live implementation of the class declared in CDirectSound.h.
	CDirectSound.cpp beside it is the "not implemented" fallback that the
	build uses only when SDL2_mixer is absent; the two are never compiled
	together (see Client/DXLib/CMakeLists.txt).

	Units: callers pass DirectSound values - volume as an attenuation in
	hundredths of a decibel (DSBVOLUME_MIN..DSBVOLUME_MAX), pan as
	DSBPAN_LEFT..DSBPAN_RIGHT. They are stored as such and converted to the
	backend's linear percentages through AudioVolumeToPercent /
	AudioPanToPercent at the one point they cross into the backend.

	2025.01.14

-----------------------------------------------------------------------------*/

#include "CDirectSound.h"
#include "DXLibBackend.h"

#include <cstdio>

/* Global instance */
CSDLAudio	g_SDLAudio;

/*=============================================================================
 * SDL Backend Implementation
 *=============================================================================*/

#ifdef DXLIB_BACKEND_SDL

namespace {

/* What an LPDIRECTSOUNDBUFFER really points at in this build. */
struct SoundBufferWrapper {
	dxlib_sound_t	sound;			// Backend handle
	LONG			volume;			// DSBVOLUME_MIN..DSBVOLUME_MAX
	LONG			pan;			// DSBPAN_LEFT..DSBPAN_RIGHT
	bool			bAutoRelease;	// Reclaimed by ReleaseTerminatedDuplicateBuffer
};

inline SoundBufferWrapper* Wrap(LPDIRECTSOUNDBUFFER buffer)
{
	return reinterpret_cast<SoundBufferWrapper*>(buffer);
}

inline LPDIRECTSOUNDBUFFER Unwrap(SoundBufferWrapper* wrapper)
{
	return reinterpret_cast<LPDIRECTSOUNDBUFFER>(wrapper);
}

inline LONG ClampVolume(LONG volume)
{
	if (volume < DSBVOLUME_MIN) return DSBVOLUME_MIN;
	if (volume > DSBVOLUME_MAX) return DSBVOLUME_MAX;
	return volume;
}

inline LONG ClampPan(LONG pan)
{
	if (pan < DSBPAN_LEFT) return DSBPAN_LEFT;
	if (pan > DSBPAN_RIGHT) return DSBPAN_RIGHT;
	return pan;
}

/* Store a new volume on the wrapper and hand it to the backend, which
 * applies it to the live channel now and again on every later play. */
bool SetVolume(SoundBufferWrapper* wrapper, LONG volume)
{
	wrapper->volume = ClampVolume(volume);
	return dxlib_sound_set_volume(wrapper->sound, AudioVolumeToPercent(wrapper->volume)) == 0;
}

bool SetPan(SoundBufferWrapper* wrapper, LONG pan)
{
	wrapper->pan = ClampPan(pan);
	return dxlib_sound_set_pan(wrapper->sound, AudioPanToPercent(wrapper->pan)) == 0;
}

SoundBufferWrapper* NewWrapper(dxlib_sound_t sound, LONG volume, LONG pan, bool bAutoRelease)
{
	SoundBufferWrapper* wrapper = new SoundBufferWrapper;
	wrapper->sound = sound;
	wrapper->bAutoRelease = bAutoRelease;
	SetVolume(wrapper, volume);
	SetPan(wrapper, pan);
	return wrapper;
}

void FreeWrapper(SoundBufferWrapper* wrapper)
{
	dxlib_sound_free(wrapper->sound);
	delete wrapper;
}

} // namespace

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------
CSDLAudio::CSDLAudio()
{
	m_pDS = NULL;
	m_bInit = false;
	m_bMute = false;
	m_MaxVolume = DSBVOLUME_MAX;
	m_listDuplicatedBuffer.clear();
}

CSDLAudio::~CSDLAudio()
{
	Release();
}

//-----------------------------------------------------------------------------
// Init / Release
//-----------------------------------------------------------------------------
bool CSDLAudio::Init(HWND hWnd)
{
	if (dxlib_sound_init(hWnd) != 0) {
		return DirectSoundFailed("dxlib_sound_init");
	}

	m_pDS = (LPDIRECTSOUND)0x01;	// Non-null indicator
	m_bInit = true;
	return true;
}

void CSDLAudio::Release()
{
	ReleaseDuplicateBuffer();
	dxlib_sound_release();

	m_pDS = NULL;
	m_bInit = false;
}

void CSDLAudio::ReleaseDuplicateBuffer()
{
	for (LPDIRECTSOUNDBUFFER_LIST::iterator it = m_listDuplicatedBuffer.begin();
		it != m_listDuplicatedBuffer.end(); ++it)
	{
		if (*it) {
			FreeWrapper(Wrap(*it));
		}
	}
	m_listDuplicatedBuffer.clear();
}

bool CSDLAudio::IsInit() const
{
	return m_bInit;
}

LPDIRECTSOUND CSDLAudio::GetDS() const
{
	return m_pDS;
}

bool CSDLAudio::DirectSoundFailed(const char* str)
{
	fprintf(stderr, "[CSDLAudio] %s failed\n", str);
	return false;
}

//-----------------------------------------------------------------------------
// Load / Release / Duplicate
//-----------------------------------------------------------------------------
LPDIRECTSOUNDBUFFER CSDLAudio::LoadWav(LPSTR filename)
{
	if (!m_bInit) return NULL;

	dxlib_sound_t sound = dxlib_sound_load_wav(filename);
	if (!sound) {
		return NULL;
	}

	return Unwrap(NewWrapper(sound, m_MaxVolume, DSBPAN_CENTER, false));
}

LPDIRECTSOUNDBUFFER CSDLAudio::CreateBuffer(LPVOID sdat, DWORD size, DWORD caps, LPWAVEFORMATEX wfx)
{
	(void)caps;

	if (!m_bInit || !wfx) return NULL;

	dxlib_sound_t sound = dxlib_sound_create_buffer(sdat, (int)size,
		wfx->nChannels, (int)wfx->nSamplesPerSec, wfx->wBitsPerSample);
	if (!sound) {
		return NULL;
	}

	return Unwrap(NewWrapper(sound, m_MaxVolume, DSBPAN_CENTER, false));
}

void CSDLAudio::Release(LPDIRECTSOUNDBUFFER buffer)
{
	if (!buffer) return;

	// A buffer on the auto-release list is owned by that list; releasing it
	// here as well would free it twice on the next game tick.
	m_listDuplicatedBuffer.remove(buffer);

	FreeWrapper(Wrap(buffer));
}

LPDIRECTSOUNDBUFFER CSDLAudio::DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER buffer, bool bAutoRelease)
{
	if (!buffer) return NULL;

	SoundBufferWrapper* source = Wrap(buffer);

	dxlib_sound_t duplicated = dxlib_sound_duplicate(source->sound);
	if (!duplicated) {
		return NULL;
	}

	SoundBufferWrapper* wrapper = NewWrapper(duplicated, source->volume, source->pan, bAutoRelease);

	if (bAutoRelease) {
		m_listDuplicatedBuffer.push_back(Unwrap(wrapper));
	}

	return Unwrap(wrapper);
}

//-----------------------------------------------------------------------------
// Play / Stop
//-----------------------------------------------------------------------------
bool CSDLAudio::IsPlay(LPDIRECTSOUNDBUFFER buffer) const
{
	if (!buffer) return false;

	return dxlib_sound_is_playing(Wrap(buffer)->sound) != 0;
}

/* Restart from the beginning, cutting off any playback of this buffer. */
bool CSDLAudio::NewPlay(LPDIRECTSOUNDBUFFER buffer, bool bLoop)
{
	if (!buffer) return false;
	if (m_bMute) return false;

	SoundBufferWrapper* wrapper = Wrap(buffer);

	if (dxlib_sound_is_playing(wrapper->sound)) {
		dxlib_sound_stop(wrapper->sound);
	}

	return dxlib_sound_play(wrapper->sound, bLoop ? 1 : 0) == 0;
}

/* Play without cutting off a running instance: when the buffer is already
 * sounding and the caller allows it, an auto-released duplicate plays
 * alongside instead. */
bool CSDLAudio::Play(LPDIRECTSOUNDBUFFER buffer, bool bLoop, bool bDuplicate)
{
	if (!buffer) return false;
	if (m_bMute) return false;

	SoundBufferWrapper* wrapper = Wrap(buffer);

	if (bDuplicate && dxlib_sound_is_playing(wrapper->sound)) {
		LPDIRECTSOUNDBUFFER dup = DuplicateSoundBuffer(buffer, true);
		if (dup) {
			wrapper = Wrap(dup);
		}
	}

	return dxlib_sound_play(wrapper->sound, bLoop ? 1 : 0) == 0;
}

bool CSDLAudio::Stop(LPDIRECTSOUNDBUFFER buffer)
{
	if (!buffer) return false;

	return dxlib_sound_stop(Wrap(buffer)->sound) == 0;
}

/* Called once per game tick to reclaim duplicates that have finished. */
void CSDLAudio::ReleaseTerminatedDuplicateBuffer()
{
	LPDIRECTSOUNDBUFFER_LIST::iterator it = m_listDuplicatedBuffer.begin();
	while (it != m_listDuplicatedBuffer.end()) {
		SoundBufferWrapper* wrapper = Wrap(*it);
		if (wrapper && !dxlib_sound_is_playing(wrapper->sound)) {
			FreeWrapper(wrapper);
			it = m_listDuplicatedBuffer.erase(it);
		} else {
			++it;
		}
	}
}

//-----------------------------------------------------------------------------
// Mute
//-----------------------------------------------------------------------------
bool CSDLAudio::IsMute() const
{
	return m_bMute;
}

void CSDLAudio::SetMute()
{
	m_bMute = true;
}

void CSDLAudio::UnSetMute()
{
	m_bMute = false;
}

//-----------------------------------------------------------------------------
// Frequency - SDL_mixer cannot change a channel's playback rate.
//-----------------------------------------------------------------------------
bool CSDLAudio::AddFrequency(LPDIRECTSOUNDBUFFER buffer, int step)
{
	(void)buffer;
	(void)step;
	return false;
}

bool CSDLAudio::SubFrequency(LPDIRECTSOUNDBUFFER buffer, int step)
{
	(void)buffer;
	(void)step;
	return false;
}

//-----------------------------------------------------------------------------
// Volume
//-----------------------------------------------------------------------------
bool CSDLAudio::SetMaxVolume(LPDIRECTSOUNDBUFFER buffer)
{
	if (!buffer) return false;

	return SetVolume(Wrap(buffer), m_MaxVolume);
}

bool CSDLAudio::AddVolume(LPDIRECTSOUNDBUFFER buffer, int step)
{
	if (!buffer) return false;

	SoundBufferWrapper* wrapper = Wrap(buffer);
	LONG volume = wrapper->volume + step;
	if (volume > m_MaxVolume) volume = m_MaxVolume;
	return SetVolume(wrapper, volume);
}

bool CSDLAudio::SubVolume(LPDIRECTSOUNDBUFFER buffer, int step)
{
	if (!buffer) return false;

	SoundBufferWrapper* wrapper = Wrap(buffer);
	return SetVolume(wrapper, wrapper->volume - step);
}

/* The distance attenuation the zone sound code uses: `step` below the
 * current limit, in hundredths of a dB. */
bool CSDLAudio::SubVolumeFromMax(LPDIRECTSOUNDBUFFER buffer, int step)
{
	if (!buffer) return false;

	return SetVolume(Wrap(buffer), m_MaxVolume - step);
}

void CSDLAudio::SetVolumeLimit(LONG volume)
{
	m_MaxVolume = ClampVolume(volume);
}

LONG CSDLAudio::GetVolumeLimit() const
{
	return m_MaxVolume;
}

//-----------------------------------------------------------------------------
// Pan
//-----------------------------------------------------------------------------
bool CSDLAudio::RightPan(LPDIRECTSOUNDBUFFER buffer, int step)
{
	if (!buffer) return false;

	SoundBufferWrapper* wrapper = Wrap(buffer);
	return SetPan(wrapper, wrapper->pan + step);
}

bool CSDLAudio::LeftPan(LPDIRECTSOUNDBUFFER buffer, int step)
{
	if (!buffer) return false;

	SoundBufferWrapper* wrapper = Wrap(buffer);
	return SetPan(wrapper, wrapper->pan - step);
}

bool CSDLAudio::CenterToRightPan(LPDIRECTSOUNDBUFFER buffer, int step)
{
	if (!buffer) return false;

	return SetPan(Wrap(buffer), DSBPAN_CENTER + step);
}

bool CSDLAudio::CenterToLeftPan(LPDIRECTSOUNDBUFFER buffer, int step)
{
	if (!buffer) return false;

	return SetPan(Wrap(buffer), DSBPAN_CENTER - step);
}

bool CSDLAudio::CenterPan(LPDIRECTSOUNDBUFFER buffer)
{
	if (!buffer) return false;

	return SetPan(Wrap(buffer), DSBPAN_CENTER);
}

bool CSDLAudio::ChangePan(LPDIRECTSOUNDBUFFER buffer, int pan)
{
	if (!buffer) return false;

	return SetPan(Wrap(buffer), pan);
}

#endif /* DXLIB_BACKEND_SDL */
