//----------------------------------------------------------------------
// test_item_core.cpp
//----------------------------------------------------------------------
//
// The item model (MItem, docs/RESTRUCTURING.md task 4.4) after its
// split from the executable's use handlers. These pin the behaviour
// the split had to preserve: the option list, the requirement math
// over the item and option tables, quest detection through the flag
// and the timed-item register, the colour cycles the host's animation
// clock drives, the drop animation the host's frame pack sizes, and
// the teen-build skull naming through the string table. A test binary
// installs its own MItemHost, or none; the executable installs the
// real one in GameInit.cpp.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "MItem.h"
#include "MItemTable.h"
#include "MItemOptionTable.h"
#include "MItemLimits.h"
#include "MGameStringTable.h"
#include "MStringArray.h"
#include "UserInformation.h"
#include "ClientConfig.h"
#include "MTimeItemManager.h"
#include "ItemClassDef.h"
#include "RaceType.h"

#include <cstring>

namespace {

//----------------------------------------------------------------------
// The globals MItem reads, owned by one fixture per test so no test
// sees another's tables.
//----------------------------------------------------------------------
struct ItemWorld
{
	ItemWorld()
	{
		g_pItemTable = new ITEMCLASS_TABLE;
		g_pItemTable->Init(MAX_ITEM_CLASS);
		g_pItemTable->InitClass(ITEM_CLASS_SWORD, 2);
		g_pItemTable->InitClass(ITEM_CLASS_SKULL, 1);
		g_pItemTable->InitClass(ITEM_CLASS_PET_ITEM, 1);

		// Option 0 is the "no option" row; 1 and 2 carry requirements.
		g_pItemOptionTable = new ITEMOPTION_TABLE;
		g_pItemOptionTable->Init(3);
		g_pItemOptionTable->Get(0).ColorSet = 0;
		g_pItemOptionTable->Get(1).RequireSUM = 10;
		g_pItemOptionTable->Get(1).ColorSet = 501;
		g_pItemOptionTable->Get(2).RequireSUM = 25;
		g_pItemOptionTable->Get(2).ColorSet = 502;

		g_pGameStringTable = new MStringArray;
		g_pGameStringTable->Init(STRING_MESSAGE_SOUL_STONE + 1);
		(*g_pGameStringTable)[STRING_MESSAGE_SOUL_STONE] = "Soul Stone";

		g_pUserInformation = new UserInformation;
		g_pUserInformation->GoreLevel = true;

		g_pClientConfig = new ClientConfig;
		g_pClientConfig->UniqueItemColorSet = 405;
		g_pClientConfig->QuestItemColorSet = 345;

		// The executable creates the register at start-up, before any item.
		g_pTimeItemManager = new MTimeItemManager;
		MItem::SetHost(NULL);
	}

	~ItemWorld()
	{
		MItem::SetHost(NULL);
		delete g_pTimeItemManager;	g_pTimeItemManager = NULL;
		delete g_pClientConfig;		g_pClientConfig = NULL;
		delete g_pUserInformation;	g_pUserInformation = NULL;
		delete g_pGameStringTable;	g_pGameStringTable = NULL;
		delete g_pItemOptionTable;	g_pItemOptionTable = NULL;
		delete g_pItemTable;		g_pItemTable = NULL;
	}
};

// MItem's own virtuals are all defined in the library, so a test item
// only has to name its class.
struct Sword : public MItem
{
	ITEM_CLASS	GetItemClass() const	{ return ITEM_CLASS_SWORD; }
};

struct Skull : public MItem
{
	ITEM_CLASS	GetItemClass() const	{ return ITEM_CLASS_SKULL; }
};

struct PetItem : public MItem
{
	ITEM_CLASS	GetItemClass() const	{ return ITEM_CLASS_PET_ITEM; }
};

ITEMTABLE_INFO&	SwordInfo(int type = 0)
{
	return (*g_pItemTable)[ITEM_CLASS_SWORD][type];
}

//----------------------------------------------------------------------
// A host the tests drive by hand.
//----------------------------------------------------------------------
DWORD	s_Frame = 0;
int		s_DropFrames = 0;
int		s_RefreshCalls = 0;

int		DropFrameCount(TYPE_FRAMEID)	{ return s_DropFrames; }
void	RefreshPetAffect(MItem*)		{ s_RefreshCalls++; }

const MItemHost	s_Host = { &s_Frame, DropFrameCount, RefreshPetAffect };

void	InstallHost()
{
	s_Frame = 0;
	s_DropFrames = 0;
	s_RefreshCalls = 0;
	MItem::SetHost(&s_Host);
}

bool	StrEq(const char* actual, const char* expected)
{
	return actual != NULL && std::strcmp(actual, expected) == 0;
}

} // namespace

