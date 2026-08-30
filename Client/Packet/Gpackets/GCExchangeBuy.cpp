//////////////////////////////////////////////////////////////////////////////
// Filename    : GCExchangeBuy.cpp
// Written By  : Exchange System
// Description : Server replies to the client's exchange buy request
//////////////////////////////////////////////////////////////////////////////

#include "GCExchangeBuy.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

#include "Player.h"

GCExchangeBuy::GCExchangeBuy()
{
	m_Success = false;
	m_OrderID = 0;
}

GCExchangeBuy::~GCExchangeBuy()
{
}

void GCExchangeBuy::read(SocketInputStream & iStream)
{
	__BEGIN_TRY

	BYTE success;
	iStream.read(success);
	m_Success = (success != 0);

	// bstr : length byte is always present, even when the message is empty
	BYTE szMessage;
	iStream.read(szMessage);
	if (szMessage > 0)
	{
		char buf[256];
		iStream.read(buf, szMessage);
		buf[szMessage] = '\0';
		m_Message = buf;
	}
	else
	{
		m_Message = "";
	}

	// no 64-bit stream overload exists - read the raw 8 bytes directly
	iStream.read((char*)&m_OrderID, sizeof(ulonglong));

	__END_CATCH
}

void GCExchangeBuy::write(SocketOutputStream & oStream) const
{
	__BEGIN_TRY

	oStream.write((BYTE)(m_Success ? 1 : 0));

	// bstr : length byte is always written, even when the message is empty
	oStream.write((BYTE)m_Message.length());
	if (!m_Message.empty())
		oStream.write(m_Message);

	// no 64-bit stream overload exists - write the raw 8 bytes directly
	oStream.write((const char*)&m_OrderID, sizeof(ulonglong));

	__END_CATCH
}

string GCExchangeBuy::toString() const
{
	StringStream msg;
	msg << "GCExchangeBuy(Success:" << (m_Success ? "true" : "false")
		<< ", Message:" << m_Message
		<< ", OrderID:" << m_OrderID
		<< ")";
	return msg.toString();
}

void GCExchangeBuy::execute(Player* pPlayer)
{
	__BEGIN_TRY

	GCExchangeBuyHandler::execute(this, pPlayer);

	__END_CATCH
}

void GCExchangeBuyHandler::execute(GCExchangeBuy* pPacket, Player* pPlayer)
	throw ( ProtocolException , Error )
{
	__BEGIN_TRY

	// Client-side packet handling
	// This will be implemented to update the exchange UI with the buy result

	__END_CATCH
}
