//----------------------------------------------------------------------
// test_packet_goldens.cpp
//----------------------------------------------------------------------
//
// Golden-byte pins and round-trips for representative packets through
// the real stream classes (docs/RESTRUCTURING.md task 2.4; the recipe
// is the server repo's task 1.2).
//
// The .hex files under tests/golden/ ARE the wire contract. Where a
// packet is pinned in both repos, the golden here is a byte-identical
// copy of server/tests/golden/<same name>: this client and that server
// carry hand-maintained copies of every packet class, and a byte that
// moves in one repo breaks live sessions with no compile error
// anywhere. A failing golden is a protocol change to be reviewed, not
// a test to be silenced; diffing the two golden directories is the
// cross-repo check.
//
// Encrypt codes: 0 is the plain branch, 1..5 the __USE_ENCRYPTER__
// branch through every `code % N` case of SHUFFLE_STATEMENT_2.._5
// (EncryptUtility.h) - the shuffle IS part of the wire format, and the
// _4/_5 tables have non-rotation cases that codes 1..3 never reach.
// Packets that never touch the encrypter are pinned at code 0 only,
// and a test asserts they are still encrypter-free, so a packet that
// starts shuffling cannot hide behind a single golden.
//
// Fixture values follow the server's canonical-value rules: distinct
// per field and >= 128 in every byte the width allows, so a signedness
// flip, a same-width type swap or two transposed fields all move bytes
// in the golden (an all-<128 fixture cannot tell BYTE from char).
//
// Set UPDATE_GOLDENS=1 to (re)record instead of compare. Any other
// value compares - UPDATE_GOLDENS=0 must not silently rewrite every
// pin and pass.
//
// Compiled with the packetwire defines (tests/CMakeLists.txt), so the
// Packet and stream definitions are identical to the library's.
//
//----------------------------------------------------------------------

#include "test_framework.h"
#include "packet_stream_access.h"

#include "Socket.h"
#include "SocketImpl.h"
#include "SocketEncryptInputStream.h"
#include "SocketEncryptOutputStream.h"
#include "Exception.h"

#include "Cpackets/CGMove.h"
#include "Cpackets/CGSay.h"
#include "Cpackets/CGWhisper.h"
#include "Cpackets/CLLogin.h"
#include "Gpackets/GCAddInstalledMineToZone.h"
#include "Gpackets/GCAddNewItemToZone.h"
#include "Gpackets/GCDropItemToZone.h"
#include "Gpackets/GCGuildChat.h"
#include "Gpackets/GCMoveError.h"
#include "Gpackets/GCMoveOK.h"
#include "Gpackets/GCSay.h"
#include "Gpackets/GCSystemMessage.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

const uchar	kEncryptCodes[] = { 0, 1, 2, 3, 4, 5 };
const size_t	kEncryptCodeCount = sizeof(kEncryptCodes) / sizeof(kEncryptCodes[0]);

//----------------------------------------------------------------------
// Streams over a never-used socket (see packet_stream_access.h).
//----------------------------------------------------------------------
struct OutFixture
{
	Socket				m_Socket;
	SocketEncryptOutputStream	m_Stream;

	OutFixture()
	: m_Socket((EnsureSocketsInitialised(), new SocketImpl())),
	  m_Stream(&m_Socket)
	{
	}
};

struct InFixture
{
	Socket				m_Socket;
	SocketEncryptInputStream	m_Stream;

	InFixture()
	: m_Socket((EnsureSocketsInitialised(), new SocketImpl())),
	  m_Stream(&m_Socket, 4096)
	{
	}
};

// The packet body exactly as a session with encrypt code `code` would
// put it on the wire. Nothing is flushed anywhere.
std::vector<unsigned char>	WriteBody(const Packet& packet, uchar code)
{
	OutFixture f;
	f.m_Stream.setEncryptCode(code);
	packet.write(f.m_Stream);
	return SocketOutputStreamTestAccess::Bytes(f.m_Stream);
}

