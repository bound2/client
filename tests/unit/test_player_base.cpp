//----------------------------------------------------------------------
// test_player_base.cpp
//----------------------------------------------------------------------
//
// The two files task 5.1 moved into packetwire: Player, the base under
// all three player classes - the send/receive plumbing over a socket,
// with no game state of its own - and DatagramSocket. Both became
// linkable into a test binary when the logging header moved into
// basic/ and the wire layer stopped reaching for the executable's
// debug facilities, so this file is first of all the proof that they
// link.
//
// Most of Player still cannot be driven from here: processCommand,
// processInput, processOutput, sendPacket, disconnect and toString all
// dereference a stream or the socket without testing either, and a
// player holding a real socket would have to be given a peer. What is
// reachable is the two constructors, the id, the socket accessors and
// the encryption table.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "packet_stream_access.h"		// EnsureSocketsInitialised
#include "DatagramSocket.h"
#include "Player.h"
#include "Socket.h"
#include "SocketImpl.h"

#include <cstring>

//----------------------------------------------------------------------
// The socket-free constructor
//----------------------------------------------------------------------
TEST(PlayerBase, ADefaultPlayerHoldsNoSocketAndNoKey)
{
	Player player;

	// Nothing to send or receive through yet.
	CHECK(player.getSocket() == NULL);

	// And no encryption table, which the key calls free.
	CHECK(player.pHashTable == NULL);

	// The id is the one piece of state that needs no socket.
	CHECK(player.getID().empty());
	player.setID("tester");
	CHECK_EQ(0, std::strcmp("tester", player.getID().c_str()));
}

TEST(PlayerBase, TheEncryptionTableFollowsTheHashKeyAndIsGivenBack)
{
	Player player;

	// Both streams are absent, so setKey builds the table and stops
	// there - which is what makes it reachable without a socket.
	player.setKey(7, 1234);
	CHECK(player.pHashTable != NULL);

	// The table is 512 bytes walked out of an 8-bit state seeded with
	// (HashKey + 4658) & 0xFF, so it follows the hash key modulo 256
	// and nothing else - the encrypt key only picks an offset into it.
	// The same key therefore gives the same table, and 1234 against 99
	// below differ modulo 256; 1234 against 1490 would not.
	BYTE first[512];
	std::memcpy(first, player.pHashTable, 512);

	player.delKey();
	CHECK(player.pHashTable == NULL);

	// Giving back a table that is not there is not an error.
	player.delKey();
	CHECK(player.pHashTable == NULL);

	player.setKey(7, 1234);
	CHECK(player.pHashTable != NULL);
	CHECK_EQ(0, std::memcmp(first, player.pHashTable, 512));

	// A different hash key gives a different table.
	player.delKey();
	player.setKey(7, 99);
	CHECK(player.pHashTable != NULL);
	CHECK(std::memcmp(first, player.pHashTable, 512) != 0);

	player.delKey();
}

TEST(PlayerBase, SettingASocketKeepsIt)
{
	Player player;

	// A socket the player is only ever asked to hand back, never to
	// read or write: setSocket replaces the streams only if they are
	// there, and the socket-free constructor made none, so nothing
	// dereferences it.
	Socket*	pFake = (Socket*)0x1;

	player.setSocket(pFake);
	CHECK(pFake == player.getSocket());

	// Put it back before the player is destroyed - the destructor
	// closes and deletes whatever socket it holds.
	player.setSocket(NULL);
	CHECK(player.getSocket() == NULL);

	// The key calls stay safe across that, since they test the streams
	// one at a time.
	player.setKey(1, 2);
	CHECK(player.pHashTable != NULL);
	player.delKey();
	CHECK(player.pHashTable == NULL);
}

//----------------------------------------------------------------------
// The constructor that takes a socket
//----------------------------------------------------------------------
TEST(PlayerBase, ASocketBuiltPlayerHasNoKeyToGiveBack)
{
	//------------------------------------------------------------------
	// The constructor RequestClientPlayer and RequestServerPlayer both
	// forward to. It set every member but pHashTable, which delKey
	// then delete[]s - and delKey has two live callers, the reconnect
	// handlers. They are safe only because ClientPlayer, the class
	// they cast to, leaves its base default-constructed; a base
	// initialiser on that one class would have made every reconnect a
	// free of whatever the memory held.
	//------------------------------------------------------------------
	EnsureSocketsInitialised();

	{
		// A socket over a descriptor nothing ever created: the player
		// takes ownership and closes it, which Winsock answers with
		// the exception SocketImpl::close catches.
		Player	player(new Socket(new SocketImpl()));

		CHECK(player.pHashTable == NULL);
		CHECK(player.getSocket() != NULL);

		// So giving back a key it was never given is not a free of a
		// wild pointer.
		player.delKey();
		CHECK(player.pHashTable == NULL);

		// And a table it is given really is given back - the streams
		// exist here, so setKey reaches their (no-op) setKey too.
		player.setKey(3, 77);
		CHECK(player.pHashTable != NULL);
		player.delKey();
		CHECK(player.pHashTable == NULL);

		// Asking twice replaces the table rather than stranding the
		// first one; the destructor frees whatever is left.
		player.setKey(3, 77);
		player.setKey(4, 88);
		CHECK(player.pHashTable != NULL);
	}
}

//----------------------------------------------------------------------
// The other file this slice moved
//----------------------------------------------------------------------
TEST(DatagramSocketLink, ItsObjectIsInTheLibrary)
{
	//------------------------------------------------------------------
	// A static library hands the linker only the objects that resolve
	// something, so a member nothing references is never pulled in and
	// a build says nothing about whether it could have been. Taking
	// the address of each entry point is what forces it: the whole
	// object has to come in, and with it every symbol it references -
	// which is how the wire layer's one remaining seam to the
	// executable, Player::processCommand's SendBugReport, showed up.
	//
	// Nothing here opens a socket. Both constructors do, and the
	// server one binds a port, which is not a thing a unit test should
	// need to be free.
	//------------------------------------------------------------------
	uint (DatagramSocket::*pSend)(Datagram*) = &DatagramSocket::send;
	Datagram* (DatagramSocket::*pReceive)() = &DatagramSocket::receive;
	SOCKET (DatagramSocket::*pGet)() const = &DatagramSocket::getSOCKET;

	CHECK(pSend != NULL);
	CHECK(pReceive != NULL);
	CHECK(pGet != NULL);

	// Its buffer is sized for the largest datagram it can be handed.
	CHECK(sizeof(DatagramSocket) > DATAGRAM_SOCKET_BUFFER_LEN);
}
