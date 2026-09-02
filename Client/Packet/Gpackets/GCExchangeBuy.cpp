//////////////////////////////////////////////////////////////////////////////
// Filename    : GCExchangeBuy.cpp
// Written By  : Exchange System
// Description : Server replies to the client's exchange buy request
//////////////////////////////////////////////////////////////////////////////

#include "GCExchangeBuy.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

// Out-of-line definition so the constant may also be odr-used (bound to a
// reference) by the UI and tests, not only read as a compile-time constant.
const PacketSize_t GCExchangeBuy::kMaxMessage;

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

	// bstr : length byte is always present, even when the message is empty.
	// The else-branch is not optional - read() must fully overwrite the
	// packet's state so that it stays a true mirror of write().
	BYTE szMessage;
	iStream.read(szMessage);
	if (szMessage > 0)
	{
		// char buf[256] is safe for any BYTE length : at 255 the terminator
		// lands on buf[255], the last element.
		char buf[256];
		iStream.read(buf, szMessage);
		buf[szMessage] = '\0';
		m_Message = buf;
	}
	else
	{
		m_Message.clear();
	}

	// no 64-bit stream overload exists - read the raw 8 bytes directly
	iStream.read((char*)&m_OrderID, sizeof(ulonglong));

	__END_CATCH
}

void GCExchangeBuy::write(SocketOutputStream & oStream) const
{
	__BEGIN_TRY

	oStream.write((BYTE)(m_Success ? 1 : 0));

	// bstr : length byte is always written, even when the message is empty.
	//
	// The clamp to kMaxMessage is duplicated, deliberately and identically, in
	// write() and in getPacketSize(). The packet size goes into the stream
	// header BEFORE write() is called, so the two must agree for every
	// possible value of the field; writing the truncated length byte but the
	// full string body desynchronises the connection.
	// oStream.write(m_Message) is NOT used here : it would emit the whole
	// string regardless of the length byte just written.
	const BYTE szMessage = (BYTE)(m_Message.length() > kMaxMessage ? kMaxMessage : m_Message.length());
	oStream.write(szMessage);
	if (szMessage > 0)
		oStream.write(m_Message.c_str(), szMessage);

	// no 64-bit stream overload exists - write the raw 8 bytes directly
	oStream.write((const char*)&m_OrderID, sizeof(ulonglong));

	__END_CATCH
}

string GCExchangeBuy::toString() const
{
	StringStream msg;
	// StringStream has a ulonglong overload, so the id prints in full - do not
	// route it through the int/long overloads, which would truncate it.
	msg << "GCExchangeBuy(Success:" << (m_Success ? "true" : "false")
		<< ", Message:" << m_Message
		<< ", OrderID:" << (ulonglong)m_OrderID
		<< ")";
	return msg.toString();
}
