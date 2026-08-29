//----------------------------------------------------------------------
// test_mpalette.cpp
//----------------------------------------------------------------------
//
// Tests for MPalette in Client/SpriteLib/MPalette.h.
//
// MPalette owns a raw WORD* and frees it in its destructor, so it needs
// copy semantics that give each instance its own storage.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "MPalette.h"

//----------------------------------------------------------------------
// Copy construction must deep copy.
//
// MPalette declares a destructor that runs delete[] on its colour table
// but no copy constructor, so the compiler supplies a member-wise one
// and both instances end up owning the same pointer.
//----------------------------------------------------------------------
TEST(MPalette, CopyConstructionProducesIndependentStorage)
{
	MPalette	original;

	original.Init(4);

	original[0] = 0x1111;
	original[1] = 0x2222;
	original[2] = 0x3333;
	original[3] = 0x4444;

	MPalette	copy(original);

	CHECK_EQ(4, copy.GetSize());
	CHECK_EQ(0x1111, copy[0]);
	CHECK_EQ(0x4444, copy[3]);

	copy[0] = 0x9999;

	CHECK_EQ(0x1111, original[0]);
	CHECK_EQ(0x9999, copy[0]);
}

//----------------------------------------------------------------------
// Copying an uninitialised palette must not allocate or fault.
//----------------------------------------------------------------------
TEST(MPalette, CopyConstructionHandlesEmptyPalette)
{
	MPalette	original;

	CHECK_EQ(0, original.GetSize());
	CHECK(!original.IsInit());

	MPalette	copy(original);

	CHECK_EQ(0, copy.GetSize());
}

//----------------------------------------------------------------------
// Assigning a palette to itself must leave it alone.
//
// operator= calls Release() first, which frees the colour table and
// zeroes the size. When the source and destination are the same object
// that also empties the source, so the size copied back is zero and the
// palette is silently discarded.
//----------------------------------------------------------------------
TEST(MPalette, SelfAssignmentPreservesContents)
{
	MPalette	palette;

	palette.Init(3);

	palette[0] = 0x0A0A;
	palette[1] = 0x0B0B;
	palette[2] = 0x0C0C;

	const MPalette&	alias = palette;

	palette = alias;

	CHECK_EQ(3, palette.GetSize());
	CHECK_EQ(0x0A0A, palette[0]);
	CHECK_EQ(0x0B0B, palette[1]);
	CHECK_EQ(0x0C0C, palette[2]);
}

//----------------------------------------------------------------------
// Ordinary assignment deep copies, so the palettes stay independent.
//----------------------------------------------------------------------
TEST(MPalette, AssignmentProducesIndependentStorage)
{
	MPalette	source;

	source.Init(2);

	source[0] = 0x1234;
	source[1] = 0x5678;

	MPalette	destination;

	destination = source;

	CHECK_EQ(2, destination.GetSize());
	CHECK_EQ(0x1234, destination[0]);
	CHECK_EQ(0x5678, destination[1]);

	destination[0] = 0x4321;

	CHECK_EQ(0x1234, source[0]);
}

//----------------------------------------------------------------------
// Indexing an uninitialised palette must not dereference null.
//
// operator[] returned m_pColor[n] with nothing checking that the table
// existed, so reading through the reference on a palette that had never
// been given storage faulted.
//----------------------------------------------------------------------
TEST(MPalette, IndexingAnEmptyPaletteDoesNotDereferenceNull)
{
	MPalette	palette;

	CHECK_EQ(0, palette.GetSize());
	CHECK_EQ(0, palette[0]);
	CHECK_EQ(0, palette[200]);
}

//----------------------------------------------------------------------
// An index at or beyond the table size must not read past the table.
//
// The index is a full byte while the table holds m_Size entries, and
// sprite pixel bytes are used as palette indices directly, so a sprite
// carrying an index larger than the palette read out of bounds while
// being drawn.
//----------------------------------------------------------------------
TEST(MPalette, IndexBeyondSizeDoesNotReadPastTheTable)
{
	MPalette	palette;

	palette.Init(4);

	palette[0] = 0x1111;
	palette[1] = 0x2222;
	palette[2] = 0x3333;
	palette[3] = 0x4444;

	CHECK_EQ(0x4444, palette[3]);

	// One past the end, and far past the end.
	CHECK_EQ(0, palette[4]);
	CHECK_EQ(0, palette[200]);
	CHECK_EQ(0, palette[255]);
}
