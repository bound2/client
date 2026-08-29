//----------------------------------------------------------------------
// test_cfilter.cpp
//----------------------------------------------------------------------
//
// Tests for CFilter in Client/SpriteLib/CFilter.h.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "CFilter.h"

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
