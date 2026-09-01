//////////////////////////////////////////////////////////////////////////////
// Filename    : PacketHandlerRegistry.cpp
// Description : see PacketHandlerRegistry.h. One DE_REGISTER line per
//               migrated packet; a packet whose execute() has been
//               deleted (ratchet R2) MUST be registered here, or
//               receiving it throws InvalidProtocolException from the
//               Packet base - a mis-migration surfaces as a disconnect
//               in the very first live run, not as a silent no-op.
//////////////////////////////////////////////////////////////////////////////

#include "Client_PCH.h"
#include "PacketHandlerRegistry.h"

#include "PacketDispatcher.h"

// Slice 1 (task 2.2): the shop/stash/say/guild families - the
// highest-risk parsers named by the code-health review, migrated first.
#include "Gpackets/GCGuildChat.h"
#include "Gpackets/GCGuildMemberList.h"
#include "Gpackets/GCGuildResponse.h"
#include "Gpackets/GCSay.h"
#include "Gpackets/GCShopBought.h"
#include "Gpackets/GCShopBuyFail.h"
#include "Gpackets/GCShopBuyOK.h"
#include "Gpackets/GCShopList.h"
#include "Gpackets/GCShopListMysterious.h"
#include "Gpackets/GCShopMarketCondition.h"
#include "Gpackets/GCShopSellFail.h"
#include "Gpackets/GCShopSellOK.h"
#include "Gpackets/GCShopSold.h"
#include "Gpackets/GCShopVersion.h"
#include "Gpackets/GCStashList.h"
#include "Gpackets/GCStashSell.h"

void registerClientPacketHandlers()
{
	// InitGame() is the only caller today, but registration on a filled
	// slot asserts, so stay safe against a second initialisation pass.
	static bool bRegistered = false;
	if (bRegistered)
		return;
	bRegistered = true;

	DE_REGISTER_PACKET_HANDLER(GCGuildChat);
	DE_REGISTER_PACKET_HANDLER(GCGuildMemberList);
	DE_REGISTER_PACKET_HANDLER(GCGuildResponse);
	DE_REGISTER_PACKET_HANDLER(GCSay);
	DE_REGISTER_PACKET_HANDLER(GCShopBought);
	DE_REGISTER_PACKET_HANDLER(GCShopBuyFail);
	DE_REGISTER_PACKET_HANDLER(GCShopBuyOK);
	DE_REGISTER_PACKET_HANDLER(GCShopList);
	DE_REGISTER_PACKET_HANDLER(GCShopListMysterious);
	DE_REGISTER_PACKET_HANDLER(GCShopMarketCondition);
	DE_REGISTER_PACKET_HANDLER(GCShopSellFail);
	DE_REGISTER_PACKET_HANDLER(GCShopSellOK);
	DE_REGISTER_PACKET_HANDLER(GCShopSold);
	DE_REGISTER_PACKET_HANDLER(GCShopVersion);

	// The two stash packets' deleted execute() additionally wrapped the
	// handler in __BEGIN_DEBUG/__END_DEBUG. That pair is a no-op on this
	// build (__WIN32__), but the cout-logging branch exists on other
	// platforms, so the explicit thunks preserve it exactly - the same
	// call the server repo made for its CGStashList.
	{
		struct Thunk {
			static void call(Packet* pPacket, Player* pPlayer)
			{
				__BEGIN_DEBUG
				GCStashListHandler::execute(static_cast<GCStashList*>(pPacket), pPlayer);
				__END_DEBUG
			}
		};
		PacketDispatcher::registerHandler(GCStashList().getPacketID(), &Thunk::call);
	}
	{
		struct Thunk {
			static void call(Packet* pPacket, Player* pPlayer)
			{
				__BEGIN_DEBUG
				GCStashSellHandler::execute(static_cast<GCStashSell*>(pPacket), pPlayer);
				__END_DEBUG
			}
		};
		PacketDispatcher::registerHandler(GCStashSell().getPacketID(), &Thunk::call);
	}
}
