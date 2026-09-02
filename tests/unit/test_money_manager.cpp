//----------------------------------------------------------------------
// test_money_manager.cpp
//----------------------------------------------------------------------
//
// MMoneyManager (gamemodel, docs/RESTRUCTURING.md task 4.2): a wallet
// with a limit. The contract under test is the limit - no balance
// below zero, none above the limit, and CanAddMoney/CanUseMoney
// answering for the BALANCE the operation would leave, which is what
// the trade manager asks before it moves money - plus the storage
// hint, which the executable receives through a hook.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "MMoneyManager.h"

namespace {

int	g_HintCount = 0;
void	CountHint()	{ g_HintCount++; }

} // namespace

TEST(MoneyManager, StartsEmptyWithTheTwoBillionLimit)
{
	MMoneyManager wallet;
	CHECK_EQ(0, wallet.GetMoney());
	CHECK_EQ(2000000000, wallet.GetMoneyLimit());
	CHECK_EQ(2000000000, wallet.GetMaxAddMoney());
}

TEST(MoneyManager, SetMoneyRefusesNegativeAndOverTheLimit)
{
	MMoneyManager wallet;
	wallet.SetMoneyLimit(1000);

	CHECK(wallet.SetMoney(1000));
	CHECK_EQ(1000, wallet.GetMoney());
	CHECK(!wallet.SetMoney(1001));
	CHECK_EQ(1000, wallet.GetMoney());	// a refused set leaves the balance
	CHECK(!wallet.SetMoney(-1));
	CHECK_EQ(1000, wallet.GetMoney());
	CHECK(wallet.SetMoney(0));
	CHECK_EQ(0, wallet.GetMoney());
}

TEST(MoneyManager, AddAndUseMoveTheBalanceWithinTheLimit)
{
	MMoneyManager wallet;
	wallet.SetMoneyLimit(1000);

	CHECK(wallet.AddMoney(600));
	CHECK(wallet.AddMoney(400));
	CHECK_EQ(1000, wallet.GetMoney());
	CHECK(!wallet.AddMoney(1));
	CHECK_EQ(1000, wallet.GetMoney());

	CHECK(wallet.UseMoney(999));
	CHECK_EQ(1, wallet.GetMoney());
	CHECK(!wallet.UseMoney(2));
	CHECK_EQ(1, wallet.GetMoney());
	CHECK(wallet.UseMoney(1));
	CHECK_EQ(0, wallet.GetMoney());
}

TEST(MoneyManager, CanUseMoneyAnswersForTheBalanceItWouldLeave)
{
	MMoneyManager wallet;
	CHECK(wallet.SetMoney(50));
	CHECK(wallet.CanUseMoney(50));
	CHECK(!wallet.CanUseMoney(51));
	CHECK(wallet.CanUseMoney(0));
}

//----------------------------------------------------------------------
// The storage hint: once per wallet, on the first balance past
// 100,000, only where a hook is installed. A copy keeps the "already
// hinted" state and drops the hook.
//----------------------------------------------------------------------
TEST(MoneyManager, StorageHintFiresOncePastTheThresholdThroughTheHook)
{
	g_HintCount = 0;
	MMoneyManager wallet;
	wallet.SetStorageHintHook(CountHint);

	CHECK(wallet.SetMoney(100000));
	CHECK_EQ(0, g_HintCount);		// not past the threshold yet
	CHECK(wallet.SetMoney(100001));
	CHECK_EQ(1, g_HintCount);
	CHECK(wallet.SetMoney(0));
	CHECK(wallet.SetMoney(500000));
	CHECK_EQ(1, g_HintCount);		// once per wallet

	MMoneyManager copy(wallet);
	CHECK(copy.SetMoney(900000));
	CHECK_EQ(1, g_HintCount);		// the copy carries the state, not the hook
}

TEST(MoneyManager, NoHookMeansNoHintAndNoCrash)
{
	g_HintCount = 0;
	MMoneyManager wallet;			// a trade's temporary wallet
	CHECK(wallet.SetMoney(1000000));
	CHECK_EQ(0, g_HintCount);
}
