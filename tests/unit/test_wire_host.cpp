//----------------------------------------------------------------------
// test_wire_host.cpp
//----------------------------------------------------------------------
//
// What the wire layer asks of the program around it
// (docs/RESTRUCTURING.md task 5.1): three tuning values the receive
// loops and the connection managers used to read straight out of the
// executable's ClientConfig, and the connection a report about a
// malformed packet is sent to.
//
// The point of the seam is that the library answers without the
// executable, so most of what there is to test is what it answers when
// nobody has installed a host - which is the state a test binary, and
// a client whose config file is missing, are both in.
//
//----------------------------------------------------------------------

#include "test_framework.h"

#include "ClientCommunicationManager.h"
#include "ClientPlayer.h"
#include "RequestClientPlayer.h"
#include "RequestServerPlayer.h"
#include "RequestServerPlayerManager.h"
#include "Player.h"
#include "PacketDiagnostics.h"
#include "WireHost.h"

namespace {

int	s_MaxProcessPacket	= 0;
int	s_MaxRequestService	= 0;
uint	s_UDPPort		= 0;
Player*	s_pTarget		= NULL;

ZoneID_t	s_EncryptZoneID		= 0;
int		s_EncryptServerID	= 0;
bool		s_EnglishSeed		= false;

DWORD		s_Now			= 0;
bool		s_InGame		= false;

// The six file-transfer entries have the same signature in pairs, so a
// host wired to the wrong one would still answer. Recording the name
// makes the test say WHICH was reached.
std::string	s_RequestAsked;

void	Asked(const char* pWhich, const std::string& name)
{
	s_RequestAsked += pWhich;
	s_RequestAsked += "(";
	s_RequestAsked += name;
	s_RequestAsked += ") ";
}

int	HostMaxProcessPacket()		{ return s_MaxProcessPacket; }
int	HostMaxRequestService()		{ return s_MaxRequestService; }
uint	HostUDPPort()			{ return s_UDPPort; }
Player*	HostBugReportTarget()		{ return s_pTarget; }
ZoneID_t	HostEncryptZoneID()	{ return s_EncryptZoneID; }
int	HostEncryptServerID()		{ return s_EncryptServerID; }
bool	HostEnglishSeed()		{ return s_EnglishSeed; }

DWORD	HostCurrentTime()		{ return s_Now; }
bool	HostInGameMode()		{ return s_InGame; }

bool	HostReceiveMyRequest(const std::string& n, RequestClientPlayer*)	{ Asked("ReceiveMy", n); return true; }
bool	HostHasMyRequest(const std::string& n)				{ Asked("HasMy", n); return true; }
bool	HostRemoveMyRequest(const std::string& n)			{ Asked("RemoveMy", n); return true; }
bool	HostSendOtherRequest(const std::string& n, RequestServerPlayer*)	{ Asked("SendOther", n); return true; }
bool	HostHasOtherRequest(const std::string& n)			{ Asked("HasOther", n); return true; }
bool	HostRemoveOtherRequest(const std::string& n)			{ Asked("RemoveOther", n); return true; }

const WireHost	s_Host = { HostMaxProcessPacket, HostMaxRequestService, HostUDPPort, HostBugReportTarget,
				HostEncryptZoneID, HostEncryptServerID, HostEnglishSeed,
				HostCurrentTime, HostInGameMode,
				HostReceiveMyRequest, HostHasMyRequest, HostRemoveMyRequest,
				HostSendOtherRequest, HostHasOtherRequest, HostRemoveOtherRequest };

// A host that answers nothing, which is not the same as no host.
const WireHost	s_EmptyHost = { NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

// SendBugReport asks for a target only once it has decided the report
// is worth sending, so the count is the observable half of a function
// whose other half needs a connection.
int	s_Asked = 0;

Player*	CountingBugReportTarget()	{ s_Asked++; return s_pTarget; }

const WireHost	s_CountingHost = { HostMaxProcessPacket, HostMaxRequestService, HostUDPPort, CountingBugReportTarget,
					HostEncryptZoneID, HostEncryptServerID, HostEnglishSeed,
					HostCurrentTime, HostInGameMode,
					HostReceiveMyRequest, HostHasMyRequest, HostRemoveMyRequest,
					HostSendOtherRequest, HostHasOtherRequest, HostRemoveOtherRequest };

// Puts the library back the way every other test expects it.
struct NoHost
{
	~NoHost()	{ Wire::SetHost(NULL); }
};

} // namespace

//----------------------------------------------------------------------
// Without a host
//----------------------------------------------------------------------
TEST(WireHostSeam, TheDefaultsAreTheConfigsOwn)
{
	NoHost	restore;

	Wire::SetHost(NULL);

	// ClientConfig's constructor sets exactly these three, so a
	// library with no host behaves as a client whose config file was
	// missing rather than as one tuned to zero - a receive loop that
	// handled 0 packets a turn would take no packets at all.
	CHECK_EQ(11, Wire::MaxProcessPacket());
	CHECK_EQ(10, Wire::MaxRequestService());
	CHECK_EQ(9858, (int)Wire::ClientCommunicationUDPPort());

	// And there is nowhere to send a report, which is not an error.
	CHECK(Wire::BugReportTarget() == NULL);

	// The encrypt-seed inputs answer zero and the non-English formula,
	// which is what they would be before the player is in a zone.
	CHECK_EQ(0, (int)Wire::EncryptZoneID());
	CHECK_EQ(0, Wire::EncryptServerID());
	CHECK_EQ(false, Wire::EncryptUsesEnglishSeed());

	// A host that answers nothing answers the same way, one entry at a
	// time - the accessors test the pointer, not just the host.
	Wire::SetHost(&s_EmptyHost);
	CHECK_EQ(11, Wire::MaxProcessPacket());
	CHECK_EQ(10, Wire::MaxRequestService());
	CHECK_EQ(9858, (int)Wire::ClientCommunicationUDPPort());
	CHECK(Wire::BugReportTarget() == NULL);
	CHECK_EQ(0, (int)Wire::EncryptZoneID());
	CHECK_EQ(0, Wire::EncryptServerID());
	CHECK_EQ(false, Wire::EncryptUsesEnglishSeed());
}

TEST(WireHostSeam, AHostAnswersForTheProgramAroundIt)
{
	NoHost	restore;

	s_MaxProcessPacket	= 3;
	s_MaxRequestService	= 4;
	s_UDPPort		= 1234;
	s_pTarget		= NULL;

	Wire::SetHost(&s_Host);

	CHECK_EQ(3, Wire::MaxProcessPacket());
	CHECK_EQ(4, Wire::MaxRequestService());
	CHECK_EQ(1234, (int)Wire::ClientCommunicationUDPPort());

	// Read each time, not copied once: the config the executable reads
	// them from is built after start-up and deleted at shutdown.
	s_MaxProcessPacket = 99;
	CHECK_EQ(99, Wire::MaxProcessPacket());

	// Taking the host away puts the defaults back.
	Wire::SetHost(NULL);
	CHECK_EQ(11, Wire::MaxProcessPacket());
}

//----------------------------------------------------------------------
// The report itself
//----------------------------------------------------------------------
TEST(WireHostSeam, AReportIsBuiltOnlyWhenThereIsSomethingToSay)
{
	NoHost	restore;

	//------------------------------------------------------------------
	// This is the function whose definition lived in the executable
	// while the wire layer called it - the seam only a failed link
	// could see, and unreachable from a test binary until it moved.
	//
	// It asks the host for a target only after it has decided the
	// report is worth sending, so counting the asks says which reports
	// get that far without needing a Player to send one to.
	//------------------------------------------------------------------
	s_pTarget = NULL;
	s_Asked = 0;
	Wire::SetHost(&s_CountingHost);

	// Nothing to format.
	SendBugReport(NULL);
	CHECK_EQ(0, s_Asked);

	// A report that says nothing, and one that says one character:
	// both dropped before a packet is built (the length test is
	// `<= 1`, so a single character does not survive it either).
	SendBugReport("");
	SendBugReport("x");
	CHECK_EQ(0, s_Asked);

	// Two characters is a report.
	SendBugReport("xy");
	CHECK_EQ(1, s_Asked);

	// So is a formatted one, and a plain one.
	SendBugReport("%d %s", 7, "formatted");
	SendBugReport("a report with no connection to send it on");
	CHECK_EQ(3, s_Asked);

	// Longer than the 256-byte buffer it formats into: truncated
	// rather than overrun, and still a report.
	char	shout[600];
	for (int i = 0; i < 599; i++)
		shout[i] = 'A';
	shout[599] = '\0';
	SendBugReport("%s", shout);
	CHECK_EQ(4, s_Asked);

	// Every one of those was asked for a target and told there is
	// none, which is where they stopped.
	CHECK(Wire::BugReportTarget() == NULL);
	CHECK_EQ(5, s_Asked);		// the line above asked too
}

//----------------------------------------------------------------------
// The library's own diagnostic seam, which used to run through the
// executable and back
//----------------------------------------------------------------------
TEST(WireHostSeam, AWireDiagnosticReachesTheReporterWithNoHookInstalled)
{
	NoHost	restore;

	PacketDiagnostics::BugReportFn	pPrevious = PacketDiagnostics::getBugReportHook();

	s_pTarget = NULL;
	s_Asked = 0;
	Wire::SetHost(&s_CountingHost);
	PacketDiagnostics::setBugReportHook(NULL);

	// With no hook, a report from inside the library used to go
	// nowhere at all: the hook was how it left, and the executable
	// installed one that called straight back into this library. It
	// goes to SendBugReport directly now.
	PacketDiagnostics::reportBug("too large PacketSize ID)%d %d/%d", 1, 2, 3);
	CHECK_EQ(1, s_Asked);

	// A hook still intercepts, and nothing reaches the reporter then.
	PacketDiagnostics::setBugReportHook(pPrevious);
}

//----------------------------------------------------------------------
// The file the seam let into the library
//----------------------------------------------------------------------
TEST(WireHostSeam, TheCommunicationManagersObjectIsInTheLibrary)
{
	// Same link proof as DatagramSocket's, and for the same reason: a
	// static library hands the linker only the objects that resolve
	// something. Its constructor opens a UDP socket on the configured
	// port, so nothing here builds one.
	void (ClientCommunicationManager::*pUpdate)() = &ClientCommunicationManager::Update;
	void (ClientCommunicationManager::*pSend)(Datagram*) = &ClientCommunicationManager::sendDatagram;

	CHECK(pUpdate != NULL);
	CHECK(pSend != NULL);
}

TEST(WireHostSeam, AHostAnswersTheEncryptSeedInputs)
{
	NoHost	restore;

	s_EncryptZoneID		= 0x1234;
	s_EncryptServerID	= 7;
	s_EnglishSeed		= true;

	Wire::SetHost(&s_Host);

	CHECK_EQ(0x1234, (int)Wire::EncryptZoneID());
	CHECK_EQ(7, Wire::EncryptServerID());
	CHECK_EQ(true, Wire::EncryptUsesEnglishSeed());

	// Read each time. The zone changes under the client while the
	// connection stays open, so a value copied once would be the wrong
	// one for every zone after the first.
	s_EncryptZoneID = 0x0009;
	CHECK_EQ(9, (int)Wire::EncryptZoneID());
}

//----------------------------------------------------------------------
// The stream cipher's seed
//----------------------------------------------------------------------
//
// Pinned rather than trusted, because nothing exercises it: no target
// in this build defines __USE_ENCRYPTER__, so the only caller -
// ClientPlayer::setEncryptCode() - is not compiled. If the encrypter is
// ever turned back on, these are the bytes the server has to agree
// with, and the expectations below were computed from the expression
// that stood in ClientPlayer.cpp before it moved.
//----------------------------------------------------------------------
TEST(WireEncryptSeed, MatchesTheExpressionItWasExtractedFrom)
{
	// ((zone >> 8) ^ zone) ^ ((server + 1) << 4)
	// zone 0x1234: 0x12 ^ 0x34 = 0x26; server 7: 8 << 4 = 0x80
	CHECK_EQ(0xA6, (int)WireEncryptSeed(0x1234, 7, false));

	// The English formula multiplies by 51 instead: 8 * 51 = 408,
	// truncated to a uchar after the xor with 0x26.
	CHECK_EQ((int)(uchar)(0x26 ^ 408), (int)WireEncryptSeed(0x1234, 7, true));

	// Zone 0 and server 0 - the answer with no host installed.
	CHECK_EQ(0x10, (int)WireEncryptSeed(0, 0, false));
	CHECK_EQ(51, (int)WireEncryptSeed(0, 0, true));
}

TEST(WireEncryptSeed, TheThreeNonEnglishRegionsAgreedAndStillDo)
{
	// The function this replaced wrote four branches - Netmarble,
	// Chinese, English and a default - of which only the English one
	// differed. Collapsing them is only safe if that is true, so it is
	// asserted rather than asserted-in-a-comment: for every server
	// number a byte can hold, the non-English answer is the shift form.
	for (int server = 0; server < 256; ++server)
	{
		const uchar expected = (uchar)( ( ( 0xBEEF >> 8 ) ^ 0xBEEF ) ^ ( ( server + 1 ) << 4 ) );

		CHECK_EQ((int)expected, (int)WireEncryptSeed(0xBEEF, server, false));
	}
}

//----------------------------------------------------------------------
// The file this slice let into the library
//----------------------------------------------------------------------
TEST(WireHostSeam, TheGameConnectionsObjectIsInTheLibrary)
{
	// The link proof for ClientPlayer.cpp, whose last two game-code
	// includes went behind the host above. Constructing one would open
	// a socket, so this takes addresses instead - but of NON-VIRTUAL
	// members. A pointer to a virtual member is a vtable index and need
	// not reference the defining object at all, so `processCommand`
	// and `disconnect`, which is what this test reached for first,
	// would have proved nothing. Verified by taking the file back out
	// of tests/arch/packetwire_files.txt: with these two the link
	// fails, with the virtual pair it succeeded.
	void (ClientPlayer::*pStatus)(PlayerStatus) = &ClientPlayer::setPlayerStatus;
	Packet* (ClientPlayer::*pOldPacket)(uint) = &ClientPlayer::getOldPacket;

	CHECK(pStatus != NULL);
	CHECK(pOldPacket != NULL);
}

//----------------------------------------------------------------------
// The request-service family's seams
//----------------------------------------------------------------------
TEST(WireHostSeam, TheRequestSeamsAnswerConservativelyWithNoHost)
{
	NoHost	restore;

	Wire::SetHost(NULL);

	// A clock of zero, and NOT in the game world. The second is the
	// conservative answer rather than the convenient one:
	// RequestClientPlayer throws on a request packet that arrives
	// outside the game, so a binary with no host refuses them all
	// rather than accepting them all.
	CHECK_EQ(0, (int)Wire::CurrentTime());
	CHECK_EQ(false, Wire::InGameMode());

	// And no file transfer is registered, which is what a caller
	// asking whether it still has one needs to hear so that it cleans
	// up instead of waiting on a manager that is not there.
	CHECK_EQ(false, Wire::ReceiveMyRequest("peer", NULL));
	CHECK_EQ(false, Wire::HasMyRequest("peer"));
	CHECK_EQ(false, Wire::RemoveMyRequest("peer"));
	CHECK_EQ(false, Wire::SendOtherRequest("peer", NULL));
	CHECK_EQ(false, Wire::HasOtherRequest("peer"));
	CHECK_EQ(false, Wire::RemoveOtherRequest("peer"));

	// A host that answers nothing answers the same way, one entry at a
	// time - the accessors test the pointer, not just the host.
	Wire::SetHost(&s_EmptyHost);
	CHECK_EQ(0, (int)Wire::CurrentTime());
	CHECK_EQ(false, Wire::InGameMode());
	CHECK_EQ(false, Wire::HasMyRequest("peer"));
	CHECK_EQ(false, Wire::HasOtherRequest("peer"));
}

TEST(WireHostSeam, AHostAnswersTheRequestSeamsAndIsAskedTheRightOne)
{
	NoHost	restore;

	s_Now		= 4321;
	s_InGame	= true;
	s_RequestAsked.clear();

	Wire::SetHost(&s_Host);

	CHECK_EQ(4321, (int)Wire::CurrentTime());
	CHECK_EQ(true, Wire::InGameMode());

	// Read each time. The request timeouts are differences against this
	// clock, so a value copied once would freeze every one of them.
	s_Now = 9999;
	CHECK_EQ(9999, (int)Wire::CurrentTime());

	// The six file-transfer calls are near-identical in shape, which is
	// exactly how one gets wired to the wrong host entry. Each records
	// its own name, so the test says which was reached rather than only
	// that something was.
	Wire::ReceiveMyRequest("a", NULL);
	Wire::HasMyRequest("b");
	Wire::RemoveMyRequest("c");
	Wire::SendOtherRequest("d", NULL);
	Wire::HasOtherRequest("e");
	Wire::RemoveOtherRequest("f");

	CHECK(s_RequestAsked ==
		"ReceiveMy(a) HasMy(b) RemoveMy(c) SendOther(d) HasOther(e) RemoveOther(f) ");
}

//----------------------------------------------------------------------
// The three files this slice let into the library
//----------------------------------------------------------------------
TEST(WireHostSeam, TheRequestServiceObjectsAreInTheLibrary)
{
	// Non-virtual members whose definitions are in the .cpp files, for
	// the reason the ClientPlayer proof below records: a pointer to a
	// virtual member is a vtable index and need not reference the
	// defining object, and an inline member in the header is not in the
	// object at all. Constructing any of these would open a socket.
	uint (RequestClientPlayer::*pLen)() const = &RequestClientPlayer::getInputStreamLength;
	uint (RequestServerPlayer::*pSend)(const char*, uint) = &RequestServerPlayer::send;
	void (RequestServerPlayerManager::*pInit)(int) = &RequestServerPlayerManager::Init;
	void (RequestServerPlayerManager::*pWait)() = &RequestServerPlayerManager::WaitRequest;

	CHECK(pLen != NULL);
	CHECK(pSend != NULL);
	CHECK(pInit != NULL);
	CHECK(pWait != NULL);
}