// The packet as SocketOutputStream::write(const Packet*) frames it: id,
// size and the sequence byte, then the body. Pins the header field
// widths, which no body golden can see - a change to PacketID_t or
// PacketSize_t desyncs every packet at byte one.
std::vector<unsigned char>	WriteFramed(const Packet& packet, uchar code)
{
	OutFixture f;
	f.m_Stream.setEncryptCode(code);
	f.m_Stream.write(&packet);
	return SocketOutputStreamTestAccess::Bytes(f.m_Stream);
}

std::string	ToHex(const std::vector<unsigned char>& bytes)
{
	static const char* digits = "0123456789abcdef";
	std::string hex;
	hex.reserve(bytes.size() * 2);
	for (size_t i = 0; i < bytes.size(); i++)
	{
		hex.push_back(digits[bytes[i] >> 4]);
		hex.push_back(digits[bytes[i] & 0x0F]);
	}
	return hex;
}

bool	IsRecording()
{
	const char* value = std::getenv("UPDATE_GOLDENS");
	return value != NULL && std::strcmp(value, "1") == 0;
}

std::string	GoldenPath(const std::string& name, uchar code)
{
	std::ostringstream path;
	path << WIRE_GOLDEN_DIR << "/" << name << ".code" << (int)code << ".hex";
	return path.str();
}

// Compare `bytes` against tests/golden/<name>.code<code>.hex, or record
// it under UPDATE_GOLDENS=1. A missing golden fails with instructions.
void	ExpectGolden(const std::string& name, uchar code, const std::vector<unsigned char>& bytes)
{
	const std::string path = GoldenPath(name, code);
	const std::string actual = ToHex(bytes);

	if (IsRecording())
	{
		std::ofstream file(path.c_str(), std::ios::trunc | std::ios::binary);
		CHECK(file.good());
		file << actual << "\n";
		std::printf("  recorded golden %s\n", path.c_str());
		return;
	}

	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.good())
	{
		std::fprintf(stderr,
			"  missing golden %s - run once with UPDATE_GOLDENS=1 to record it,\n"
			"  review the bytes against the packet's read()/write(), and commit it\n",
			path.c_str());
		CHECK(file.good());
		return;
	}
	std::string expected;
	std::getline(file, expected);
	if (expected != actual)
		std::fprintf(stderr,
			"  wire bytes changed for %s (encrypt code %d)\n"
			"    golden: %s\n"
			"    actual: %s\n"
			"  this is a protocol-breaking change unless the server repo ships the\n"
			"  identical change; if intended, re-record with UPDATE_GOLDENS=1\n",
			name.c_str(), (int)code, expected.c_str(), actual.c_str());
	CHECK(expected == actual);
}

// write() src with encrypt code `code`, read() the bytes back into dst
// through an input stream with the same code. Also pins the two
// invariants a packet can silently break: getPacketSize() must equal
// what write() emitted (the framing header advertises it), and read()
// must consume exactly that (a byte left over desyncs every packet
// after it).
void	RoundTrip(const Packet& src, Packet& dst, uchar code)
{
	std::vector<unsigned char> body = WriteBody(src, code);
	CHECK_EQ(src.getPacketSize(), body.size());

	InFixture f;
	f.m_Stream.setEncryptCode(code);
	SocketInputStreamTestAccess::Preload(f.m_Stream, body.empty() ? NULL : &body[0],
					     (unsigned int)body.size());
	dst.read(f.m_Stream);
	CHECK(f.m_Stream.isEmpty());
}

// A packet that never calls readEncrypt/writeEncrypt writes the same
// bytes under every code. Pinned so a packet cannot start shuffling
// while its single code-0 golden keeps passing.
bool	EncrypterFree(const Packet& packet)
{
	const std::vector<unsigned char> plain = WriteBody(packet, 0);
	for (size_t i = 1; i < kEncryptCodeCount; i++)
		if (WriteBody(packet, kEncryptCodes[i]) != plain)
			return false;
	return true;
}