//----------------------------------------------------------------------
// Construction
//----------------------------------------------------------------------
TEST(ItemCore, FreshItemStartsUnoptionedUndroppedAndUncoloured)
{
	ItemWorld world;
	Sword item;

	CHECK_EQ(1, (int)item.GetNumber());
	CHECK_EQ(0, (int)item.GetItemType());
	CHECK_EQ(0, item.GetItemOptionListCount());
	CHECK_EQ(0, (int)item.IsDropping());
	CHECK_EQ(0, item.GetSilver());
	CHECK_EQ(-1, item.GetGrade());
	CHECK_EQ(0, (int)item.GetEnchantLevel());
	CHECK_EQ(0xFFFF, (int)item.GetItemColorSet());
	CHECK_EQ(0, item.IsQuestItem());
	CHECK(MItem::GetHost() == NULL);
}

//----------------------------------------------------------------------
// The option list
//----------------------------------------------------------------------
TEST(ItemCore, OptionListIgnoresZeroAndChangesInPlace)
{
	ItemWorld world;
	Sword item;

	item.AddItemOption(0);
	CHECK_EQ(0, item.GetItemOptionListCount());

	item.AddItemOption(1);
	CHECK_EQ(1, item.GetItemOptionListCount());

	// A change keeps the option's position.
	item.ChangeItemOption(1, 2);
	CHECK_EQ(1, item.GetItemOptionListCount());
	CHECK_EQ(2, (int)item.GetItemOptionList().front());

	item.AddItemOption(1);
	CHECK_EQ(2, item.GetItemOptionListCount());

	// Changing to "no option" removes the entry instead.
	item.ChangeItemOption(2, 0);
	CHECK_EQ(1, item.GetItemOptionListCount());
	CHECK_EQ(1, (int)item.GetItemOptionList().front());

	item.RemoveItemOption(1);
	CHECK_EQ(0, item.GetItemOptionListCount());

	// Removing what is not there is a no-op.
	item.RemoveItemOption(7);
	CHECK_EQ(0, item.GetItemOptionListCount());
}

//----------------------------------------------------------------------
// Requirements: the table's value plus twice the options' sum, capped
// at the old attribute ceiling unless the item is Ousters gear.
//----------------------------------------------------------------------
TEST(ItemCore, RequiredStrengthAddsOptionsAndCapsAtTheOldCeiling)
{
	ItemWorld world;
	Sword item;

	SwordInfo().SetRequireSTR(50);
	CHECK_EQ(50, item.GetRequireSTR());

	item.AddItemOption(1);			// +10 -> doubled
	CHECK_EQ(10, item.GetItemOptionRequireSUM());
	CHECK_EQ(70, item.GetRequireSTR());

	// Past the old ceiling the sum is clamped to it.
	SwordInfo().SetRequireSTR(190);
	CHECK_EQ(MAX_SLAYER_ATTR_OLD, item.GetRequireSTR());

	// Ousters gear is not clamped.
	SwordInfo().Race = FLAG_RACE_OUSTERS;
	CHECK_EQ(210, item.GetRequireSTR());
	SwordInfo().Race = 0;

	// No table requirement means none at all, options or not.
	SwordInfo().SetRequireSTR(0);
	CHECK_EQ(0, item.GetRequireSTR());

	// A quest item asks nothing either.
	SwordInfo().SetRequireSTR(50);
	item.SetQuestFlag(true);
	CHECK_EQ(0, item.GetRequireSTR());
}

