//----------------------------------------------------------------------
// test_player_base.cpp
//----------------------------------------------------------------------
//
// Player, the base of the two player classes (docs/RESTRUCTURING.md
// task 5.1): the send/receive plumbing over a socket, with no game
// state of its own. It became linkable into a test binary when the
// logging header moved into basic/ and the wire layer stopped reaching
// for the executable's debug facilities, so this file is first of all
// the proof that it links.
//
// What it can reach is the socket-free half. Everything past the
// default constructor - processInput, processOutput, sendPacket,
// disconnect, toString - dereferences the socket or a stream without
// testing either, and the constructor that takes a socket cannot be
// driven from here: its destructor closes the socket, and closesocket
// on a descriptor Winsock never handed out throws an Error the catch
// inside SocketImpl::close does not catch. The plan records both.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "Player.h"

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

	// The table is 512 bytes derived from the hash key alone, so the
	// same key gives the same table.
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

TEST(PlayerBase, SettingASocketOnAPlayerThatHasNoStreamsMakesNone)
{
	Player player;

	// setSocket replaces the streams only if they are there, and the
	// socket-free constructor makes none - so the player still has
	// nothing to read or write through, whatever socket it is given.
	player.setSocket(NULL);
	CHECK(player.getSocket() == NULL);

	// The key calls stay safe across that, since they test the streams
	// one at a time.
	player.setKey(1, 2);
	CHECK(player.pHashTable != NULL);
	player.delKey();
	CHECK(player.pHashTable == NULL);
}
