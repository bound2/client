//----------------------------------------------------------------------
// test_ctypepack.cpp
//----------------------------------------------------------------------
//
// Tests for CTypePack in Client/SpriteLib/CTypePack.h.
//
// Get() returns a reference, so it has no way to report a bad index to
// the caller. That makes the range check its only defence: without one
// it indexes straight off the end of the array, or through a null
// pointer when the pack was never initialised.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "CTypePack.h"

namespace {

//----------------------------------------------------------------------
// Smallest element type satisfying what CTypePack's members require.
//----------------------------------------------------------------------
class PackElement
{
public:
	PackElement() : m_Value(0), m_bInit(false) {}

	bool	IsInit() const		{ return m_bInit; }

	void	Release()		{ m_bInit = false; m_Value = 0; }

	bool	LoadFromFile(std::ifstream& /*file*/)	{ m_bInit = true; return true; }
	bool	SaveToFile(std::ofstream& /*file*/)	{ return true; }

	void	SetValue(int value)	{ m_Value = value; m_bInit = true; }
	int	GetValue() const	{ return m_Value; }

private:
	int	m_Value;
	bool	m_bInit;
};

} // namespace

//----------------------------------------------------------------------
// Indexing inside the pack behaves normally.
//----------------------------------------------------------------------
TEST(CTypePack, GetReturnsElementsWithinRange)
{
	CTypePack<PackElement>	pack;

	pack.Init(4);

	CHECK_EQ(4, pack.GetSize());

	pack[0].SetValue(11);
	pack[3].SetValue(44);

	CHECK_EQ(11, pack.Get(0).GetValue());
	CHECK_EQ(44, pack.Get(3).GetValue());
}

//----------------------------------------------------------------------
// An index past the end must not read off the end of the array.
//
// Get indexed m_pData[n] with nothing comparing n against m_Size, so a
// caller asking for element 5000 of a four element pack was handed a
// reference roughly 5000 elements past the allocation.
//----------------------------------------------------------------------
TEST(CTypePack, GetRejectsIndexBeyondSize)
{
	CTypePack<PackElement>	pack;

	pack.Init(4);

	PackElement&	element = pack.Get(5000);

	// The fallback element stands in for the out of range access.
	CHECK(!element.IsInit());
}

//----------------------------------------------------------------------
// A pack that was never initialised holds a null array.
//
// With no range check, Get formed m_pData[0] against a null pointer and
// the first read through the returned reference faulted.
//----------------------------------------------------------------------
TEST(CTypePack, GetOnUninitialisedPackDoesNotDereferenceNull)
{
	CTypePack<PackElement>	pack;

	CHECK_EQ(0, pack.GetSize());

	PackElement&	element = pack.Get(0);

	CHECK(!element.IsInit());
}

//----------------------------------------------------------------------
// Releasing a range must stay inside the pack.
//
// ReleasePart(first, last) clamped last to 0xFFFE and then called
// Release() on every element in between, with nothing comparing the
// index against m_Size. Asking it to release a range wider than the
// pack wrote through elements past the end of the allocation.
//----------------------------------------------------------------------
TEST(CTypePack, ReleasePartByRangeStaysInsideThePack)
{
	CTypePack<PackElement>	pack;

	pack.Init(4);

	for (WORD i = 0; i < 4; i++)
		pack[i].SetValue(10 + i);

	pack.ReleasePart(0, 100);

	CHECK_EQ(4, pack.GetSize());

	CHECK(!pack[0].IsInit());
	CHECK(!pack[3].IsInit());
}

//----------------------------------------------------------------------
// Releasing by list must release every element the list names.
//
// The loop ran GetSize() times but never advanced its iterator, so it
// released whatever the first entry pointed at over and over and left
// every other listed element alone.
//----------------------------------------------------------------------
TEST(CTypePack, ReleasePartByListReleasesEveryListedElement)
{
	CTypePack<PackElement>	pack;

	pack.Init(4);

	for (WORD i = 0; i < 4; i++)
		pack[i].SetValue(10 + i);

	COrderedList<TYPE_SPRITEID>	list;

	list.Add(1);
	list.Add(3);

	pack.ReleasePart(list);

	// Only the listed elements are released.
	CHECK(pack[0].IsInit());
	CHECK(!pack[1].IsInit());
	CHECK(pack[2].IsInit());
	CHECK(!pack[3].IsInit());
}

//----------------------------------------------------------------------
// An index in the list that falls outside the pack must be ignored
// rather than written through.
//----------------------------------------------------------------------
TEST(CTypePack, ReleasePartByListIgnoresIndexesOutsideThePack)
{
	CTypePack<PackElement>	pack;

	pack.Init(2);

	pack[0].SetValue(1);
	pack[1].SetValue(2);

	COrderedList<TYPE_SPRITEID>	list;

	list.Add(0);
	list.Add(900);

	pack.ReleasePart(list);

	CHECK_EQ(2, pack.GetSize());
	CHECK(!pack[0].IsInit());
	CHECK(pack[1].IsInit());
}

//----------------------------------------------------------------------
// The boundary index is one past the last valid element.
//----------------------------------------------------------------------
TEST(CTypePack, GetRejectsFirstIndexPastTheEnd)
{
	CTypePack<PackElement>	pack;

	pack.Init(3);

	pack[2].SetValue(99);

	CHECK_EQ(99, pack.Get(2).GetValue());

	PackElement&	element = pack.Get(3);

	CHECK(!element.IsInit());
}
