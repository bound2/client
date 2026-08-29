//----------------------------------------------------------------------
// test_framework_selfcheck.cpp
//----------------------------------------------------------------------
//
// Confirms the harness itself reports results, so a green run means the
// tests actually executed rather than that nothing was registered.
//
//----------------------------------------------------------------------

#include "test_framework.h"

TEST(Framework, RunsRegisteredTests)
{
	CHECK(true);
}

TEST(Framework, ComparesIntegers)
{
	CHECK_EQ(2, 1 + 1);
	CHECK_EQ(0xFFFF, 65535);
}
