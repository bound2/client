//----------------------------------------------------------------------
// test_packet_span_io.cpp
//----------------------------------------------------------------------
//
// C++20 span contracts for the packet socket streams. The legacy
// pointer/length overloads cannot know the destination capacity; these tests
// pin the bounded overloads that new and migrated packet code should use.
//
//----------------------------------------------------------------------

#include "test_framework.h"
#include "packet_stream_access.h"

#include "Exception.h"
#include "Socket.h"
#include "SocketImpl.h"

#include <array>
#include <cstddef>
#include <span>

namespace {

struct SpanStreamFixture
{
	Socket			m_Socket;
	SocketInputStream	m_Input;
	SocketOutputStream	m_Output;

	SpanStreamFixture()
	: m_Socket((EnsureSocketsInitialised(), new SocketImpl())),
	  m_Input(&m_Socket, 16),
	  m_Output(&m_Socket, 16)
	{
	}
};

} // namespace

TEST(SocketInputStream, SpanReadRejectsLengthBeyondDestination)
{
	SpanStreamFixture f;
	const unsigned char bytes[] = { 0x11, 0x22, 0x33, 0x44 };
	SocketInputStreamTestAccess::Preload(f.m_Input, bytes, sizeof(bytes));

	std::array<char, 3> destination = { (char)0x7A, (char)0x7A, (char)0x7A };
	bool bThrew = false;
	try {
		f.m_Input.read(std::span<char>(destination), 4);
	} catch (InvalidProtocolException&) {
		bThrew = true;
	}

	CHECK(bThrew);
	CHECK_EQ(4, f.m_Input.length());
	CHECK_EQ(0x7A, (unsigned char)destination[0]);
	CHECK_EQ(0x7A, (unsigned char)destination[1]);
	CHECK_EQ(0x7A, (unsigned char)destination[2]);
}

TEST(SocketInputStream, ByteSpanReadReassemblesWrappedData)
{
	SpanStreamFixture f;
	const unsigned char bytes[] = { 0x91, 0xA2, 0xB3, 0xC4 };
	SocketInputStreamTestAccess::Preload(f.m_Input, bytes, sizeof(bytes), 14);

	std::array<std::byte, 4> destination;
	f.m_Input.read(std::span<std::byte>(destination));

	for (size_t i = 0; i < destination.size(); i++)
		CHECK_EQ(bytes[i], std::to_integer<unsigned char>(destination[i]));
	CHECK(f.m_Input.isEmpty());
}

TEST(SocketOutputStream, ByteSpanWritePreservesExactBytes)
{
	SpanStreamFixture f;
	const std::array<std::byte, 4> source = {
		std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE}, std::byte{0xEF}
	};

	f.m_Output.write(std::span<const std::byte>(source));
	const std::vector<unsigned char> actual =
		SocketOutputStreamTestAccess::Bytes(f.m_Output);

	CHECK_EQ(source.size(), actual.size());
	for (size_t i = 0; i < source.size(); i++)
		CHECK_EQ(std::to_integer<unsigned char>(source[i]), actual[i]);
}
