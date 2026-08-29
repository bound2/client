//----------------------------------------------------------------------
// test_tarray_spritelib.cpp
//----------------------------------------------------------------------
//
// Client/SpriteLib/TArray.h is a second copy of the same global-namespace
// TArray template that lives in Client/framelib/TArray.h. Both use the
// include guard __TARRAY_H__, so a translation unit only ever sees
// whichever one it reaches first, and different translation units in the
// same program can end up with different definitions.
//
// The two copies must therefore stay behaviourally identical. This file
// duplicates the ownership checks from test_tarray.cpp against the
// SpriteLib copy; the duplication here mirrors the duplication in the
// source and should disappear with it if the copies are ever merged.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "Client/SpriteLib/TArray.h"

//----------------------------------------------------------------------
TEST(TArraySpriteLib, CopyConstructionProducesIndependentStorage)
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

	copy[0] = 999;

	CHECK_EQ(10, original[0]);
	CHECK_EQ(999, copy[0]);
}

//----------------------------------------------------------------------
TEST(TArraySpriteLib, CopyConstructionHandlesEmptyArray)
{
	TArray<int, WORD>	original;

	TArray<int, WORD>	copy(original);

	CHECK_EQ(0, copy.GetSize());
}
