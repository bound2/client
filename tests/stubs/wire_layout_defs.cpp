//----------------------------------------------------------------------
// wire_layout_defs.cpp
//----------------------------------------------------------------------
//
// Two inputs to getPacketMaxSize() are defined out of line, in files
// unit_tests cannot link: QuestStatusInfo.cpp (drags the socket stream
// chain) and GCMonsterKillQuestInfo.cpp (a packet .cpp, i.e. its handler
// and the game). test_wire_layout.cpp needs their values, so they are
// mirrored here, expression for expression.
//
// This is the one place the client inventory can go stale without the
// wire_inventory_fresh test noticing: if either original changes, update
// it here too. The cross-repo diff (server/tests/tools/
// wire_inventory_diff.sh) is the backstop - the server computes its own
// value, so a real disagreement still surfaces there.
//
//----------------------------------------------------------------------

#include "QuestStatusInfo.h"
#include "Gpackets/GCMonsterKillQuestInfo.h"

// Client/Packet/QuestStatusInfo.cpp
PacketSize_t QuestStatusInfo::getMaxSize()
{
	return szDWORD + szBYTE + szBYTE + MissionInfo::getMaxSize() * MAX_MISSION_NUM;
}

// Client/Packet/Gpackets/GCMonsterKillQuestInfo.cpp
const int GCMonsterKillQuestInfo::szQuestInfo = szQuestID + szSpriteType + szWORD + szDWORD;
