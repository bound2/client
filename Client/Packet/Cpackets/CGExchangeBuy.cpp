//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExchangeBuy.cpp
// Written By  : Exchange System
// Description : Client buys an item from the exchange
//////////////////////////////////////////////////////////////////////////////

#include "CGExchangeBuy.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

#include "Player.h"

// Out-of-line definition so the constant may also be odr-used (bound to a
// reference) by the UI and tests, not only read as a compile-time constant.
const PacketSize_t CGExchangeBuy::kMaxIdempotencyKey;

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

	// bstr : length byte is always present, even when the key is empty.
	// The else-branch is not optional - read() must fully overwrite the
	// packet's state so that it stays a true mirror of write().
	BYTE szIdempotencyKey;
	iStream.read(szIdempotencyKey);

	// The cap is below the 255 a BYTE length can express, so a malformed
	// peer could declare more than write() can ever emit and make read()
	// consume past this packet into the ones queued behind it. Refuse it.
	if (szIdempotencyKey > kMaxIdempotencyKey)
		throw InvalidProtocolException("CGExchangeBuy: idempotency key longer than the cap");

	if (szIdempotencyKey > 0)
	{
		// char buf[256] is safe for any BYTE length : at 255 the terminator
		// lands on buf[255], the last element.
		char buf[256];
		iStream.read(buf, szIdempotencyKey);
		buf[szIdempotencyKey] = '\0';
		m_IdempotencyKey = buf;
	}
	else
	{
		m_IdempotencyKey.clear();
	}

	__END_CATCH
}

void CGExchangeBuy::write(SocketOutputStream& oStream) const
{
	__BEGIN_TRY

	// no 64-bit stream overload exists - write the raw 8 bytes directly
	oStream.write((const char*)&m_ListingID, sizeof(ulonglong));

	// bstr : length byte is always written, even when the key is empty.
	//
	// The clamp to kMaxIdempotencyKey is duplicated, deliberately and
	// identically, in write() and in getPacketSize(). The packet size goes
	// into the stream header BEFORE write() is called, so the two must agree
	// for every possible value of the field; writing the truncated length byte
	// but the full string body desynchronises the connection.
	// oStream.write(m_IdempotencyKey) is NOT used here : it would emit the
	// whole string regardless of the length byte just written.
	const BYTE szIdempotencyKey = (BYTE)(m_IdempotencyKey.length() > kMaxIdempotencyKey ? kMaxIdempotencyKey : m_IdempotencyKey.length());
	oStream.write(szIdempotencyKey);
	if (szIdempotencyKey > 0)
		oStream.write(m_IdempotencyKey.c_str(), szIdempotencyKey);

	__END_CATCH
}

string CGExchangeBuy::toString() const
{
	StringStream msg;
	// StringStream has a ulonglong overload, so the id prints in full - do not
	// route it through the int/long overloads, which would truncate it.
	msg << "CGExchangeBuy(ListingID:" << (ulonglong)m_ListingID
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
