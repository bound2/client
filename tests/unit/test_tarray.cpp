//----------------------------------------------------------------------
// test_tarray.cpp
//----------------------------------------------------------------------
//
// Tests for the TArray template in Client/framelib/TArray.h.
//
// TArray owns a raw DataType* and frees it in its destructor, so it needs
// copy semantics that give each instance its own storage. These tests
// cover that ownership contract.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "TArray.h"

//----------------------------------------------------------------------
// Copy construction must deep copy.
//
// TArray declares a destructor that runs delete[] on its buffer. Without
// a matching copy constructor the compiler supplies a member-wise one,
// which hands both instances the same pointer: mutating one is visible
// through the other, and the second destructor frees memory the first
// already released.
//----------------------------------------------------------------------
TEST(TArray, CopyConstructionProducesIndependentStorage)
{
	TArray<int, WORD>	original(3);

	original[0] = 10;
	original[1] = 20;
	original[2] = 30;

	TArray<int, WORD>	copy(original);

	CHECK_EQ(3, copy.GetSize());
	CHECK_EQ(10, copy[0]);
	CHECK_EQ(20, copy[1]);
	CHECK_EQ(30, copy[2]);

	// Writing through the copy must not disturb the original.
	copy[0] = 999;

	CHECK_EQ(10, original[0]);
	CHECK_EQ(999, copy[0]);

	// Both instances are destroyed here. With shared storage the second
	// destructor frees an already released buffer.
}

//----------------------------------------------------------------------
// Copying an empty array must not allocate or fault.
//----------------------------------------------------------------------
TEST(TArray, CopyConstructionHandlesEmptyArray)
{
	TArray<int, WORD>	original;

	CHECK_EQ(0, original.GetSize());

	TArray<int, WORD>	copy(original);

	CHECK_EQ(0, copy.GetSize());
}

//----------------------------------------------------------------------
// Appending within the range of SizeType concatenates both arrays.
//----------------------------------------------------------------------
TEST(TArray, AppendConcatenatesElements)
{
	TArray<int, BYTE>	target(3);

	target[0] = 1;
	target[1] = 2;
	target[2] = 3;

	TArray<int, BYTE>	source(2);

	source[0] = 4;
	source[1] = 5;

	target += source;

	CHECK_EQ(5, target.GetSize());
	CHECK_EQ(1, target[0]);
	CHECK_EQ(2, target[1]);
	CHECK_EQ(3, target[2]);
	CHECK_EQ(4, target[3]);
	CHECK_EQ(5, target[4]);

	// The source must be left untouched.
	CHECK_EQ(2, source.GetSize());
	CHECK_EQ(4, source[0]);
}

//----------------------------------------------------------------------
// Appending must never write more elements than it allocated.
//
// operator+= sized its new buffer with "SizeType newSize = m_Size +
// array.m_Size". For a narrow SizeType the sum is computed as an int and
// then truncated on assignment, so 200 + 100 allocates 44 elements while
// the copy loops still write all 300, running 256 elements past the end
// of the allocation.
//
// 300 cannot be represented in a BYTE at all, so the only safe outcome
// is to refuse the append and leave the target as it was.
//----------------------------------------------------------------------
TEST(TArray, AppendRefusesWhenCombinedSizeOverflowsSizeType)
{
	TArray<int, BYTE>	target(200);

	for (int i = 0; i < 200; i++)
		target[(BYTE)i] = i;

	TArray<int, BYTE>	source(100);

	for (int i = 0; i < 100; i++)
		source[(BYTE)i] = 1000 + i;

	target += source;

	CHECK_EQ(200, target.GetSize());

	// The original contents must still be intact.
	CHECK_EQ(0, target[0]);
	CHECK_EQ(100, target[100]);
	CHECK_EQ(199, target[(BYTE)199]);
}

//----------------------------------------------------------------------
// The boundary case that still fits: 255 elements in a BYTE.
//----------------------------------------------------------------------
TEST(TArray, AppendAcceptsLargestRepresentableSize)
{
	TArray<int, BYTE>	target(200);

	for (int i = 0; i < 200; i++)
		target[(BYTE)i] = i;

	TArray<int, BYTE>	source(55);

	for (int i = 0; i < 55; i++)
		source[(BYTE)i] = 1000 + i;

	target += source;

	CHECK_EQ(255, target.GetSize());
	CHECK_EQ(0, target[0]);
	CHECK_EQ(199, target[(BYTE)199]);
	CHECK_EQ(1000, target[(BYTE)200]);
	CHECK_EQ(1054, target[(BYTE)254]);
}

//----------------------------------------------------------------------
// The copy must survive the original being destroyed first, which is the
// case that turns shared ownership into a dangling read.
//----------------------------------------------------------------------
TEST(TArray, CopyOutlivesOriginal)
{
	TArray<int, WORD>	copy;

	{
		TArray<int, WORD>	original(2);

		original[0] = 41;
		original[1] = 42;

		TArray<int, WORD>	temporary(original);

		copy = temporary;
	}

	CHECK_EQ(2, copy.GetSize());
	CHECK_EQ(41, copy[0]);
	CHECK_EQ(42, copy[1]);
}
