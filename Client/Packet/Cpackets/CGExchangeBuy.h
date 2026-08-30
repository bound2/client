//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExchangeBuy.h
// Written By  : Exchange System
// Description : Client buys an item from the exchange
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_EXCHANGE_BUY_H__
#define __CG_EXCHANGE_BUY_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGExchangeBuy;
//////////////////////////////////////////////////////////////////////////////

class CGExchangeBuy : public Packet
{
public:
	CGExchangeBuy();
	virtual ~CGExchangeBuy();

	void read(SocketInputStream& iStream);
	void write(SocketOutputStream& oStream) const;
	void execute(Player* pPlayer);

	PacketSize_t getPacketSize() const { return sizeof(ulonglong) + sizeof(BYTE) + m_IdempotencyKey.length(); }
	PacketID_t getPacketID() const { return PACKET_CG_EXCHANGE_BUY; }
	string getPacketName() const { return "CGExchangeBuy"; }
	string toString() const;

	// Getters
	ulonglong getListingID() const { return m_ListingID; }
	const string& getIdempotencyKey() const { return m_IdempotencyKey; }

	// Setters
	void setListingID(ulonglong listingID) { m_ListingID = listingID; }
	void setIdempotencyKey(const string& idempotencyKey) { m_IdempotencyKey = idempotencyKey; }

private:
	ulonglong	m_ListingID;
	string		m_IdempotencyKey;
};

//////////////////////////////////////////////////////////////////////////////
// class CGExchangeBuyFactory;
//////////////////////////////////////////////////////////////////////////////

class CGExchangeBuyFactory : public PacketFactory
{
public:
	Packet* createPacket() throw() { return new CGExchangeBuy(); }
	string getPacketName() const throw() { return "CGExchangeBuy"; }
	PacketID_t getPacketID() const throw() { return Packet::PACKET_CG_EXCHANGE_BUY; }
	PacketSize_t getPacketMaxSize() const throw() { return 8 + 1 + 255; }
};

#endif