//----------------------------------------------------------------------
// Fixtures shared with the server repo (values copied from its
// packet_roundtrip_test.cpp / packet_encrypter_test.cpp so the goldens
// are the same files).
//----------------------------------------------------------------------
void	Fill(GCMoveOK& p)	{ p.setX(11); p.setY(22); p.setDir(3); }
void	Fill(CGMove& p)		{ p.setX(101); p.setY(57); p.setDir(6); }
void	Fill(GCMoveError& p)	{ p.setX(0x91); p.setY(0xA3); }
void	Fill(CGSay& p)		{ p.setColor(0x11223344); p.setMessage("hello darkeden"); }
void	Fill(CGWhisper& p)
{
	p.setName("Reiot");
	p.setColor(0xCAFEBABE);
	p.setMessage("wire pin test");
}

// GCAddItemToZone is the abstract base of the three item-to-zone
// packets and the only encrypter layout with a variable-length tail: a
// BYTE-counted option list and a BYTE-counted SubItemInfo list follow
// the shuffled header unencrypted. Two options and one sub-item, so
// both counts are non-trivial and the per-element layout is pinned.
void	FillItemBase(GCAddItemToZone& p)
{
	p.setObjectID(0xE0F10213);
	p.setX(0x87);
	p.setY(0x99);
	p.setItemClass(0xAB);
	p.setItemType(0x8ABC);
	p.addOptionType(0xBD);
	p.addOptionType(0xCF);
	p.setSilver(0x9DEF);
	p.setGrade(-0x12345678);
	p.setDurability(0xF2031425);
	p.setEnchantLevel(-0x37);
	p.setItemNum(0xD1);
	SubItemInfo* pSub = new SubItemInfo();
	pSub->setObjectID(0x03142536);
	pSub->setItemClass(0xE3);
	pSub->setItemType(0x9BCD);
	pSub->setItemNum(0xF5);
	pSub->setSlotID(0x86);
	p.addListElement(pSub);	// the packet's destructor owns it
	p.setListNum(1);
}

void	CheckItemBaseEqual(GCAddItemToZone& a, GCAddItemToZone& b)
{
	CHECK_EQ(a.getObjectID(), b.getObjectID());
	CHECK_EQ(a.getX(), b.getX());
	CHECK_EQ(a.getY(), b.getY());
	CHECK_EQ(a.getItemClass(), b.getItemClass());
	CHECK_EQ(a.getItemType(), b.getItemType());
	CHECK(a.getOptionType() == b.getOptionType());
	CHECK_EQ(a.getSilver(), b.getSilver());
	CHECK_EQ(a.getGrade(), b.getGrade());
	CHECK_EQ(a.getDurability(), b.getDurability());
	CHECK_EQ(a.getEnchantLevel(), b.getEnchantLevel());
	CHECK_EQ(a.getItemNum(), b.getItemNum());
	CHECK_EQ(a.getListNum(), b.getListNum());
	// No const accessor for the sub-item list: pop the single element
	// from each side and compare. Popped elements are ours to free.
	SubItemInfo* pa = a.popFrontListElement();
	SubItemInfo* pb = b.popFrontListElement();
	CHECK(pa != NULL && pb != NULL);
	if (pa != NULL && pb != NULL)
	{
		CHECK_EQ(pa->getObjectID(), pb->getObjectID());
		CHECK_EQ(pa->getItemClass(), pb->getItemClass());
		CHECK_EQ(pa->getItemType(), pb->getItemType());
		CHECK_EQ(pa->getItemNum(), pb->getItemNum());
		CHECK_EQ(pa->getSlotID(), pb->getSlotID());
	}
	delete pa;
	delete pb;
}

//----------------------------------------------------------------------
// Client-authored fixtures: the chat/guild/system-message family the
// code-health review named as the highest-risk parsers, and the login
// packet whose layout switches on the netmarble flag.
//----------------------------------------------------------------------
void	Fill(GCSay& p)
{
	p.setObjectID(0x81A2C3E4);
	p.setColor(0x95B6D7F8);
	p.setMessage("wire pin say");
}

// type != 0 carries the sending guild's name before the sender.
void	Fill(GCGuildChat& p)
{
	p.setType(0x85);
	p.setSendGuildName("Nosferatu");
	p.setSender("Reiot");
	p.setColor(0xA1B2C3D4);
	p.setMessage("guild wire pin");
}