//----------------------------------------------------------------------
// Quest items: the flag, or the timed-item register knowing the id.
//----------------------------------------------------------------------
TEST(ItemCore, QuestItemComesFromTheFlagOrTheTimedItemRegister)
{
	ItemWorld world;
	Sword item;
	item.SetID(77);

	CHECK_EQ(0, item.IsQuestItem());

	item.SetQuestFlag(true);
	CHECK_EQ(1, item.IsQuestItem());
	item.SetQuestFlag(false);
	CHECK_EQ(0, item.IsQuestItem());

	g_pTimeItemManager->AddTimeItem(77, 60);
	CHECK_EQ(1, item.IsQuestItem());
}

// The flag is the item's own; it must not need the register to exist.
TEST(ItemCore, QuestFlagCountsWithoutTheTimedItemRegister)
{
	ItemWorld world;
	delete g_pTimeItemManager;
	g_pTimeItemManager = NULL;

	Sword item;
	item.SetQuestFlag(true);
	CHECK_EQ(1, item.IsQuestItem());
}

//----------------------------------------------------------------------
// Colour cycles: the grade walks 0..14 and back over 28 frames of the
// host's clock; without a host the clock reads 0.
//----------------------------------------------------------------------
TEST(ItemCore, ColourSetsCycleWithTheHostClock)
{
	ItemWorld world;

	CHECK_EQ(405, MItem::GetUniqueItemColorset());
	CHECK_EQ(345, MItem::GetQuestItemColorset());

	InstallHost();
	s_Frame = 0;
	CHECK_EQ(405, MItem::GetUniqueItemColorset());
	s_Frame = 14;
	CHECK_EQ(419, MItem::GetUniqueItemColorset());
	CHECK_EQ(359, MItem::GetQuestItemColorset());
	s_Frame = 20;
	CHECK_EQ(413, MItem::GetUniqueItemColorset());
	s_Frame = 28;
	CHECK_EQ(405, MItem::GetUniqueItemColorset());
	s_Frame = 27;
	CHECK_EQ(406, MItem::GetUniqueItemColorset());
}

TEST(ItemCore, SpecialColourPrefersQuestThenUnique)
{
	ItemWorld world;
	Sword item;

	CHECK_EQ(0xffff, item.GetSpecialColorItemColorset());

	SwordInfo().ItemStyle = 1;		// unique
	CHECK_EQ(405, item.GetSpecialColorItemColorset());

	item.SetQuestFlag(true);		// quest wins
	CHECK_EQ(345, item.GetSpecialColorItemColorset());

	CHECK_EQ(405, MItem::GetSpecialColorItemColorset(UNIQUE_ITEM_COLOR));
	CHECK_EQ(345, MItem::GetSpecialColorItemColorset(QUEST_ITEM_COLOR));
	CHECK_EQ(0xffff, MItem::GetSpecialColorItemColorset(7));
}

