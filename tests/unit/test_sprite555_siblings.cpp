//----------------------------------------------------------------------
// test_sprite555_siblings.cpp
//----------------------------------------------------------------------
//
// CAlphaSprite555 and CIndexSprite555 carry the same run length decode
// as CSprite555, differing only in what each segment holds. Both walk a
// scanline using counts taken from the file, so both need the same bound
// against the length of the scanline they allocated.
//
// CAlphaSprite555 scanline body:
//
//      data[0]                    number of segments
//      per segment:
//          WORD  transparentRun
//          WORD  colourRun
//          then colourRun pairs of (alpha, colour)
//
// CIndexSprite555 scanline body:
//
//      data[0]                    number of transparent pairs
//      per pair:
//          WORD  transparentRun
//          WORD  indexCount
//          WORD  indexData[indexCount]
//          WORD  colourRun
//          WORD  colour[colourRun]
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "CAlphaSprite555.h"
#include "CIndexSprite555.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

namespace {

const char* const	kTempFile = "sprite555_siblings_test.bin";

void	PushWord(std::vector<unsigned char>& bytes, WORD value)
{
	bytes.push_back((unsigned char)(value & 0xFF));
	bytes.push_back((unsigned char)((value >> 8) & 0xFF));
}

void	WriteScanlineSprite(WORD width, WORD height, WORD declaredLen,
			    const std::vector<WORD>& scanline)
{
	std::vector<unsigned char>	bytes;

	PushWord(bytes, width);
	PushWord(bytes, height);
	PushWord(bytes, declaredLen);

	for (size_t i = 0; i < scanline.size(); i++)
		PushWord(bytes, scanline[i]);

	std::ofstream	out(kTempFile, std::ios::binary | std::ios::trunc);

	out.write((const char*)&bytes[0], (std::streamsize)bytes.size());
}

void	RemoveTempFile()
{
	std::remove(kTempFile);
}

} // namespace

//----------------------------------------------------------------------
// CAlphaSprite555: one segment carrying two alpha and colour pairs.
//----------------------------------------------------------------------
TEST(CAlphaSprite555, LoadFromFileReadsAWellFormedSprite)
{
	std::vector<WORD>	scanline;

	scanline.push_back(1);		// one segment
	scanline.push_back(0);		// transparent run
	scanline.push_back(2);		// colour run
	scanline.push_back(0x0F);	// alpha
	scanline.push_back(0xFFFF);	// colour
	scanline.push_back(0x0F);	// alpha
	scanline.push_back(0x07E0);	// colour

	WriteScanlineSprite(2, 1, (WORD)scanline.size(), scanline);

	CAlphaSprite555	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(sprite.LoadFromFile(in));
	CHECK_EQ(2, sprite.GetWidth());
	CHECK_EQ(1, sprite.GetHeight());

	RemoveTempFile();
}

//----------------------------------------------------------------------
// CAlphaSprite555: a colour run past the end of the scanline.
//----------------------------------------------------------------------
TEST(CAlphaSprite555, LoadFromFileRejectsColourRunPastTheScanline)
{
	std::vector<WORD>	scanline;

	scanline.push_back(1);		// one segment
	scanline.push_back(0);		// transparent run
	scanline.push_back(60000);	// colour run, far beyond the scanline
	scanline.push_back(0x0F);
	scanline.push_back(0xFFFF);
	scanline.push_back(0x0F);
	scanline.push_back(0x07E0);

	WriteScanlineSprite(2, 1, (WORD)scanline.size(), scanline);

	CAlphaSprite555	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(!sprite.LoadFromFile(in));

	RemoveTempFile();
}

//----------------------------------------------------------------------
// CAlphaSprite555: an empty scanline has no segment count to read.
//----------------------------------------------------------------------
TEST(CAlphaSprite555, LoadFromFileRejectsEmptyScanline)
{
	std::vector<WORD>	scanline;

	scanline.push_back(0);

	WriteScanlineSprite(2, 1, 0, scanline);

	CAlphaSprite555	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(!sprite.LoadFromFile(in));

	RemoveTempFile();
}

//----------------------------------------------------------------------
// CIndexSprite555: one pair carrying no index data and two colours.
//----------------------------------------------------------------------
TEST(CIndexSprite555, LoadFromFileReadsAWellFormedSprite)
{
	std::vector<WORD>	scanline;

	scanline.push_back(1);		// one transparent pair
	scanline.push_back(0);		// transparent run
	scanline.push_back(0);		// index count
	scanline.push_back(2);		// colour run
	scanline.push_back(0xFFFF);	// colour
	scanline.push_back(0x07E0);	// colour

	WriteScanlineSprite(2, 1, (WORD)scanline.size(), scanline);

	CIndexSprite555	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(sprite.LoadFromFile(in));
	CHECK_EQ(2, sprite.GetWidth());
	CHECK_EQ(1, sprite.GetHeight());

	RemoveTempFile();
}

//----------------------------------------------------------------------
// CIndexSprite555: a colour run past the end of the scanline.
//----------------------------------------------------------------------
TEST(CIndexSprite555, LoadFromFileRejectsColourRunPastTheScanline)
{
	std::vector<WORD>	scanline;

	scanline.push_back(1);
	scanline.push_back(0);
	scanline.push_back(0);
	scanline.push_back(60000);	// colour run, far beyond the scanline
	scanline.push_back(0xFFFF);
	scanline.push_back(0x07E0);

	WriteScanlineSprite(2, 1, (WORD)scanline.size(), scanline);

	CIndexSprite555	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(!sprite.LoadFromFile(in));

	RemoveTempFile();
}

//----------------------------------------------------------------------
// CIndexSprite555: an index count that skips past the scanline.
//
// The index count is added to the walking position before the colour
// count is read from it, so a large value moves the read off the end.
//----------------------------------------------------------------------
TEST(CIndexSprite555, LoadFromFileRejectsIndexCountPastTheScanline)
{
	std::vector<WORD>	scanline;

	scanline.push_back(1);
	scanline.push_back(0);
	scanline.push_back(60000);	// index count, far beyond the scanline
	scanline.push_back(2);
	scanline.push_back(0xFFFF);
	scanline.push_back(0x07E0);

	WriteScanlineSprite(2, 1, (WORD)scanline.size(), scanline);

	CIndexSprite555	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(!sprite.LoadFromFile(in));

	RemoveTempFile();
}

//----------------------------------------------------------------------
// CIndexSprite555: a truncated scanline.
//----------------------------------------------------------------------
TEST(CIndexSprite555, LoadFromFileRejectsTruncatedScanline)
{
	std::vector<WORD>	scanline;

	scanline.push_back(1);
	scanline.push_back(0);

	// Declares twelve WORDs but supplies two.
	WriteScanlineSprite(2, 1, 12, scanline);

	CIndexSprite555	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(!sprite.LoadFromFile(in));

	RemoveTempFile();
}
