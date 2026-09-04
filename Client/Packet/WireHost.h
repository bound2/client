//----------------------------------------------------------------------
// WireHost.h
//----------------------------------------------------------------------
//
// What the wire layer needs from the program around it
// (docs/RESTRUCTURING.md task 5.1).
//
// The receive loops and the connection managers under Client/Packet are
// otherwise self-contained, but they read three tuning values out of
// the executable's config and send a bug report through its own
// executable's socket. Both are seams to code the library sits below,
// so the library asks for them instead: the executable fills a WireHost
// in and installs it, exactly as MItemHost and MPriceHost work for
// gamemodel.
//
// Every accessor answers without a host. The defaults are the ones
// ClientConfig's own constructor sets, so a binary that never installs
// a host - a test binary - behaves as a client whose config file was
// missing rather than as one whose tuning is zero.
//
//----------------------------------------------------------------------

#ifndef __WIREHOST_H__
#define __WIREHOST_H__

#include "Types.h"
#include "Types/ZoneTypes.h"

class Player;

//----------------------------------------------------------------------
// The answers with no host, which are ClientConfig's own constructor
// values. Named so that the executable's accessors can fall back to
// the same numbers rather than writing them a second time.
//----------------------------------------------------------------------
enum {
	WIRE_DEFAULT_MAX_PROCESS_PACKET		= 11,
	WIRE_DEFAULT_MAX_REQUEST_SERVICE	= 10,
	WIRE_DEFAULT_UDP_PORT			= 9858
};

//----------------------------------------------------------------------
// The host
//----------------------------------------------------------------------
struct WireHost {

	// How many packets one turn of a receive loop handles.
	int	(*MaxProcessPacket)();

	// How many request-service connections may be open at once.
	int	(*MaxRequestService)();

	// The UDP port the client communication manager binds.
	uint	(*ClientCommunicationUDPPort)();

	// Where a report about a malformed packet is sent. NULL while
	// there is no connection, which is not an error - the report is
	// dropped.
	Player*	(*BugReportTarget)();

	// The two values ClientPlayer::setEncryptCode() derives the stream
	// cipher's seed from: the zone the player is in, and the account's
	// server number.
	ZoneID_t	(*EncryptZoneID)();
	int		(*EncryptServerID)();

	// Which of the two seed formulas to use. The English client spaces
	// the server number differently; every other region shares one
	// formula. See WireEncryptSeed below.
	bool		(*EncryptUsesEnglishSeed)();

};

//----------------------------------------------------------------------
// The stream cipher's seed.
//----------------------------------------------------------------------
// Pulled out of ClientPlayer::setEncryptCode() so that it can be
// tested, which is the only thing about it that can be checked here:
// nothing in this repository or its build ever defines
// __USE_ENCRYPTER__, so the encrypted socket streams are never
// constructed and this function has no live caller. It is pinned rather
// than deleted because turning the encrypter on has to agree with the
// server, byte for byte.
//
// The original wrote four branches - Netmarble, Chinese, English, and a
// default - of which three computed the identical expression. Only the
// English one differs, multiplying the server number by 51 where the
// others shift it left by four.
//----------------------------------------------------------------------
uchar	WireEncryptSeed ( ZoneID_t zoneID , int serverID , bool bEnglishSeed ) throw ();

//----------------------------------------------------------------------
// The wire layer's view of it
//----------------------------------------------------------------------
class Wire {

public :

	// Installed once at start-up; NULL puts every default back.
	static void	SetHost ( const WireHost * pHost ) throw () { s_pHost = pHost; }

	static int	MaxProcessPacket () throw ();
	static int	MaxRequestService () throw ();
	static uint	ClientCommunicationUDPPort () throw ();
	static Player *	BugReportTarget () throw ();
	static ZoneID_t	EncryptZoneID () throw ();
	static int	EncryptServerID () throw ();
	static bool	EncryptUsesEnglishSeed () throw ();

private :

	static const WireHost *	s_pHost;

};

//----------------------------------------------------------------------
// Report a malformed packet to the server, as a chat message.
//----------------------------------------------------------------------
// The executable defined this and the wire layer called it, which is a
// seam no include rule and no ratchet over globals can see - only a
// failed link found it. It lives here now, and takes its target from
// the host.
//----------------------------------------------------------------------
void	SendBugReport ( const char * bug , ... );

#endif	// __WIREHOST_H__
