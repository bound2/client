//----------------------------------------------------------------------
// test_skill_core.cpp
//----------------------------------------------------------------------
//
// The skill core (docs/RESTRUCTURING.md task 4.4, fourth slice): the
// info table one entry per skill, the set of skills a character may use
// now, and the domains that hold a skill tree and hand out what the
// character learns. The half that decides what the player can use at
// this moment - the weapon in hand, the inventory, the zone - is the
// executable's MSkillAvailable.cpp and is not reachable from here; the
// use delays run on the item host's clock, which these tests drive.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "gamemodel_world.h"
#include "MSkillManager.h"
#include "SkillDef.h"

#include <cstdio>
#include <cstring>
#include <fstream>

namespace {

DWORD	s_Now = 0;

int			DropFrameCount(TYPE_FRAMEID)	{ return 0; }
void		RefreshAffect(MItem*)			{}
void		PlayItemSound(TYPE_SOUNDID)		{}
void		RecalculateStatus()				{}
void		ResetQuickItemSlot()			{}
void		RepairHint()					{}
MMagazine*	EmptyMagazineFor(MItem*)		{ return NULL; }

DWORD	s_Frame = 0;

// The clock is all the skill core wants from a host.
const MItemHost	s_Host = { &s_Frame, DropFrameCount, RefreshAffect, PlayItemSound, &s_Now,
							RecalculateStatus, ResetQuickItemSlot, RepairHint, EmptyMagazineFor };

const char* const	kTempFile = "skill_core_test.bin";

// Three skills wired into a chain: the root, what it leads to, and what
// that leads to.
const ACTIONINFO	kRoot	= SKILL_SINGLE_BLOW;
const ACTIONINFO	kChild	= SKILL_DOUBLE_IMPACT;
const ACTIONINFO	kLeaf	= SKILL_FAST_RELOAD;

struct SkillWorld : GameModelWorld
{
	SkillWorld()
	{
		s_Now = 0;

		g_pSkillInfoTable = new MSkillInfoTable;
		g_pSkillManager = new MSkillManager;
		g_pSkillAvailable = new MSkillSet;

		// A three-step chain, one level apart.
		(*g_pSkillInfoTable)[kRoot].Set(0, "Single Blow", 0, 0, 0, "Single Blow");
		(*g_pSkillInfoTable)[kChild].Set(1, "Double Impact", 1, 0, 0, "Double Impact");
		(*g_pSkillInfoTable)[kLeaf].Set(2, "Fast Reload", 2, 0, 0, "Fast Reload");
		(*g_pSkillInfoTable)[kRoot].AddNextSkill(kChild);
		(*g_pSkillInfoTable)[kChild].AddNextSkill(kLeaf);

		MItem::SetHost(&s_Host);
	}

	~SkillWorld()
	{
		delete g_pSkillAvailable;	g_pSkillAvailable = NULL;
		delete g_pSkillManager;		g_pSkillManager = NULL;
		delete g_pSkillInfoTable;	g_pSkillInfoTable = NULL;
	}
};

} // namespace

//----------------------------------------------------------------------
// One skill's entry
//----------------------------------------------------------------------
TEST(SkillInfoNode, UseDelaysRunOnTheHostClock)
{
	SkillWorld world;
	SKILLINFO_NODE node;

	// Nothing used yet: available, with nothing left to wait for.
	CHECK(node.IsAvailableTime());
	CHECK_EQ(0, (int)node.GetAvailableTimeLeft());

	// Using it starts its delay from now.
	node.SetDelayTime(3000);
	CHECK_EQ(3000, (int)node.GetDelayTime());
	s_Now = 10000;
	node.SetNextAvailableTime();
	CHECK_EQ(false, node.IsAvailableTime());
	CHECK_EQ(3000, (int)node.GetAvailableTimeLeft());
	s_Now = 12999;
	CHECK_EQ(false, node.IsAvailableTime());
	CHECK_EQ(1, (int)node.GetAvailableTimeLeft());
	s_Now = 13000;
	CHECK(node.IsAvailableTime());
	CHECK_EQ(0, (int)node.GetAvailableTimeLeft());

	// Setting it available takes a delay of its own, or none at all.
	node.SetAvailableTime(500);
	CHECK_EQ(false, node.IsAvailableTime());
	node.SetAvailableTime(0);
	CHECK(node.IsAvailableTime());

	// Without a clock there is no delay.
	node.SetNextAvailableTime();
	CHECK_EQ(false, node.IsAvailableTime());
	MItem::SetHost(NULL);
	CHECK(node.IsAvailableTime());
	CHECK_EQ(0, (int)node.GetAvailableTimeLeft());
	MItem::SetHost(&s_Host);
	CHECK_EQ(false, node.IsAvailableTime());
}

