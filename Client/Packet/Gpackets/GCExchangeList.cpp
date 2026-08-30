//////////////////////////////////////////////////////////////////////////////
// Filename    : GCExchangeList.cpp
// Written By  : Exchange System
// Description : Server sends listing list to client
//////////////////////////////////////////////////////////////////////////////

#include "GCExchangeList.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

#include "Player.h"

GCExchangeList::GCExchangeList()
{
	m_Page = 1;
	m_PageSize = 20;
	m_Total = 0;
}

GCExchangeList::~GCExchangeList()
{
}

void GCExchangeList::read(SocketInputStream & iStream)
{
	__BEGIN_TRY

	// Read page info
	iStream.read(m_Page);
	iStream.read(m_PageSize);
	iStream.read(m_Total);

	// Read listing count
	uint16_t count = 0;
	iStream.read(count);

	// Read listings - this must stay an exact mirror of write() below and of
	// the server repo's GCExchangeList::write().
	m_Listings.clear();
	for (uint16_t i = 0; i < count; i++)
	{
		ExchangeListing listing;

		char buf[256];
		uint8_t len;

		// 64-bit values travel as little-endian raw bytes; the stream has no
		// 64-bit overload, so read them through the raw byte interface.
		ulonglong listingID = 0;
		iStream.read((char*)&listingID, 8);
		listing.listingID = (int64_t)listingID;

		ushort serverID = 0;
		iStream.read(serverID);
		listing.serverID = (int16_t)serverID;

		// SellerAccount
		iStream.read(len);
		if (len > 0)
		{
			iStream.read(buf, len);
			buf[len] = '\0';
			listing.sellerAccount = buf;
		}

		// SellerPlayer
		iStream.read(len);
		if (len > 0)
		{
			iStream.read(buf, len);
			buf[len] = '\0';
			listing.sellerPlayer = buf;
		}

		iStream.read(listing.sellerRace);
		iStream.read(listing.itemClass);
		iStream.read(listing.itemType);

		ulonglong itemID = 0;
		iStream.read((char*)&itemID, 8);
		listing.itemID = (int64_t)itemID;

		iStream.read(listing.objectID);
		iStream.read(listing.pricePoint);
		iStream.read(listing.currency);
		iStream.read(listing.status);

		// BuyerAccount
		iStream.read(len);
		if (len > 0)
		{
			iStream.read(buf, len);
			buf[len] = '\0';
			listing.buyerAccount = buf;
		}

		// BuyerPlayer
		iStream.read(len);
		if (len > 0)
		{
			iStream.read(buf, len);
			buf[len] = '\0';
			listing.buyerPlayer = buf;
		}

		iStream.read(listing.taxRate);
		iStream.read(listing.taxAmount);

		// Timestamp strings (soldAt/cancelledAt/updatedAt are not on the wire)
		iStream.read(len);
		if (len > 0)
		{
			iStream.read(buf, len);
			buf[len] = '\0';
			listing.createdAt = buf;
		}

		iStream.read(len);
		if (len > 0)
		{
			iStream.read(buf, len);
			buf[len] = '\0';
			listing.expireAt = buf;
		}

		iStream.read(listing.version);

		// Snapshot fields
		// ItemName
		iStream.read(len);
		if (len > 0)
		{
			iStream.read(buf, len);
			buf[len] = '\0';
			listing.itemName = buf;
		}

		iStream.read(listing.enchantLevel);
		iStream.read(listing.grade);
		iStream.read(listing.durability);
		iStream.read(listing.silver);
		iStream.read(listing.optionType1);
		iStream.read(listing.optionType2);
		iStream.read(listing.optionType3);
		iStream.read(listing.optionValue1);
		iStream.read(listing.optionValue2);
		iStream.read(listing.optionValue3);
		iStream.read(listing.stackCount);

		m_Listings.push_back(listing);
	}

	__END_CATCH
}

