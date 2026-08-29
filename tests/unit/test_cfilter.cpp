//----------------------------------------------------------------------
// test_cfilter.cpp
//----------------------------------------------------------------------
//
// Tests for CFilter in Client/SpriteLib/CFilter.h.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "CFilter.h"

#include <cstdio>
#include <cstring>
#include <fstream>

namespace {

const char* const	kTempFile = "cfilter_loadfromfile_test.bin";

void	WriteRawFile(const void* data, size_t bytes)
{
	std::ofstream	out(kTempFile, std::ios::binary | std::ios::trunc);

	if (bytes > 0)
		out.write((const char*)data, (std::streamsize)bytes);
}

//----------------------------------------------------------------------
// Builds a CFilter file: a WORD width, a WORD height, then height rows
// of width bytes. bodyBytes controls how much of that body is actually
// written, so a caller can produce a truncated file.
//----------------------------------------------------------------------
void	WriteFilterFile(WORD width, WORD height, size_t bodyBytes)
{
	unsigned char	buffer[4 + 256];

	std::memcpy(buffer, &width, sizeof(width));
	std::memcpy(buffer + 2, &height, sizeof(height));

	if (bodyBytes > sizeof(buffer) - 4)
		bodyBytes = sizeof(buffer) - 4;

	for (size_t i = 0; i < bodyBytes; i++)
		buffer[4 + i] = (unsigned char)(i & 0xFF);

	WriteRawFile(buffer, 4 + bodyBytes);
}

void	RemoveTempFile()
{
	std::remove(kTempFile);
}

} // namespace

//----------------------------------------------------------------------
// IsInit and IsNotInit must be opposites.
//
// Both were written as "m_ppFilter == NULL", so IsInit answered the
// question backwards: it reported true for a filter with no storage and
// false once Init had allocated it.
//----------------------------------------------------------------------
TEST(CFilter, IsInitReportsWhetherStorageIsAllocated)
{
	CFilter	filter;

	CHECK(!filter.IsInit());
	CHECK(filter.IsNotInit());

	filter.Init(4, 4);

	CHECK(filter.IsInit());
	CHECK(!filter.IsNotInit());

	filter.Release();

	CHECK(!filter.IsInit());
	CHECK(filter.IsNotInit());
}

//----------------------------------------------------------------------
// A well formed file loads.
//----------------------------------------------------------------------
TEST(CFilter, LoadFromFileReadsAWellFormedFile)
{
	WriteFilterFile(4, 3, 4 * 3);

	CFilter		filter;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(filter.LoadFromFile(in));
	CHECK_EQ(4, filter.GetWidth());
	CHECK_EQ(3, filter.GetHeight());
	CHECK(filter.IsInit());

	RemoveTempFile();
}

//----------------------------------------------------------------------
// A height larger than the file must be rejected before it is used.
//
// LoadFromFile read the header straight into m_Width and m_Height and
// only then called Init(), which begins with Release(). Release frees
// m_ppFilter[i] for i below m_Height, but m_ppFilter still held the row
// count from the previous contents. A file declaring 1000 rows against
// a filter holding 4 therefore ran delete[] over 996 pointers read from
// past the end of the row array: a free driven entirely by file data.
//----------------------------------------------------------------------
TEST(CFilter, LoadFromFileRejectsHeightLargerThanTheFile)
{
	WriteFilterFile(4, 1000, 0);

	CFilter	filter;

	filter.Init(4, 4);

	CHECK(filter.IsInit());
	CHECK_EQ(4, filter.GetHeight());

	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(!filter.LoadFromFile(in));

	// The rejected header must not have disturbed the existing filter.
	CHECK_EQ(4, filter.GetWidth());
	CHECK_EQ(4, filter.GetHeight());

	RemoveTempFile();
}

//----------------------------------------------------------------------
// A file whose body is shorter than the declared size is rejected.
//----------------------------------------------------------------------
TEST(CFilter, LoadFromFileRejectsTruncatedBody)
{
	// Declares 8 x 8 but carries only 10 of the 64 body bytes.
	WriteFilterFile(8, 8, 10);

	CFilter		filter;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(!filter.LoadFromFile(in));

	RemoveTempFile();
}

//----------------------------------------------------------------------
// A file too short to even hold the header is rejected.
//----------------------------------------------------------------------
TEST(CFilter, LoadFromFileRejectsTruncatedHeader)
{
	const unsigned char	singleByte = 0x04;

	WriteRawFile(&singleByte, 1);

	CFilter		filter;
	std::ifstream	in(kTempFile, std::ios::binary);

	CHECK(!filter.LoadFromFile(in));
	CHECK(!filter.IsInit());

	RemoveTempFile();
}
