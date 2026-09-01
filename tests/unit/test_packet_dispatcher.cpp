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
//   - double registration is refused with a real runtime throw (not an
//     Assert that vanishes under NDEBUG), because two handlers silently
//     fighting over one id is how a mis-merge ships;
//   - a migrated packet that carries no execute() override reaches the
//     Packet base default, which throws - the branch's central safety
//     claim that a mis-migration disconnects instead of no-opping.
//
// The table is process-global and written only at startup, so each test
// registers a DISTINCT id and never re-registers one used by another
// test. Any valid id is safe HERE because unit_tests does not link
// Client/PacketHandlerRegistry.cpp (it is executable-side), so nothing
// else writes this process's table - the ids are NOT "reserved" in the
// protocol (the id space is fully allocated up to PACKET_MAX).
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

// Distinct ids per test (see the file header for why any valid id is
// safe in this binary).
const PacketID_t ID_DISPATCH   = Packet::PACKET_MAX - 1;
const PacketID_t ID_TRY        = Packet::PACKET_MAX - 2;
const PacketID_t ID_DOUBLE     = Packet::PACKET_MAX - 3;
const PacketID_t ID_UNMIGRATED = Packet::PACKET_MAX - 4;	// never registered

//----------------------------------------------------------------------
// A migrated packet: no execute() override at all, so calling the
// virtual reaches the Packet base default.
//----------------------------------------------------------------------
class MigratedPacket : public Packet
{
public:
	void read(SocketInputStream&) { }
	void write(SocketOutputStream&) const { }
	PacketID_t getPacketID() const  { return ID_UNMIGRATED; }
	PacketSize_t getPacketSize() const  { return 0; }
};

} // namespace

TEST(PacketDispatcher, DispatchRunsTheRegisteredHandler)
{
	PacketDispatcher::registerHandler(ID_DISPATCH, &recordingHandler);

	TestPacket packet(ID_DISPATCH);

	// A distinct non-null sentinel, never dereferenced: comparing NULL
	// against NULL would pass even if the dispatcher dropped the player.
	int sentinel = 0;
	Player* pPlayer = reinterpret_cast<Player*>(&sentinel);

	int callsBefore = g_nCalls;
	g_pSeenPlayer = 0;
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
	} catch (Error&) {
		bThrew = true;
	}
	CHECK(bThrew);
}

TEST(PacketDispatcher, OutOfRangeIdIsRejectedEverywhere)
{
	TestPacket packet(Packet::PACKET_MAX);

	bool bThrew = false;
	try {
		PacketDispatcher::dispatch(&packet, 0);
	} catch (InvalidProtocolException&) {
		bThrew = true;
	}
	CHECK(bThrew);

	// tryDispatch reports it instead of throwing (receive loops fall
	// back to the legacy virtual).
	CHECK_EQ(false, PacketDispatcher::tryDispatch(&packet, 0));

	bThrew = false;
	try {
		PacketDispatcher::registerHandler(Packet::PACKET_MAX, &recordingHandler);
	} catch (Error&) {
		bThrew = true;
	}
	CHECK(bThrew);
}

TEST(Packet, BaseExecuteDefaultThrowsForUnregisteredMigratedPacket)
{
	// The migration's central safety property: a packet whose execute()
	// was deleted but whose id was never registered must throw when the
	// legacy virtual is reached (via the receive loops' fallback), not
	// silently no-op.
	MigratedPacket packet;

	bool bThrew = false;
	try {
		packet.execute(0);
	} catch (InvalidProtocolException&) {
		bThrew = true;
	}
	CHECK(bThrew);
}
