//////////////////////////////////////////////////////////////////////////////
// Filename    : PacketDispatcher.h
// Description : packet-id -> handler dispatch table, filled in at the
//               executable's composition root (docs/RESTRUCTURING.md task
//               2.1). Replaces the per-packet virtual execute(): the
//               packetwire library keeps the wire classes, the executable
//               owns which handler runs. Ported from the server
//               repository's identical, battle-tested design (its task
//               2.3), with one addition: tryDispatch, the transitional
//               entry the receive loops use while packets are migrated
//               direction-by-direction - it reports an unregistered id
//               instead of throwing, so the caller can fall back to the
//               legacy virtual. When ratchet R2 reaches 0, the loops
//               switch to dispatch() and tryDispatch is deleted.
//////////////////////////////////////////////////////////////////////////////

#ifndef __PACKET_DISPATCHER_H__
#define __PACKET_DISPATCHER_H__

#include "Packet.h"
#include "Types.h"

class Player;

class PacketDispatcher {
public:
	typedef void (*HandlerFn)(Packet* pPacket, Player* pPlayer);

	// Call at the composition root only, before any packet is received.
	// Asserts on an out-of-range id and on double registration.
	static void registerHandler(PacketID_t packetID, HandlerFn fn);

	// Runs the registered handler for the packet's id; receiving an id
	// with no registered handler is a protocol error and throws
	// InvalidProtocolException. The table is written only during startup,
	// so dispatch needs no locking.
	static void dispatch(Packet* pPacket, Player* pPlayer);

	// Transitional (see the file header): runs the handler and returns
	// true if one is registered; returns false - never throws - for an
	// unmigrated id, so the receive loops can fall back to
	// pPacket->execute().
	static bool tryDispatch(Packet* pPacket, Player* pPlayer);

private:
	static HandlerFn s_Handlers[];
};

// Registration helpers for the composition root: bind packet class Cls
// to Cls##Handler::execute, preserving the exact call the packet's own
// execute() used to make - including the __BEGIN_TRY/__END_CATCH frame
// every deleted execute() carried, so a Throwable rethrown out of a
// handler still picks up one stack-annotation entry per packet in
// builds where that macro pair is live. The _NOPLAYER form is for
// handlers that take only the packet.
#define DE_REGISTER_PACKET_HANDLER(Cls)                                       \
	{                                                                         \
		struct Thunk {                                                        \
			static void call(Packet* pPacket, Player* pPlayer) {              \
				__BEGIN_TRY                                                   \
				Cls##Handler::execute(static_cast<Cls*>(pPacket), pPlayer);   \
				__END_CATCH                                                   \
			}                                                                 \
		};                                                                    \
		PacketDispatcher::registerHandler(Cls().getPacketID(), &Thunk::call); \
	}

#define DE_REGISTER_PACKET_HANDLER_NOPLAYER(Cls)                              \
	{                                                                         \
		struct Thunk {                                                        \
			static void call(Packet* pPacket, Player*) {                      \
				__BEGIN_TRY                                                   \
				Cls##Handler::execute(static_cast<Cls*>(pPacket));            \
				__END_CATCH                                                   \
			}                                                                 \
		};                                                                    \
		PacketDispatcher::registerHandler(Cls().getPacketID(), &Thunk::call); \
	}

#endif
