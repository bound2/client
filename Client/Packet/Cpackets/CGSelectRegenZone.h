//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSelectRegenZone.h 
// Written By  : excel96
// Description : 
// �����̾ ���ο� ��Ż�� �̿��� �ٸ� ������ �̵��ϰ��� �� �� ������ ��Ŷ
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_SELECT_REGEN_ZONE_H__
#define __CG_SELECT_REGEN_ZONE_H__

#include "Types.h"
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGSelectRegenZone;
//////////////////////////////////////////////////////////////////////////////

class CGSelectRegenZone : public Packet 
{
public:
    void read(SocketInputStream & iStream) throw(ProtocolException, Error);
    void write(SocketOutputStream & oStream) const throw(ProtocolException, Error);
	void execute(Player* pPlayer) throw(ProtocolException, Error);
	PacketID_t getPacketID() const throw() { return PACKET_CG_SELECT_REGEN_ZONE; }
	PacketSize_t getPacketSize() const throw() { return szBYTE; }
#ifdef __DEBUG_OUTPUT__
	std::string getPacketName() const throw() { return "CGSelectRegenZone"; }
	std::string toString() const throw();
#endif

public:
	BYTE getRegenZoneID() const { return m_RegenZoneID; }
	void setRegenZoneID( BYTE RegenZoneID ) { m_RegenZoneID = RegenZoneID; }
private:
	BYTE m_RegenZoneID;
};

//////////////////////////////////////////////////////////////////////////////
// class CGSelectRegenZoneFactory;
//////////////////////////////////////////////////////////////////////////////

class CGSelectRegenZoneFactory : public PacketFactory 
{
	Packet* createPacket() throw() { return new CGSelectRegenZone(); }
	std::string getPacketName() const throw() { return "CGSelectRegenZone"; }
	PacketID_t getPacketID() const throw() { return Packet::PACKET_CG_SELECT_REGEN_ZONE; }
	PacketSize_t getPacketMaxSize() const throw() { return szBYTE; }
};

//////////////////////////////////////////////////////////////////////////////
// class CGSelectRegenZoneHandler;
//////////////////////////////////////////////////////////////////////////////

class CGSelectRegenZoneHandler 
{
public:
	static void execute(CGSelectRegenZone* pCGSelectRegenZone, Player* pPlayer) throw(Error);
};

#endif