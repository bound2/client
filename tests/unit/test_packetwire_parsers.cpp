//----------------------------------------------------------------------
// test_packetwire_parsers.cpp
//----------------------------------------------------------------------
//
// First tests against the packetwire library (docs/RESTRUCTURING.md,
// task 1.2). The wire info classes parse server-supplied counts in a
// loop, and their only defense against a hostile or corrupt count is
// the stream's own underflow check: SocketInputStream::read throws
// InsufficientDataException before touching bytes it does not have.
// These tests pin that contract from both sides - the stream's bounds
// behavior directly, and the parsers' reliance on it.
//
// Test seam: fill() is the only production writer of the stream's ring
// buffer and needs a connected peer, so SocketInputStreamTestAccess
// (befriended by the class) preloads the buffer directly. The Socket
// handed to the stream is never used - the constructor only asserts it
// is non-null - but its destructor closes an INVALID_SOCKET through
// Winsock, so Winsock must be initialised or that close throws Error
// out of a destructor.
//
// This file compiles with the same defines as the packetwire library
// (__GAME_CLIENT__=1, and __WIN32__/__WINDOWS__ on Windows - set in
// tests/CMakeLists.txt) so every shared class definition is identical
// to the library's.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "SocketInputStream.h"
#include "Socket.h"
#include "SocketImpl.h"
#include "ModifyInfo.h"
#include "InventoryInfo.h"
#include "Exception.h"

#include <string>
#include <string.h>

#ifdef _WIN32
#include <winsock.h>
#endif

//----------------------------------------------------------------------
// Befriended by SocketInputStream: writes test bytes into the private
// ring buffer, optionally starting at a nonzero head so a read has to
// reassemble across the wrap point.
//----------------------------------------------------------------------
class SocketInputStreamTestAccess
{
public:
	static void Preload(SocketInputStream& stream, const unsigned char* data,
			    unsigned int len, unsigned int head = 0)
	{
		// The ring keeps one slot empty to distinguish full from empty.
		CHECK(len < stream.m_BufferLen);

		for (unsigned int i = 0; i < len; i++)
			stream.m_Buffer[(head + i) % stream.m_BufferLen] = (char)data[i];

		stream.m_Head = head;
		stream.m_Tail = (head + len) % stream.m_BufferLen;
	}
};

namespace {

//----------------------------------------------------------------------
// One-time Winsock init (see the file header). MAKEWORD(2,2) matches
// what any modern Windows provides; on other platforms this is a no-op.
//----------------------------------------------------------------------
void EnsureSocketsInitialised()
{
#ifdef _WIN32
	static bool bDone = false;
	if (!bDone)
	{
		WSADATA data;
		WSAStartup(MAKEWORD(2, 2), &data);
		bDone = true;
	}
#endif
}

//----------------------------------------------------------------------
// Stream fixture: a small ring over a never-used socket.
//----------------------------------------------------------------------
struct StreamFixture
{
	Socket				m_Socket;
	SocketInputStream	m_Stream;

	explicit StreamFixture(uint bufferLen = 64)
	: m_Socket((EnsureSocketsInitialised(), new SocketImpl())),
	  m_Stream(&m_Socket, bufferLen)
	{
	}

	void Preload(const unsigned char* data, unsigned int len, unsigned int head = 0)
	{
		SocketInputStreamTestAccess::Preload(m_Stream, data, len, head);
	}
};

//----------------------------------------------------------------------
// ModifyInfo is abstract (it inherits Packet's pure execute() and
// getPacketID()); the concrete subclasses are the GC packets. This
// minimal subclass exists only so read() can run against a real object.
//----------------------------------------------------------------------
class TestModifyInfo : public ModifyInfo
{
public:
	void execute(Player*) { }
	PacketID_t getPacketID() const  { return 0; }
};

} // namespace

//----------------------------------------------------------------------
// SocketInputStream contract
//----------------------------------------------------------------------

TEST(SocketInputStream, ZeroLengthReadIsRejected)
{
	StreamFixture f;
	const unsigned char bytes[] = { 0x01, 0x02 };
	f.Preload(bytes, sizeof(bytes));

	bool bThrew = false;
	char out[4];
	try {
		f.m_Stream.read(out, 0);
	} catch (InvalidProtocolException&) {
		bThrew = true;
	}
	CHECK(bThrew);
}