void	FillType0(GCGuildChat& p)
{
	p.setType(0);
	p.setSender("Reiot");
	p.setColor(0xA1B2C3D4);
	p.setMessage("guild wire pin");
}

void	Fill(GCSystemMessage& p)
{
	p.setMessage("system wire pin");
	p.setColor(0xC1D2E3F4);
	p.setType(SYSTEM_MESSAGE_COMBAT);
}

void	Fill(CLLogin& p)
{
	const BYTE mac[6] = { 0x80, 0x91, 0xA2, 0xB3, 0xC4, 0xD5 };
	p.setID("reiot");
	p.setPassword("wirepin");
	p.setMacAddress(mac);
}

} // namespace

//----------------------------------------------------------------------
// GCMoveOK - fixed-width fields through SHUFFLE_STATEMENT_3
//----------------------------------------------------------------------
TEST(GCMoveOK, RoundTripsForEveryEncryptCode)
{
	for (size_t i = 0; i < kEncryptCodeCount; i++)
	{
		GCMoveOK src, dst;
		Fill(src);
		RoundTrip(src, dst, kEncryptCodes[i]);
		CHECK_EQ(src.getX(), dst.getX());
		CHECK_EQ(src.getY(), dst.getY());
		CHECK_EQ(src.getDir(), dst.getDir());
	}
}

TEST(GCMoveOK, BodyBytesMatchTheSharedGoldenForEveryEncryptCode)
{
	GCMoveOK packet;
	Fill(packet);
	for (size_t i = 0; i < kEncryptCodeCount; i++)
		ExpectGolden("GCMoveOK", kEncryptCodes[i], WriteBody(packet, kEncryptCodes[i]));
}

// The one framed golden: header widths (ushort id, uint size, BYTE
// sequence) plus the body, the same file the server pins.
TEST(GCMoveOK, FramedBytesMatchTheSharedGolden)
{
	GCMoveOK packet;
	Fill(packet);
	ExpectGolden("GCMoveOK.framed", 0, WriteFramed(packet, 0));
}

//----------------------------------------------------------------------
// CGMove - the client-to-server twin
//----------------------------------------------------------------------
TEST(CGMove, RoundTripsForEveryEncryptCode)
{
	for (size_t i = 0; i < kEncryptCodeCount; i++)
	{
		CGMove src, dst;
		Fill(src);
		RoundTrip(src, dst, kEncryptCodes[i]);
		CHECK_EQ(src.getX(), dst.getX());
		CHECK_EQ(src.getY(), dst.getY());
		CHECK_EQ(src.getDir(), dst.getDir());
	}
}

TEST(CGMove, BodyBytesMatchTheSharedGoldenForEveryEncryptCode)
{
	CGMove packet;
	Fill(packet);
	for (size_t i = 0; i < kEncryptCodeCount; i++)
		ExpectGolden("CGMove", kEncryptCodes[i], WriteBody(packet, kEncryptCodes[i]));
}

//----------------------------------------------------------------------
// GCMoveError - SHUFFLE_STATEMENT_2
//----------------------------------------------------------------------
TEST(GCMoveError, RoundTripsForEveryEncryptCode)
{
	for (size_t i = 0; i < kEncryptCodeCount; i++)
	{
		GCMoveError src, dst;
		Fill(src);
		RoundTrip(src, dst, kEncryptCodes[i]);
		CHECK_EQ(src.getX(), dst.getX());
		CHECK_EQ(src.getY(), dst.getY());
	}
}

TEST(GCMoveError, BodyBytesMatchTheSharedGoldenForEveryEncryptCode)
{
	GCMoveError packet;
	Fill(packet);
	for (size_t i = 0; i < kEncryptCodeCount; i++)
		ExpectGolden("GCMoveError", kEncryptCodes[i], WriteBody(packet, kEncryptCodes[i]));
}

