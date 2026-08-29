//----------------------------------------------------------------------
// test_calphaspritepal.cpp
//----------------------------------------------------------------------
//
// Tests for CAlphaSpritePal::Blt in Client/SpriteLib/CAlphaSpritePal.cpp.
//
// Blt walks the run length encoded scanline at draw time and writes the
// decoded pixels straight into the locked destination surface. Both run
// lengths come from the sprite file, so without a bound the destination
// pointer walks past the end of the scanline it was given.
//
// Scanline layout, all counts one byte:
//
//      BYTE  segmentCount
//      per segment:
//          BYTE  transparentRun
//          BYTE  colourRun
//          then colourRun pairs of (alpha, paletteIndex)
//
// The destination is allocated with a wide guard band filled with a
// sentinel. A write outside the sprite's own width lands in that band
// and is detected directly, without depending on the process faulting.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "CAlphaSpritePal.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

namespace {

const char* const	kTempFile = "calphaspritepal_test.bin";

const WORD	kSentinel	= 0xDEAD;
const int	kSpriteWidth	= 4;
const int	kGuardWords	= 512;

void	PushWord(std::vector<unsigned char>& bytes, WORD value)
{
	bytes.push_back((unsigned char)(value & 0xFF));
	bytes.push_back((unsigned char)((value >> 8) & 0xFF));
}

void	PushDword(std::vector<unsigned char>& bytes, DWORD value)
{
	bytes.push_back((unsigned char)(value & 0xFF));
	bytes.push_back((unsigned char)((value >> 8) & 0xFF));
	bytes.push_back((unsigned char)((value >> 16) & 0xFF));
	bytes.push_back((unsigned char)((value >> 24) & 0xFF));
}

//----------------------------------------------------------------------
// Writes a single scanline sprite in the CSpritePalBase container
// format: size, width, height, the pixel data, then one WORD per
// scanline giving that scanline's length in bytes.
//----------------------------------------------------------------------
void	WriteSprite(const std::vector<unsigned char>& scanline)
{
	std::vector<unsigned char>	bytes;

	PushDword(bytes, (DWORD)scanline.size());
	PushWord(bytes, (WORD)kSpriteWidth);
	PushWord(bytes, 1);

	for (size_t i = 0; i < scanline.size(); i++)
		bytes.push_back(scanline[i]);

	PushWord(bytes, (WORD)scanline.size());

	std::ofstream	out(kTempFile, std::ios::binary | std::ios::trunc);

	out.write((const char*)&bytes[0], (std::streamsize)bytes.size());
}

//----------------------------------------------------------------------
// Builds one segment: a transparent run, a colour run, and the (alpha,
// palette index) pairs the colour run promises.
//----------------------------------------------------------------------
std::vector<unsigned char>	MakeScanline(int transparentRun, int colourRun)
{
	std::vector<unsigned char>	scanline;

	scanline.push_back(1);				// one segment
	scanline.push_back((unsigned char)transparentRun);
	scanline.push_back((unsigned char)colourRun);

	for (int i = 0; i < colourRun; i++)
	{
		scanline.push_back(0x20);		// alpha
		scanline.push_back(0x01);		// palette index
	}

	return scanline;
}

void	RemoveTempFile()
{
	std::remove(kTempFile);
}

//----------------------------------------------------------------------
// Destination surface with a sentinel filled guard band to the right of
// the sprite.
//----------------------------------------------------------------------
class GuardedSurface
{
public:
	GuardedSurface() : m_Pixels(kSpriteWidth + kGuardWords, kSentinel) {}

	WORD*	Data()		{ return &m_Pixels[0]; }
	WORD	Pitch() const	{ return (WORD)(m_Pixels.size() * sizeof(WORD)); }

	//------------------------------------------------------------
	// Nothing beyond the sprite's own width may have been written.
	//------------------------------------------------------------
	bool	GuardBandIntact() const
	{
		for (size_t i = kSpriteWidth; i < m_Pixels.size(); i++)
		{
			if (m_Pixels[i] != kSentinel)
				return false;
		}

		return true;
	}

private:
	std::vector<WORD>	m_Pixels;
};

//----------------------------------------------------------------------
// A palette large enough for every index the test sprites use.
//----------------------------------------------------------------------
void	FillPalette(MPalette& pal)
{
	pal.Init(255);

	for (int i = 0; i < 255; i++)
		pal[(BYTE)i] = (WORD)(i * 7);
}

} // namespace

//----------------------------------------------------------------------
// A sprite that fills its declared width draws inside the surface.
//----------------------------------------------------------------------
TEST(CAlphaSpritePal, BltStaysInsideTheSurfaceForAWellFormedSprite)
{
	WriteSprite(MakeScanline(0, kSpriteWidth));

	CAlphaSpritePal	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(sprite.LoadFromFile(in));

	MPalette	pal;

	FillPalette(pal);

	GuardedSurface	surface;

	sprite.Blt(surface.Data(), surface.Pitch(), pal);

	CHECK(surface.GuardBandIntact());

	RemoveTempFile();
}

//----------------------------------------------------------------------
// A transparent run wider than the sprite must not move the destination
// pointer past the scanline.
//
// Blt added the run straight to the destination pointer with nothing
// comparing it against the sprite's width, so the colours that followed
// were written far to the right of where the sprite ends.
//----------------------------------------------------------------------
TEST(CAlphaSpritePal, BltRejectsTransparentRunWiderThanTheSprite)
{
	WriteSprite(MakeScanline(200, 50));

	CAlphaSpritePal	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(sprite.LoadFromFile(in));

	MPalette	pal;

	FillPalette(pal);

	GuardedSurface	surface;

	sprite.Blt(surface.Data(), surface.Pitch(), pal);

	CHECK(surface.GuardBandIntact());

	RemoveTempFile();
}

//----------------------------------------------------------------------
// A colour run wider than the sprite must not be drawn past its width.
//----------------------------------------------------------------------
TEST(CAlphaSpritePal, BltRejectsColourRunWiderThanTheSprite)
{
	WriteSprite(MakeScanline(0, 200));

	CAlphaSpritePal	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(sprite.LoadFromFile(in));

	MPalette	pal;

	FillPalette(pal);

	GuardedSurface	surface;

	sprite.Blt(surface.Data(), surface.Pitch(), pal);

	CHECK(surface.GuardBandIntact());

	RemoveTempFile();
}
