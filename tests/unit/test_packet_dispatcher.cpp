//----------------------------------------------------------------------
// test_packet_dispatcher.cpp
//----------------------------------------------------------------------
//
// PacketDispatcher (docs/RESTRUCTURING.md task 2.1): the id -> handler
// table that replaces the per-packet virtual execute(). These tests pin
// its contract before any packet migrates onto it:
//
//   - a registered handler runs with the packet and player it was given;
//   - tryDispatch reports (never throws) an unregistered id, because the
//     receive loops fall back to the legacy virtual during migration;
//   - dispatch throws InvalidProtocolException for an unregistered or
//     out-of-range id - after migration that is a protocol violation;
//   - double registration is refused (AssertionError), because two
//     handlers silently fighting over one id is how a mis-merge ships.
//
// The table is process-global and written only at startup, so each test
// registers a DISTINCT id (picked from the top of the valid range,
// where no real packet migration will land first) and never
// re-registers one used by another test.
//
// Compiles with the packetwire library's own defines - see
// tests/CMakeLists.txt.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "PacketDispatcher.h"
#include "Exception.h"

namespace {

//----------------------------------------------------------------------
// Minimal concrete packet carrying an arbitrary id.
//----------------------------------------------------------------------
class TestPacket : public Packet
{
public:
	explicit TestPacket(PacketID_t id) : m_ID(id) { }

	void read(SocketInputStream&) { }
	void write(SocketOutputStream&) const { }
	void execute(Player*) { }
	PacketID_t getPacketID() const  { return m_ID; }
	PacketSize_t getPacketSize() const  { return 0; }

private:
	PacketID_t m_ID;
};

// What the last handler invocation saw.
Packet* g_pSeenPacket = 0;
Player* g_pSeenPlayer = 0;
int     g_nCalls      = 0;

void recordingHandler(Packet* pPacket, Player* pPlayer)
{
	g_pSeenPacket = pPacket;
	g_pSeenPlayer = pPlayer;
	g_nCalls++;
}

// Distinct ids per test, from the top of the valid range.
const PacketID_t ID_DISPATCH   = Packet::PACKET_MAX - 1;
const PacketID_t ID_TRY        = Packet::PACKET_MAX - 2;
const PacketID_t ID_DOUBLE     = Packet::PACKET_MAX - 3;
const PacketID_t ID_UNMIGRATED = Packet::PACKET_MAX - 4;	// never registered

} // namespace

TEST(PacketDispatcher, DispatchRunsTheRegisteredHandler)
{
	PacketDispatcher::registerHandler(ID_DISPATCH, &recordingHandler);

	TestPacket packet(ID_DISPATCH);
	Player* pPlayer = (Player*)0;	// the dispatcher passes it through untouched

	int callsBefore = g_nCalls;
	PacketDispatcher::dispatch(&packet, pPlayer);

	CHECK_EQ(callsBefore + 1, g_nCalls);
	CHECK(g_pSeenPacket == &packet);
	CHECK(g_pSeenPlayer == pPlayer);
}

TEST(PacketDispatcher, TryDispatchRunsAndReportsRegistered)
{
	PacketDispatcher::registerHandler(ID_TRY, &recordingHandler);

	TestPacket packet(ID_TRY);
	int callsBefore = g_nCalls;

	CHECK(PacketDispatcher::tryDispatch(&packet, 0));
	CHECK_EQ(callsBefore + 1, g_nCalls);
}

TEST(PacketDispatcher, TryDispatchReportsUnmigratedWithoutThrowing)
{
	TestPacket packet(ID_UNMIGRATED);
	int callsBefore = g_nCalls;

	CHECK_EQ(false, PacketDispatcher::tryDispatch(&packet, 0));
	CHECK_EQ(callsBefore, g_nCalls);
}

TEST(PacketDispatcher, DispatchThrowsForUnregisteredId)
{
	TestPacket packet(ID_UNMIGRATED);

	bool bThrew = false;
	try {
		PacketDispatcher::dispatch(&packet, 0);
	} catch (InvalidProtocolException&) {
		bThrew = true;
	}
	CHECK(bThrew);
}

TEST(PacketDispatcher, DoubleRegistrationIsRefused)
{
	PacketDispatcher::registerHandler(ID_DOUBLE, &recordingHandler);

	bool bThrew = false;
	try {
		PacketDispatcher::registerHandler(ID_DOUBLE, &recordingHandler);
	} catch (AssertionError&) {
		bThrew = true;
	}
	CHECK(bThrew);
}
