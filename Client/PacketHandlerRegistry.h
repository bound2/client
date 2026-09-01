//////////////////////////////////////////////////////////////////////////////
// Filename    : PacketHandlerRegistry.h
// Description : the executable's packet-handler composition root
//               (docs/RESTRUCTURING.md task 2.2). Binds packet ids to
//               their handlers on the PacketDispatcher table; called
//               once from InitGame() beside the PacketFactoryManager
//               creation, before any connection exists.
//////////////////////////////////////////////////////////////////////////////

#ifndef __PACKET_HANDLER_REGISTRY_H__
#define __PACKET_HANDLER_REGISTRY_H__

// Idempotent: registration runs once; later calls are no-ops.
void registerClientPacketHandlers();

#endif