TEST(SkillInfoNode, NextSkillsSortByIdAndRefuseADuplicate)
{
	SkillWorld world;
	SKILLINFO_NODE node;

	CHECK(node.AddNextSkill((ACTIONINFO)40));
	CHECK(node.AddNextSkill((ACTIONINFO)10));
	CHECK(node.AddNextSkill((ACTIONINFO)30));
	CHECK_EQ(false, node.AddNextSkill((ACTIONINFO)30));

	const SKILLINFO_NODE::SKILLID_LIST& list = node.GetNextSkillList();
	CHECK_EQ(3, (int)list.size());
	SKILLINFO_NODE::SKILLID_LIST::const_iterator it = list.begin();
	CHECK_EQ(10, (int)*it);		++it;
	CHECK_EQ(30, (int)*it);		++it;
	CHECK_EQ(40, (int)*it);
}

TEST(SkillInfoNode, SaveAndLoadRoundTripTheServerFields)
{
	SkillWorld world;

	// A skill of any other domain carries the common fields only.
	SKILLINFO_NODE src;
	src.Set(3, "Blade Dance", 4, 5, 77, "Blade Dance");
	src.DomainType = SKILLDOMAIN_SWORD;		// not 0, so the field is really carried
	src.SetMP(42);
	src.SetLearnLevel(9);
	src.SkillPoint = 6;

	{
		std::ofstream out(kTempFile, std::ios::binary | std::ios::trunc);
		src.SaveFromFileServerSkillInfo(out);
	}

	SKILLINFO_NODE dst;
	{
		std::ifstream in(kTempFile, std::ios::binary);
		dst.LoadFromFileServerSkillInfo(in);
	}
	std::remove(kTempFile);

	CHECK_EQ(42, dst.GetMP());
	CHECK_EQ(9, dst.GetLearnLevel());
	CHECK_EQ((int)SKILLDOMAIN_SWORD, dst.DomainType);
	CHECK_EQ(0, dst.SkillPoint);		// an Ousters field, not in this row

	// An Ousters skill carries the elemental block too.
	SKILLINFO_NODE ousters;
	ousters.Set(2, "Flourish", 0, 0, 0, "Flourish");
	ousters.DomainType = SKILLDOMAIN_OUSTERS;
	ousters.SetMP(11);
	ousters.SkillPoint = 6;
	ousters.LevelUpPoint = 7;
	ousters.Fire = 1;
	ousters.Water = 2;
	ousters.Earth = 3;
	ousters.Wind = 4;
	ousters.CanDelete = 1;
	ousters.SkillTypeList.push_back(5);

	{
		std::ofstream out(kTempFile, std::ios::binary | std::ios::trunc);
		ousters.SaveFromFileServerSkillInfo(out);
	}

	SKILLINFO_NODE back;
	{
		std::ifstream in(kTempFile, std::ios::binary);
		back.LoadFromFileServerSkillInfo(in);
	}
	std::remove(kTempFile);

	CHECK_EQ(11, back.GetMP());
	CHECK_EQ(6, back.SkillPoint);
	CHECK_EQ(7, back.LevelUpPoint);
	CHECK_EQ(1, back.Fire);
	CHECK_EQ(4, back.Wind);
	CHECK_EQ(1, (int)back.CanDelete);
	CHECK_EQ(1, (int)back.SkillTypeList.size());
}

TEST(SkillInfoNode, AFreshEntrySitsNowhereInItsTree)
{
	SkillWorld world;
	SKILLINFO_NODE node;

	// A domain branches on the step, so it cannot be left to whatever
	// the memory held.
	CHECK_EQ((int)SKILL_STEP_NULL, (int)node.GetSkillStep());
	CHECK_EQ(0, node.GetX());
	CHECK_EQ(0, node.GetY());
	CHECK_EQ(0, node.GetMP());
	CHECK_EQ(0, (int)node.GetDelayTime());
	CHECK(node.IsAvailableTime());

	// The table's own entries are just as defined, and it sets the one
	// cast delay it knows by hand.
	CHECK_EQ((int)SKILL_STEP_NULL, (int)(*g_pSkillInfoTable)[kLeaf].GetSkillStep());
	CHECK_EQ(3000, (int)(*g_pSkillInfoTable)[SUMMON_HELICOPTER].GetDelayTime());
}

