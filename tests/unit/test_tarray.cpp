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