void GCExchangeList::write(SocketOutputStream & oStream) const
{
	__BEGIN_TRY

	// Write page info
	oStream.write(m_Page);
	oStream.write(m_PageSize);
	oStream.write(m_Total);

	// Write listing count
	uint16_t count = (uint16_t)m_Listings.size();
	oStream.write(count);

	// Write listings - exact mirror of read() above
	for (size_t i = 0; i < m_Listings.size(); i++)
	{
		const ExchangeListing & listing = m_Listings[i];

		ulonglong listingID = (ulonglong)listing.listingID;
		oStream.write((const char*)&listingID, 8);

		oStream.write((ushort)listing.serverID);

		// SellerAccount
		uint8_t len = (uint8_t)listing.sellerAccount.length();
		oStream.write(len);
		if (len > 0)
			oStream.write(listing.sellerAccount.c_str(), len);

		// SellerPlayer
		len = (uint8_t)listing.sellerPlayer.length();
		oStream.write(len);
		if (len > 0)
			oStream.write(listing.sellerPlayer.c_str(), len);

		oStream.write(listing.sellerRace);
		oStream.write(listing.itemClass);
		oStream.write(listing.itemType);

		ulonglong itemID = (ulonglong)listing.itemID;
		oStream.write((const char*)&itemID, 8);

		oStream.write(listing.objectID);
		oStream.write(listing.pricePoint);
		oStream.write(listing.currency);
		oStream.write(listing.status);

		// BuyerAccount
		len = (uint8_t)listing.buyerAccount.length();
		oStream.write(len);
		if (len > 0)
			oStream.write(listing.buyerAccount.c_str(), len);

		// BuyerPlayer
		len = (uint8_t)listing.buyerPlayer.length();
		oStream.write(len);
		if (len > 0)
			oStream.write(listing.buyerPlayer.c_str(), len);

		oStream.write(listing.taxRate);
		oStream.write(listing.taxAmount);

		// Timestamp strings
		len = (uint8_t)listing.createdAt.length();
		oStream.write(len);
		if (len > 0)
			oStream.write(listing.createdAt.c_str(), len);

		len = (uint8_t)listing.expireAt.length();
		oStream.write(len);
		if (len > 0)
			oStream.write(listing.expireAt.c_str(), len);

		oStream.write(listing.version);

		// Snapshot fields
		len = (uint8_t)listing.itemName.length();
		oStream.write(len);
		if (len > 0)
			oStream.write(listing.itemName.c_str(), len);

		oStream.write(listing.enchantLevel);
		oStream.write(listing.grade);
		oStream.write(listing.durability);
		oStream.write(listing.silver);
		oStream.write(listing.optionType1);
		oStream.write(listing.optionType2);
		oStream.write(listing.optionType3);
		oStream.write(listing.optionValue1);
		oStream.write(listing.optionValue2);
		oStream.write(listing.optionValue3);
		oStream.write(listing.stackCount);
	}

	__END_CATCH
}

PacketSize_t GCExchangeList::getPacketSize() const throw()
{
	PacketSize_t size = szint * 3		// page, pageSize, total
		+ sizeof(uint16_t);				// count

	for (size_t i = 0; i < m_Listings.size(); i++)
	{
		const ExchangeListing & listing = m_Listings[i];

		size += 8;							// listingID (u64 on the wire)
		size += sizeof(uint16_t);			// serverID (u16 on the wire)
		size += sizeof(uint8_t) + listing.sellerAccount.length();
		size += sizeof(uint8_t) + listing.sellerPlayer.length();
		size += sizeof(listing.sellerRace);
		size += sizeof(listing.itemClass);
		size += sizeof(listing.itemType);
		size += 8;							// itemID (u64 on the wire)
		size += sizeof(listing.objectID);
		size += sizeof(listing.pricePoint);
		size += sizeof(listing.currency);
		size += sizeof(listing.status);
		size += sizeof(uint8_t) + listing.buyerAccount.length();
		size += sizeof(uint8_t) + listing.buyerPlayer.length();
		size += sizeof(listing.taxRate);
		size += sizeof(listing.taxAmount);
		size += sizeof(uint8_t) + listing.createdAt.length();
		size += sizeof(uint8_t) + listing.expireAt.length();
		size += sizeof(listing.version);
		size += sizeof(uint8_t) + listing.itemName.length();
		size += sizeof(listing.enchantLevel);
		size += sizeof(listing.grade);
		size += sizeof(listing.durability);
		size += sizeof(listing.silver);
		size += sizeof(listing.optionType1);
		size += sizeof(listing.optionType2);
		size += sizeof(listing.optionType3);
		size += sizeof(listing.optionValue1);
		size += sizeof(listing.optionValue2);
		size += sizeof(listing.optionValue3);
		size += sizeof(listing.stackCount);
	}

	return size;
}

string GCExchangeList::toString() const
{
	StringStream msg;
	msg << "GCExchangeList("
		<< "Page:" << m_Page
		<< ",PageSize:" << m_PageSize
		<< ",Total:" << m_Total
		<< ",Count:" << (int)m_Listings.size()
		<< ")";
	return msg.toString();
}

void GCExchangeList::execute(Player* pPlayer)
{
	__BEGIN_TRY

	// Client-side packet handling
	// This will be implemented to update the UI

	__END_CATCH
}