//----------------------------------------------------------------------
// The set of skills a character may use
//----------------------------------------------------------------------
TEST(SkillSet, HoldsSkillsByIdAndEnablesThemOneAtATime)
{
	SkillWorld world;
	MSkillSet set;

	CHECK_EQ(false, set.IsEnableSkill(kRoot));
	CHECK(set.AddSkill(kRoot));
	CHECK(set.AddSkill(kChild));
	CHECK_EQ(false, set.AddSkill(kRoot));		// already held
	CHECK_EQ(2, (int)set.size());
	CHECK(set.IsEnableSkill(kRoot));

	// Disabling keeps the skill but takes it out of use.
	CHECK(set.DisableSkill(kRoot));
	CHECK_EQ(false, set.IsEnableSkill(kRoot));
	CHECK(set.IsEnableSkill(kChild));
	CHECK_EQ(2, (int)set.size());
	CHECK(set.EnableSkill(kRoot));
	CHECK(set.IsEnableSkill(kRoot));

	// A skill it does not hold cannot be enabled or disabled.
	CHECK_EQ(false, set.EnableSkill(kLeaf));
	CHECK_EQ(false, set.DisableSkill(kLeaf));

	// Removing takes it out for good.
	CHECK(set.RemoveSkill(kRoot));
	CHECK_EQ(false, set.RemoveSkill(kRoot));
	CHECK_EQ(false, set.IsEnableSkill(kRoot));
	CHECK_EQ(1, (int)set.size());
}

//----------------------------------------------------------------------
// A domain and its tree
//----------------------------------------------------------------------
TEST(SkillDomain, TheRootSkillPullsInTheChainBelowIt)
{
	SkillWorld world;
	MSkillDomain domain;

	CHECK_EQ(0, domain.GetSize());
	domain.SetRootSkill(kRoot);

	// All three arrive: the root can be learned next, the rest cannot
	// be learned yet.
	CHECK_EQ(3, domain.GetSize());
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_NEXT, (int)domain.GetSkillStatus(kRoot));
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_OTHER, (int)domain.GetSkillStatus(kChild));
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_OTHER, (int)domain.GetSkillStatus(kLeaf));

	// A skill outside the tree is not in the domain at all.
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_NULL, (int)domain.GetSkillStatus(SKILL_FLOURISH));

	// The domain level is the server's, and a root reset zeroes it.
	CHECK_EQ(0, domain.GetDomainLevel());
	domain.SetDomainLevel(4);
	domain.SetRootSkill(kRoot);
	CHECK_EQ(0, domain.GetDomainLevel());

	CHECK_EQ(false, (bool)domain.HasNewSkill());
	domain.SetNewSkill();
	CHECK(domain.HasNewSkill());

	domain.Clear();
	CHECK_EQ(0, domain.GetSize());
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_NULL, (int)domain.GetSkillStatus(kRoot));
}

TEST(SkillDomain, LearningWalksDownTheChainAndUnlearningBackUp)
{
	SkillWorld world;
	MSkillDomain domain;
	domain.SetRootSkill(kRoot);

	// Nothing is learned without a skill point to spend.
	CHECK_EQ(false, domain.LearnSkill(kRoot));
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_NEXT, (int)domain.GetSkillStatus(kRoot));

	// With one, the root is learned and becomes usable; what follows it
	// becomes learnable.
	CHECK_EQ(false, g_pSkillAvailable->IsEnableSkill(kRoot));
	domain.SetNewSkill();
	CHECK(domain.LearnSkill(kRoot));
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_LEARNED, (int)domain.GetSkillStatus(kRoot));
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_NEXT, (int)domain.GetSkillStatus(kChild));
	CHECK(g_pSkillAvailable->IsEnableSkill(kRoot));

	// Learning spends the point, so each refusal below is given one of
	// its own: otherwise they would all stop at that first gate.
	domain.SetNewSkill();
	CHECK_EQ(false, domain.LearnSkill(kRoot));		// already learned
	domain.SetNewSkill();
	CHECK_EQ(false, domain.LearnSkill(SKILL_FLOURISH));	// not in this domain

	// Unlearning takes the deepest one back off, and out of use. The
	// child is refused for not being learned, at its own gate.
	domain.SetNewSkill();
	CHECK(domain.LearnSkill(kChild));
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_NEXT, (int)domain.GetSkillStatus(kLeaf));
	CHECK_EQ(false, domain.UnLearnSkill(kRoot));	// not the deepest any more
	CHECK(domain.UnLearnSkill(kChild));
	// Its parent is still learned, so it is learnable again rather than
	// out of reach; what followed it is out of reach.
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_NEXT, (int)domain.GetSkillStatus(kChild));
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_OTHER, (int)domain.GetSkillStatus(kLeaf));
	CHECK_EQ(false, g_pSkillAvailable->IsEnableSkill(kChild));
	CHECK(domain.UnLearnSkill(kRoot));
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_OTHER, (int)domain.GetSkillStatus(kRoot));
	CHECK_EQ(false, g_pSkillAvailable->IsEnableSkill(kRoot));
	CHECK_EQ(false, domain.UnLearnSkill(kRoot));
}

