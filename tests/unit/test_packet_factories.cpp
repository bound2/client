//----------------------------------------------------------------------
// test_packet_factories.cpp
//----------------------------------------------------------------------
//
// The packet classes, the factory manager and the validator compile
// into packetwire since docs/RESTRUCTURING.md task 2.4, and this test
// is the link-level proof that the library stands on its own:
// PacketFactoryManager::init() news every factory the client registers,
// each factory's vtable references its packet's constructor and vtable,
// so constructing one manager pulls every packet object in the archive
// into this test binary. A packet source that still reached into the
// executable (a game global, a handler body, a debug facility) would
// fail this binary's link, not just a grep.
//
// The behavioral checks are deliberately thin: the wire layout is
// pinned elsewhere (test_wire_layout.cpp, test_packet_goldens.cpp).
//
// Compiled with the packetwire defines (tests/CMakeLists.txt).
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "Packet.h"
#include "PacketFactoryManager.h"
#include "Exception.h"

#include "Gpackets/GCSay.h"
#include "Lpackets/LCLoginOK.h"
#include "Upackets/UCRequestLoginMode.h"

namespace {

template <class PacketT>
bool	CreatesA(PacketFactoryManager& manager, PacketID_t id)
{
	Packet* pPacket = manager.createPacket(id);
	const bool bOk = pPacket != NULL
		&& dynamic_cast<PacketT*>(pPacket) != NULL
		&& pPacket->getPacketID() == id;
	delete pPacket;
	return bOk;
}

bool	Rejects(PacketFactoryManager& manager, PacketID_t id)
{
	try {
		Packet* pPacket = manager.createPacket(id);
		delete pPacket;
	} catch (InvalidProtocolException&) {
		return true;
	}
	return false;
}

} // namespace

//----------------------------------------------------------------------
// One manager, every registered factory constructed: the link proof.
// A packet from each direction the client receives comes back as the
// right concrete type under its own id.
//----------------------------------------------------------------------
TEST(PacketFactoryManager, InitRegistersTheReceivedDirectionsAndLinksWithoutTheGame)
{
	PacketFactoryManager manager;
	manager.init();

	CHECK(CreatesA<GCSay>(manager, Packet::PACKET_GC_SAY));
	CHECK(CreatesA<LCLoginOK>(manager, Packet::PACKET_LC_LOGIN_OK));
	CHECK(CreatesA<UCRequestLoginMode>(manager, Packet::PACKET_UC_REQUEST_LOGIN_MODE));
}

//----------------------------------------------------------------------
// The manager's max-size table is what PacketValidator sizes the read
// against; it must be the factory's own answer, not a copy that can
// drift.
//----------------------------------------------------------------------
TEST(PacketFactoryManager, MaxSizeComesFromTheFactory)
{
	PacketFactoryManager manager;
	manager.init();

	CHECK_EQ(GCSayFactory().getPacketMaxSize(),
		 manager.getPacketMaxSize(Packet::PACKET_GC_SAY));
	CHECK_EQ(LCLoginOKFactory().getPacketMaxSize(),
		 manager.getPacketMaxSize(Packet::PACKET_LC_LOGIN_OK));
}

//----------------------------------------------------------------------
// The client registers only what it receives. CG packets are what it
// SENDS - it constructs them directly, never through the manager - so
// their factories stay unregistered here, and an incoming CG id (which
// only a misbehaving peer would send) is refused with the protocol
// exception rather than served. An id past the table is refused the
// same way, not indexed.
//----------------------------------------------------------------------
TEST(PacketFactoryManager, UnregisteredAndOutOfRangeIdsAreRefused)
{
	PacketFactoryManager manager;
	manager.init();

	CHECK(Rejects(manager, Packet::PACKET_CG_MOVE));
	CHECK(Rejects(manager, Packet::PACKET_MAX));
	CHECK(Rejects(manager, (PacketID_t)0xFFFF));
}