//----------------------------------------------------------------------
// The GCAddItemToZone family - shuffled header + unencrypted lists
//----------------------------------------------------------------------
TEST(GCAddNewItemToZone, RoundTripsForEveryEncryptCode)
{
	for (size_t i = 0; i < kEncryptCodeCount; i++)
	{
		GCAddNewItemToZone src, dst;
		FillItemBase(src);
		RoundTrip(src, dst, kEncryptCodes[i]);
		CheckItemBaseEqual(src, dst);
	}
}

TEST(GCAddNewItemToZone, BodyBytesMatchTheSharedGoldenForEveryEncryptCode)
{
	GCAddNewItemToZone packet;
	FillItemBase(packet);
	for (size_t i = 0; i < kEncryptCodeCount; i++)
		ExpectGolden("GCAddNewItemToZone", kEncryptCodes[i], WriteBody(packet, kEncryptCodes[i]));
}

TEST(GCAddInstalledMineToZone, RoundTripsForEveryEncryptCode)
{
	for (size_t i = 0; i < kEncryptCodeCount; i++)
	{
		GCAddInstalledMineToZone src, dst;
		FillItemBase(src);
		RoundTrip(src, dst, kEncryptCodes[i]);
		CheckItemBaseEqual(src, dst);
	}
}

TEST(GCAddInstalledMineToZone, BodyBytesMatchTheSharedGoldenForEveryEncryptCode)
{
	GCAddInstalledMineToZone packet;
	FillItemBase(packet);
	for (size_t i = 0; i < kEncryptCodeCount; i++)
		ExpectGolden("GCAddInstalledMineToZone", kEncryptCodes[i], WriteBody(packet, kEncryptCodes[i]));
}

// GCDropItemToZone appends a pet ObjectID after the base layout. The
// server repo pins only its write(): its read() still consumes a
// leading flag byte that write() no longer emits (server RESTRUCTURING
// 1.2). This copy's read() consumes exactly what write() emits, so the
// client also round-trips - and the shared golden proves the client
// parses what the server actually sends.
TEST(GCDropItemToZone, RoundTripsForEveryEncryptCode)
{
	for (size_t i = 0; i < kEncryptCodeCount; i++)
	{
		GCDropItemToZone src, dst;
		FillItemBase(src);
		src.setDropPetOID(0x47586970);
		RoundTrip(src, dst, kEncryptCodes[i]);
		CheckItemBaseEqual(src, dst);
		CHECK_EQ(src.getDropPetOID(), dst.getDropPetOID());
	}
}

TEST(GCDropItemToZone, BodyBytesMatchTheSharedGoldenForEveryEncryptCode)
{
	GCDropItemToZone packet;
	FillItemBase(packet);
	packet.setDropPetOID(0x47586970);
	for (size_t i = 0; i < kEncryptCodeCount; i++)
		ExpectGolden("GCDropItemToZone", kEncryptCodes[i], WriteBody(packet, kEncryptCodes[i]));
}

//----------------------------------------------------------------------
// CGSay / CGWhisper - BYTE-length-prefixed strings, encrypter-free
//----------------------------------------------------------------------
TEST(CGSay, RoundTripsAndMatchesTheSharedGolden)
{
	CGSay src, dst;
	Fill(src);
	CHECK(EncrypterFree(src));
	RoundTrip(src, dst, 0);
	CHECK_EQ(src.getColor(), dst.getColor());
	CHECK(src.getMessage() == dst.getMessage());
	ExpectGolden("CGSay", 0, WriteBody(src, 0));
}

TEST(CGWhisper, RoundTripsAndMatchesTheSharedGolden)
{
	CGWhisper src, dst;
	Fill(src);
	CHECK(EncrypterFree(src));
	RoundTrip(src, dst, 0);
	CHECK(src.getName() == dst.getName());
	CHECK_EQ(src.getColor(), dst.getColor());
	CHECK(src.getMessage() == dst.getMessage());
	ExpectGolden("CGWhisper", 0, WriteBody(src, 0));
}

//----------------------------------------------------------------------
// Client-authored pins: chat, guild chat, system message, login
//----------------------------------------------------------------------
TEST(GCSay, RoundTripsAndMatchesGolden)
{
	GCSay src, dst;
	Fill(src);
	CHECK(EncrypterFree(src));
	RoundTrip(src, dst, 0);
	CHECK_EQ(src.getObjectID(), dst.getObjectID());
	CHECK_EQ(src.getColor(), dst.getColor());
	CHECK(src.getMessage() == dst.getMessage());
	ExpectGolden("GCSay", 0, WriteBody(src, 0));
}