TEST(SocketInputStream, ReadBeyondAvailableThrowsAndConsumesNothing)
{
	StreamFixture f;
	const unsigned char bytes[] = { 0xAA, 0xBB, 0xCC };
	f.Preload(bytes, sizeof(bytes));

	bool bThrew = false;
	char out[8];
	try {
		f.m_Stream.read(out, 4);
	} catch (InsufficientDataException&) {
		bThrew = true;
	}
	CHECK(bThrew);

	// The guard fires before the copy, so the 3 bytes are still there
	// and still readable.
	CHECK_EQ(3, f.m_Stream.length());
	f.m_Stream.read(out, 3);
	CHECK_EQ(0xAA, (unsigned char)out[0]);
	CHECK_EQ(0xBB, (unsigned char)out[1]);
	CHECK_EQ(0xCC, (unsigned char)out[2]);
}

TEST(SocketInputStream, WrappedReadReassemblesAcrossBufferEnd)
{
	StreamFixture f(16);
	const unsigned char bytes[] = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60 };

	// Head at 12 of 16: bytes 12..15 sit at the end, 16..17 wrap to 0..1.
	f.Preload(bytes, sizeof(bytes), 12);
	CHECK_EQ(6, f.m_Stream.length());

	char out[6];
	f.m_Stream.read(out, 6);
	CHECK_EQ(0, memcmp(out, bytes, 6));
	CHECK(f.m_Stream.isEmpty());
}

//----------------------------------------------------------------------
// ModifyInfo::read - [shortCount:1][(type:1,value:2)...]
//                    [longCount:1][(type:1,value:4)...]
//----------------------------------------------------------------------

TEST(ModifyInfo, ReadParsesShortAndLongLists)
{
	StreamFixture f;
	const unsigned char bytes[] = {
		0x02,				// shortCount = 2
		0x03, 0x34, 0x12,		// type 3,  value 0x1234 (LE)
		0x17, 0xFF, 0xFF,		// type 23, value 0xFFFF
		0x01,				// longCount = 1
		0x2F, 0xEF, 0xBE, 0xAD, 0xDE,	// type 47, value 0xDEADBEEF (LE)
	};
	f.Preload(bytes, sizeof(bytes));

	TestModifyInfo info;
	info.read(f.m_Stream);

	CHECK_EQ(2, info.getShortCount());
	CHECK_EQ(1, info.getLongCount());

	SHORTDATA shortData;
	info.popShortData(shortData);
	CHECK_EQ(3, shortData.type);
	CHECK_EQ(0x1234, shortData.value);
	info.popShortData(shortData);
	CHECK_EQ(23, shortData.type);
	CHECK_EQ(0xFFFF, shortData.value);

	LONGDATA longData;
	info.popLongData(longData);
	CHECK_EQ(47, longData.type);
	CHECK_EQ(0xDEADBEEFLL, (long long)longData.value);

	// Everything consumed - exactly getPacketSize() bytes.
	CHECK(f.m_Stream.isEmpty());
}

TEST(ModifyInfo, OversizedShortCountHitsTheUnderflowGuard)
{
	StreamFixture f;

	// A hostile count with no data behind it: the first entry's read
	// must throw instead of parsing garbage.
	const unsigned char bytes[] = { 0xC8 };	// shortCount = 200
	f.Preload(bytes, sizeof(bytes));

	TestModifyInfo info;
	bool bThrew = false;
	try {
		info.read(f.m_Stream);
	} catch (InsufficientDataException&) {
		bThrew = true;
	}
	CHECK(bThrew);
}

//----------------------------------------------------------------------
// InventoryInfo::read - [listNum:1][InventorySlotInfo...]
//----------------------------------------------------------------------

TEST(InventoryInfo, ZeroCountParsesToEmptyList)
{
	StreamFixture f;
	const unsigned char bytes[] = { 0x00 };
	f.Preload(bytes, sizeof(bytes));

	InventoryInfo info;
	info.read(f.m_Stream);

	CHECK_EQ(0, info.getListNum());
	CHECK(f.m_Stream.isEmpty());
}

TEST(InventoryInfo, OversizedCountHitsTheUnderflowGuard)
{
	StreamFixture f;

	// 60 is the count getMaxSize() budgets for; with no slot data the
	// nested InventorySlotInfo read must hit the underflow guard.
	const unsigned char bytes[] = { 0x3C };
	f.Preload(bytes, sizeof(bytes));

	InventoryInfo info;
	bool bThrew = false;
	try {
		info.read(f.m_Stream);
	} catch (InsufficientDataException&) {
		bThrew = true;
	}
	CHECK(bThrew);
}