//----------------------------------------------------------------------
// The manager over the domains
//----------------------------------------------------------------------
TEST(SkillManager, InitGivesEveryDomainItsRootSkill)
{
	SkillWorld world;

	g_pSkillManager->Init();

	CHECK_EQ(MAX_SKILLDOMAIN, g_pSkillManager->GetSize());
	CHECK((*g_pSkillManager)[SKILLDOMAIN_BLADE].GetSkillStatus(SKILL_SINGLE_BLOW)
			!= MSkillDomain::SKILLSTATUS_NULL);
	CHECK((*g_pSkillManager)[SKILLDOMAIN_SWORD].GetSkillStatus(SKILL_DOUBLE_IMPACT)
			!= MSkillDomain::SKILLSTATUS_NULL);
	CHECK((*g_pSkillManager)[SKILLDOMAIN_OUSTERS].GetSkillStatus(SKILL_FLOURISH)
			!= MSkillDomain::SKILLSTATUS_NULL);

	// The blade domain took the chain the table wires under its root.
	CHECK_EQ(3, (*g_pSkillManager)[SKILLDOMAIN_BLADE].GetSize());

	// A domain holds only its own root's tree.
	CHECK_EQ((int)MSkillDomain::SKILLSTATUS_NULL,
			(int)(*g_pSkillManager)[SKILLDOMAIN_OUSTERS].GetSkillStatus(SKILL_SINGLE_BLOW));

	// The other five roots are there too.
	CHECK((*g_pSkillManager)[SKILLDOMAIN_GUN].GetSkillStatus(SKILL_FAST_RELOAD)
			!= MSkillDomain::SKILLSTATUS_NULL);
	CHECK((*g_pSkillManager)[SKILLDOMAIN_ENCHANT].GetSkillStatus(MAGIC_CREATE_HOLY_WATER)
			!= MSkillDomain::SKILLSTATUS_NULL);
	CHECK((*g_pSkillManager)[SKILLDOMAIN_HEAL].GetSkillStatus(MAGIC_CURE_LIGHT_WOUNDS)
			!= MSkillDomain::SKILLSTATUS_NULL);
	CHECK((*g_pSkillManager)[SKILLDOMAIN_VAMPIRE].GetSkillStatus(MAGIC_HIDE)
			!= MSkillDomain::SKILLSTATUS_NULL);
	CHECK((*g_pSkillManager)[SKILLDOMAIN_ETC].GetSkillStatus(SKILL_SOUL_CHAIN)
			!= MSkillDomain::SKILLSTATUS_NULL);
}

TEST(SkillManager, ADomainTheFileNamesPastTheTableIsRefused)
{
	SkillWorld world;
	g_pSkillManager->Init();

	// The domain-experience file names a domain per row, and the table
	// is indexed raw. A row naming a domain outside it stops the read
	// rather than writing through a wild pointer.
	{
		std::ofstream out(kTempFile, std::ios::binary | std::ios::trunc);
		const int rows = 2;
		const int past = MAX_SKILLDOMAIN + 4;
		const int payload = 0;
		out.write((const char*)&rows, 4);
		out.write((const char*)&past, 4);
		for (int i = 0; i < 8; i++)
			out.write((const char*)&payload, 4);
	}
	{
		std::ifstream in(kTempFile, std::ios::binary);
		g_pSkillManager->LoadFromFileServerDomainInfo(in);
	}

	// A negative one, and a row that ends before its domain does.
	{
		std::ofstream out(kTempFile, std::ios::binary | std::ios::trunc);
		const int rows = 1;
		const int negative = -3;
		out.write((const char*)&rows, 4);
		out.write((const char*)&negative, 4);
	}
	{
		std::ifstream in(kTempFile, std::ios::binary);
		g_pSkillManager->LoadFromFileServerDomainInfo(in);
	}
	{
		std::ofstream out(kTempFile, std::ios::binary | std::ios::trunc);
		const int rows = 40;
		out.write((const char*)&rows, 4);
	}
	{
		std::ifstream in(kTempFile, std::ios::binary);
		g_pSkillManager->LoadFromFileServerDomainInfo(in);
	}
	std::remove(kTempFile);

	// The tree the manager built is still the tree it built.
	CHECK_EQ(MAX_SKILLDOMAIN, g_pSkillManager->GetSize());
	CHECK_EQ(3, (*g_pSkillManager)[SKILLDOMAIN_BLADE].GetSize());
}
