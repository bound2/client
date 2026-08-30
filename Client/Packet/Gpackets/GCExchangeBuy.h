//////////////////////////////////////////////////////////////////////////////
// Filename    : GCExchangeBuy.h
// Written By  : Exchange System
// Description : Server replies to the client's exchange buy request
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_EXCHANGE_BUY_H__
#define __GC_EXCHANGE_BUY_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCExchangeBuy;
//////////////////////////////////////////////////////////////////////////////

class GCExchangeBuy : public Packet
{
public:
	GCExchangeBuy();
	virtual ~GCExchangeBuy();

	void read(SocketInputStream & iStream);
	void write(SocketOutputStream & oStream) const;
	void execute(Player* pPlayer);

	// uint8 success + bstr message (length byte always written) + uint64 orderID
	PacketSize_t getPacketSize() const throw() { return sizeof(BYTE) + sizeof(BYTE) + m_Message.length() + sizeof(ulonglong); }
	PacketID_t getPacketID() const throw() { return PACKET_GC_EXCHANGE_BUY; }
	string getPacketName() const throw() { return "GCExchangeBuy"; }
	string toString() const;

	// Getters
	bool isSuccess() const { return m_Success; }
	const string& getMessage() const { return m_Message; }
	ulonglong getOrderID() const { return m_OrderID; }

	// Setters
	void setSuccess(bool success) { m_Success = success; }
	void setMessage(const string& message) { m_Message = message; }
	void setOrderID(ulonglong orderID) { m_OrderID = orderID; }

private:
	bool		m_Success;	// transported as uint8
	string		m_Message;
	ulonglong	m_OrderID;
};

//////////////////////////////////////////////////////////////////////////////
// class GCExchangeBuyFactory;
//////////////////////////////////////////////////////////////////////////////

class GCExchangeBuyFactory : public PacketFactory
{
public:
	Packet* createPacket() throw() { return new GCExchangeBuy(); }
	string getPacketName() const throw() { return "GCExchangeBuy"; }
	PacketID_t getPacketID() const throw() { return Packet::PACKET_GC_EXCHANGE_BUY; }
	// success(1) + message length byte(1) + message max(255) + orderID(8)
	// Must stay identical to the server repo's GCExchangeBuyFactory::getPacketMaxSize()
	PacketSize_t getPacketMaxSize() const throw() { return 1 + 1 + 255 + 8; }
};

//////////////////////////////////////////////////////////////////////////////
// class GCExchangeBuyHandler;
//////////////////////////////////////////////////////////////////////////////

class GCExchangeBuyHandler
{
public:
	static void execute(GCExchangeBuy* pPacket, Player* pPlayer) throw ( ProtocolException , Error );
};

#endif // __GC_EXCHANGE_BUY_H__