TEST(GCGuildChat, RoundTripsAndMatchesGoldenWithAndWithoutTheGuildName)
{
	{
		GCGuildChat src, dst;
		Fill(src);
		CHECK(EncrypterFree(src));
		RoundTrip(src, dst, 0);
		CHECK_EQ(src.getType(), dst.getType());
		CHECK(src.getSendGuildName() == dst.getSendGuildName());
		CHECK(src.getSender() == dst.getSender());
		CHECK_EQ(src.getColor(), dst.getColor());
		CHECK(src.getMessage() == dst.getMessage());
		ExpectGolden("GCGuildChat", 0, WriteBody(src, 0));
	}
	{
		GCGuildChat src, dst;
		FillType0(src);
		RoundTrip(src, dst, 0);
		CHECK_EQ(src.getType(), dst.getType());
		CHECK(std::string() == dst.getSendGuildName());
		CHECK(src.getSender() == dst.getSender());
		CHECK(src.getMessage() == dst.getMessage());
		ExpectGolden("GCGuildChat.type0", 0, WriteBody(src, 0));
	}
}

TEST(GCSystemMessage, RoundTripsAndMatchesGolden)
{
	GCSystemMessage src, dst;
	Fill(src);
	CHECK(EncrypterFree(src));
	RoundTrip(src, dst, 0);
	CHECK(src.getMessage() == dst.getMessage());
	CHECK_EQ(src.getColor(), dst.getColor());
	CHECK_EQ((int)src.getType(), (int)dst.getType());
	ExpectGolden("GCSystemMessage", 0, WriteBody(src, 0));
}

// CLLogin is written by this client and read by the login server, whose
// read() consumes id, password, MAC and a trailing login-mode byte -
// exactly what write() here emits, and what the plain golden pins. The
// netmarble layout - a 4-byte ID length and no password - is pinned by
// golden only: neither repo's read() parses it. Both variants can be
// recorded because the flag is a packet member now (task 2.4 cut the
// g_pUserInformation read out of the wire class).
TEST(CLLogin, PlainLayoutMatchesGolden)
{
	CLLogin packet;
	Fill(packet);
	CHECK(EncrypterFree(packet));
	CHECK(!packet.isNetmarble());
	const std::vector<unsigned char> body = WriteBody(packet, 0);
	CHECK_EQ(packet.getPacketSize(), body.size());
	ExpectGolden("CLLogin", 0, body);
}

// Stated as a fact so its eventual change is deliberate: this repo's
// CLLogin::read() is a stale mirror of the server's and stops one byte
// short - it never consumes the login-mode byte write() emits and the
// server reads. Harmless today (the client never reads a CLLogin), so
// the test pins the asymmetry rather than a round-trip; when read() is
// brought in step, flip this to RoundTrip() and CHECK the fields.
TEST(CLLogin, ClientSideReadStopsOneByteShortOfWrite)
{
	CLLogin src, dst;
	Fill(src);
	std::vector<unsigned char> body = WriteBody(src, 0);

	InFixture f;
	SocketInputStreamTestAccess::Preload(f.m_Stream, &body[0], (unsigned int)body.size());
	dst.read(f.m_Stream);
	CHECK(src.getID() == dst.getID());
	CHECK(src.getPassword() == dst.getPassword());
	CHECK(std::memcmp(src.getMacAddress(), dst.getMacAddress(), 6) == 0);
	CHECK_EQ(1, f.m_Stream.length());	// the login-mode byte
}

TEST(CLLogin, NetmarbleLayoutMatchesGolden)
{
	CLLogin packet;
	Fill(packet);
	packet.setNetmarble(true);
	const std::vector<unsigned char> body = WriteBody(packet, 0);
	CHECK_EQ(packet.getPacketSize(), body.size());
	ExpectGolden("CLLogin.netmarble", 0, body);
}
