//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExchangeList.cpp
// Written By  : Exchange System
// Description : Client requests listing list from server
//////////////////////////////////////////////////////////////////////////////

#include "CGExchangeList.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

#include "Player.h"

// Out-of-line definition so the constant may also be odr-used (bound to a
// reference) by the UI and tests, not only read as a compile-time constant.
const PacketSize_t CGExchangeList::kMaxSellerFilter;

CGExchangeList::CGExchangeList()
{
	m_Page = 1;
	m_PageSize = 20;
	m_ItemClass = 0xFF;  // All classes
	m_ItemType = 0xFFFF;  // All types
	m_MinPrice = 0;
	m_MaxPrice = 0;
}

CGExchangeList::~CGExchangeList()
{
}

void CGExchangeList::read(SocketInputStream& iStream)
{
	__BEGIN_TRY

	iStream.read(m_Page);
	iStream.read(m_PageSize);
	iStream.read(m_ItemClass);
	iStream.read(m_ItemType);
	iStream.read(m_MinPrice);
	iStream.read(m_MaxPrice);

	// bstr : length byte is always present, even when the filter is empty.
	// The else-branch is not optional - read() must fully overwrite the
	// packet's state so that it stays a true mirror of write().
	BYTE szSellerFilter;
	iStream.read(szSellerFilter);
	if (szSellerFilter > 0)
	{
		// char buf[256] is safe for any BYTE length : at 255 the terminator
		// lands on buf[255], the last element.
		char buf[256];
		iStream.read(buf, szSellerFilter);
		buf[szSellerFilter] = '\0';
		m_SellerFilter = buf;
	}
	else
	{
		m_SellerFilter.clear();
	}

	__END_CATCH
}

void CGExchangeList::write(SocketOutputStream& oStream) const
{
	__BEGIN_TRY

	oStream.write(m_Page);
	oStream.write(m_PageSize);
	oStream.write(m_ItemClass);
	oStream.write(m_ItemType);
	oStream.write(m_MinPrice);
	oStream.write(m_MaxPrice);

	// bstr : length byte is always written, even when the filter is empty.
	//
	// The clamp to kMaxSellerFilter is duplicated, deliberately and
	// identically, in write() and in getPacketSize(). The packet size goes
	// into the stream header BEFORE write() is called, so the two must agree
	// for every possible value of the field; writing the truncated length byte
	// but the full string body desynchronises the connection.
	// oStream.write(m_SellerFilter) is NOT used here : it would emit the whole
	// string regardless of the length byte just written.
	const BYTE szSellerFilter = (BYTE)(m_SellerFilter.length() > kMaxSellerFilter ? kMaxSellerFilter : m_SellerFilter.length());
	oStream.write(szSellerFilter);
	if (szSellerFilter > 0)
		oStream.write(m_SellerFilter.c_str(), szSellerFilter);

	__END_CATCH
}

string CGExchangeList::toString() const
{
	StringStream msg;
	msg << "CGExchangeList("
		<< "Page:" << (int)m_Page
		<< ", PageSize:" << (int)m_PageSize
		<< ", ItemClass:" << (int)m_ItemClass
		<< ", ItemType:" << (int)m_ItemType
		<< ", MinPrice:" << (int)m_MinPrice
		<< ", MaxPrice:" << (int)m_MaxPrice
		<< ", SellerFilter:" << m_SellerFilter
		<< ")";
	return msg.toString();
}

