//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExchangeBuy.cpp
// Written By  : Exchange System
// Description : Client buys an item from the exchange
//////////////////////////////////////////////////////////////////////////////

#include "CGExchangeBuy.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

#include "Player.h"

CGExchangeBuy::CGExchangeBuy()
{
	m_ListingID = 0;
}

CGExchangeBuy::~CGExchangeBuy()
{
}

void CGExchangeBuy::read(SocketInputStream& iStream)
{
	__BEGIN_TRY

	// no 64-bit stream overload exists - read the raw 8 bytes directly
	iStream.read((char*)&m_ListingID, sizeof(ulonglong));

	BYTE szIdempotencyKey;
	iStream.read(szIdempotencyKey);
	if (szIdempotencyKey > 0)
	{
		char buf[256];
		iStream.read(buf, szIdempotencyKey);
		buf[szIdempotencyKey] = '\0';
		m_IdempotencyKey = buf;
	}
	else
	{
		m_IdempotencyKey = "";
	}

	__END_CATCH
}

void CGExchangeBuy::write(SocketOutputStream& oStream) const
{
	__BEGIN_TRY

	// no 64-bit stream overload exists - write the raw 8 bytes directly
	oStream.write((const char*)&m_ListingID, sizeof(ulonglong));

	// bstr : length byte is always written, even when the key is empty
	oStream.write((BYTE)m_IdempotencyKey.length());
	if (!m_IdempotencyKey.empty())
		oStream.write(m_IdempotencyKey);

	__END_CATCH
}

string CGExchangeBuy::toString() const
{
	StringStream msg;
	msg << "CGExchangeBuy(ListingID:" << m_ListingID
		<< ", IdempotencyKey:" << m_IdempotencyKey
		<< ")";
	return msg.toString();
}

void CGExchangeBuy::execute(Player* pPlayer)
{
	__BEGIN_TRY

	// This is handled by the server's packet handler
	// The client doesn't execute this packet

	__END_CATCH
}
