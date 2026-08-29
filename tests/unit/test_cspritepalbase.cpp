//----------------------------------------------------------------------
// test_cspritepalbase.cpp
//----------------------------------------------------------------------
//
// Tests for CSpritePalBase::LoadFromFile in
// Client/SpriteLib/CSpritePalBase.cpp.
//
// The on-disk layout is:
//
//      DWORD  size      bytes of packed pixel data
//      WORD   width
//      WORD   height
//      BYTE   data[size]
//      WORD   index[height]   per-scanline byte counts
//
// The loader turns the index array into m_pPixels[], a table of pointers
// into the pixel data, by accumulating the counts. Those counts come
// from the file, so the running total has to be kept inside the
// allocation or the scanline pointers end up aimed at unrelated memory
// and are later dereferenced by the Blt routines.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "CSpritePalBase.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

namespace {

//----------------------------------------------------------------------
// CSpritePalBase is abstract. This stands in for a concrete sprite so
// the parser can be exercised on its own, and exposes the invariant the
// loader is supposed to maintain.
//----------------------------------------------------------------------
class TestSpritePal : public CSpritePalBase
{
public:
	virtual void	Blt(int, int, WORD*, int, MPalette&)	{}
	virtual bool	IsColorPixel(short, short)		{ return false; }
	virtual WORD	GetPixel(short, short, MPalette&)	{ return 0; }

	//------------------------------------------------------------
	// Every scanline pointer must land inside the pixel data. One
	// past the end is allowed, for an empty trailing scanline.
	//------------------------------------------------------------
	bool	ScanlinePointersAreInsideData() const
	{
		if (m_pData == NULL)
			return true;

		for (int i = 0; i < m_Height; i++)
		{
			if (m_pPixels[i] < m_pData)
				return false;

			if (m_pPixels[i] > m_pData + m_Size)
				return false;
		}

		return true;
	}
};

const char* const	kTempFile = "cspritepalbase_loadfromfile_test.bin";

void	WriteRawFile(const std::vector<unsigned char>& bytes)
{
	std::ofstream	out(kTempFile, std::ios::binary | std::ios::trunc);

	if (!bytes.empty())
		out.write((const char*)&bytes[0], (std::streamsize)bytes.size());
}

//----------------------------------------------------------------------
// Builds a sprite file. dataBytes controls how much pixel data is
// actually written, so a caller can declare more than it supplies.
//----------------------------------------------------------------------
void	WriteSpriteFile(DWORD declaredSize, WORD width, WORD height,
			size_t dataBytes, const std::vector<WORD>& index)
{
	std::vector<unsigned char>	bytes;

	bytes.resize(8);

	std::memcpy(&bytes[0], &declaredSize, 4);
	std::memcpy(&bytes[4], &width, 2);
	std::memcpy(&bytes[6], &height, 2);

	for (size_t i = 0; i < dataBytes; i++)
		bytes.push_back((unsigned char)(i & 0xFF));

	for (size_t i = 0; i < index.size(); i++)
	{
		const WORD	entry = index[i];

		bytes.push_back((unsigned char)(entry & 0xFF));
		bytes.push_back((unsigned char)((entry >> 8) & 0xFF));
	}

	WriteRawFile(bytes);
}

void	RemoveTempFile()
{
	std::remove(kTempFile);
}

} // namespace

//----------------------------------------------------------------------
// A well formed sprite loads and its scanline table stays in range.
//----------------------------------------------------------------------
TEST(CSpritePalBase, LoadFromFileReadsAWellFormedSprite)
{
	std::vector<WORD>	index;

	index.push_back(4);
	index.push_back(4);

	WriteSpriteFile(8, 4, 2, 8, index);

	TestSpritePal	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(sprite.LoadFromFile(in));
	CHECK_EQ(4, sprite.GetWidth());
	CHECK_EQ(2, sprite.GetHeight());
	CHECK(sprite.ScanlinePointersAreInsideData());

	RemoveTempFile();
}

//----------------------------------------------------------------------
// Scanline offsets that run past the pixel data must be rejected.
//
// The loader accumulated the index entries into a pointer without ever
// comparing the running total against the size of the data it had
// allocated, so a file supplying large counts produced scanline
// pointers aimed far outside the buffer.
//----------------------------------------------------------------------
TEST(CSpritePalBase, LoadFromFileRejectsScanlineOffsetsPastTheData)
{
	std::vector<WORD>	index;

	// Eight bytes of pixel data, but the first scanline claims 60000.
	index.push_back(60000);
	index.push_back(60000);

	WriteSpriteFile(8, 4, 2, 8, index);

	TestSpritePal	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	const bool	loaded = sprite.LoadFromFile(in);

	// Whether or not the load is reported as successful, no scanline
	// pointer may be left aimed outside the allocation.
	CHECK(sprite.ScanlinePointersAreInsideData());
	CHECK(!loaded);

	RemoveTempFile();
}

//----------------------------------------------------------------------
// A declared data size the file cannot supply must be rejected before
// it is used to size an allocation.
//----------------------------------------------------------------------
TEST(CSpritePalBase, LoadFromFileRejectsSizeLargerThanTheFile)
{
	std::vector<WORD>	index;

	index.push_back(1);

	// Claims 100000 bytes of pixel data while carrying 4.
	WriteSpriteFile(100000, 4, 1, 4, index);

	TestSpritePal	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(!sprite.LoadFromFile(in));
	CHECK(sprite.ScanlinePointersAreInsideData());

	RemoveTempFile();
}

//----------------------------------------------------------------------
// A file that cannot supply the index array is rejected too.
//----------------------------------------------------------------------
TEST(CSpritePalBase, LoadFromFileRejectsMissingIndexArray)
{
	std::vector<WORD>	index;

	// Declares four scanlines but supplies no index entries at all.
	WriteSpriteFile(8, 4, 4, 8, index);

	TestSpritePal	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(!sprite.LoadFromFile(in));
	CHECK(sprite.ScanlinePointersAreInsideData());

	RemoveTempFile();
}

//----------------------------------------------------------------------
// A file too short to hold the header is rejected.
//----------------------------------------------------------------------
TEST(CSpritePalBase, LoadFromFileRejectsTruncatedHeader)
{
	std::vector<unsigned char>	bytes;

	bytes.push_back(0x08);
	bytes.push_back(0x00);

	WriteRawFile(bytes);

	TestSpritePal	sprite;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(!sprite.LoadFromFile(in));
	CHECK(sprite.ScanlinePointersAreInsideData());

	RemoveTempFile();
}