//----------------------------------------------------------------------
// The drop animation: sized by the host's frame pack, run for
// MAX_DROP_COUNT steps, then parked on the last frame.
//----------------------------------------------------------------------
TEST(ItemCore, DroppingRunsTheHostSizedAnimationAndParks)
{
	ItemWorld world;
	Sword item;
	SwordInfo().SetDropFrameID(14);

	// No host: nothing to animate, so the item never starts dropping.
	item.SetDropping();
	CHECK_EQ(0, (int)item.IsDropping());

	InstallHost();
	s_DropFrames = 4;
	item.SetDropping();
	CHECK_EQ(1, (int)item.IsDropping());
	CHECK_EQ(4, (int)item.GetMaxFrame());
	CHECK_EQ(0, (int)item.GetFrame());
	CHECK_EQ(36, item.GetDropHeight());

	for (int i = 0; i < MAX_DROP_COUNT - 1; i++)
	{
		item.NextDropFrame();
		CHECK_EQ(1, (int)item.IsDropping());
	}
	CHECK_EQ(4, item.GetDropHeight());

	item.NextDropFrame();
	CHECK_EQ(0, (int)item.IsDropping());
	CHECK_EQ(3, (int)item.GetFrame());
	CHECK_EQ(4, item.GetDropHeight());	// the count backs off one step
}

TEST(ItemCore, NoDropFrameMeansNoDrop)
{
	ItemWorld world;
	Sword item;
	SwordInfo().SetDropFrameID(FRAMEID_NULL);

	InstallHost();
	s_DropFrames = 4;
	item.SetDropping();
	CHECK_EQ(0, (int)item.IsDropping());
}

//----------------------------------------------------------------------
// Names: the teen build shows a skull's "<X> Head" as "<X> Soul Stone".
//----------------------------------------------------------------------
TEST(ItemCore, SkullNameBecomesSoulStoneWhenGoreIsOff)
{
	ItemWorld world;
	(*g_pItemTable)[ITEM_CLASS_SKULL][0].HName = "Wolf Head";

	{
		Skull gory;
		CHECK(StrEq(gory.GetName(), "Wolf Head"));
	}

	g_pUserInformation->GoreLevel = false;
	{
		Skull teen;
		CHECK(StrEq(teen.GetName(), "Wolf Soul Stone"));
		// The substituted name is cached on the item.
		CHECK(StrEq(teen.GetName(), "Wolf Soul Stone"));
	}

	// A name too short to carry the noun is left alone.
	(*g_pItemTable)[ITEM_CLASS_SKULL][0].HName = "Orb";
	{
		Skull teen;
		CHECK(StrEq(teen.GetName(), "Orb"));
	}
}

TEST(ItemCore, SwordNameIsTheTableName)
{
	ItemWorld world;
	SwordInfo().HName = "Long Sword";
	SwordInfo().EName = "Longsword";

	Sword item;
	CHECK(StrEq(item.GetName(), "Long Sword"));
	CHECK(StrEq(item.GetEName(), "Longsword"));
}

//----------------------------------------------------------------------
// The pet affect refresh goes through the host before the colour read.
//----------------------------------------------------------------------
TEST(ItemCore, PetColourReadRefreshesTheAffectThroughTheHost)
{
	ItemWorld world;
	PetItem pet;

	// No host: the read still answers.
	pet.GetItemOptionColorSet();

	InstallHost();
	pet.GetItemOptionColorSet();
	CHECK_EQ(1, s_RefreshCalls);

	// A sword is not a pet; the host is not asked.
	Sword sword;
	sword.GetItemOptionColorSet();
	CHECK_EQ(1, s_RefreshCalls);
}

TEST(ItemCore, OptionColourSetPicksTheNthOptionOrTheNoneRow)
{
	ItemWorld world;
	Sword item;

	CHECK_EQ(0, (int)item.GetItemOptionColorSet());		// row 0 of an Init'd table

	item.AddItemOption(1);
	item.AddItemOption(2);
	CHECK_EQ(501, (int)item.GetItemOptionColorSet(0));
	CHECK_EQ(502, (int)item.GetItemOptionColorSet(1));
	CHECK_EQ(0, (int)item.GetItemOptionColorSet(2));	// past the list: the none row

	item.SetQuestFlag(true);
	CHECK_EQ(QUEST_ITEM_COLOR, (int)item.GetItemOptionColorSet());
}
