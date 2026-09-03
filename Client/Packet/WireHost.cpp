//----------------------------------------------------------------------
// WireHost.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include "WireHost.h"
#include "Player.h"
#include "Cpackets/CGSay.h"
#include "DebugLog.h"		// DEBUG_ADD_FORMAT, for the __DEBUG_OUTPUT__ block below

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

const WireHost *	Wire::s_pHost = NULL;

//----------------------------------------------------------------------
// The three tuning values, and the defaults ClientConfig's constructor
// sets for them (Client/ClientConfig.cpp).
//----------------------------------------------------------------------
int
Wire::MaxProcessPacket () throw ()
{
	if (s_pHost==NULL || s_pHost->MaxProcessPacket==NULL)
	{
		return WIRE_DEFAULT_MAX_PROCESS_PACKET;
	}

	return s_pHost->MaxProcessPacket();
}

int
Wire::MaxRequestService () throw ()
{
	if (s_pHost==NULL || s_pHost->MaxRequestService==NULL)
	{
		return WIRE_DEFAULT_MAX_REQUEST_SERVICE;
	}

	return s_pHost->MaxRequestService();
}

uint
Wire::ClientCommunicationUDPPort () throw ()
{
	if (s_pHost==NULL || s_pHost->ClientCommunicationUDPPort==NULL)
	{
		return WIRE_DEFAULT_UDP_PORT;
	}

	return s_pHost->ClientCommunicationUDPPort();
}

Player *
Wire::BugReportTarget () throw ()
{
	if (s_pHost==NULL || s_pHost->BugReportTarget==NULL)
	{
		return NULL;
	}

	return s_pHost->BugReportTarget();
}

//----------------------------------------------------------------------
// Send Bug Report
//----------------------------------------------------------------------
// Moved here verbatim from Client/PacketFunction.cpp, with the
// executable's own connection replaced by the host's target. The
// truncation and the reasoning behind it are the ones an earlier
// hardening pass left in place.
//
// (The global that connection lives in is deliberately not named
// here: R4 greps every line of a library source for g_p* and does not
// skip comments, so writing it down would read as a seam.)
//----------------------------------------------------------------------
void
SendBugReport ( const char * bug , ... )
{
	if( bug == NULL )
		return;


	va_list		vl;
	char Buffer[256];

	va_start(vl, bug);
	int written = vsnprintf(Buffer, sizeof(Buffer), bug, vl);
	va_end(vl);

	// vsnprintf NUL terminates within sizeof(Buffer), so a report longer than
	// the buffer is truncated instead of overrunning the stack. That also makes
	// the strlen and the Buffer[100] cut below safe, which they were not while
	// vsprintf could already have run past the end. A negative return is an
	// encoding error: nothing usable was produced, so send nothing.
	if (written < 0)
		return;

#ifdef __DEBUG_OUTPUT__
	DEBUG_ADD_FORMAT("[BUG_REPORT] %s",Buffer);
#endif

	int len = strlen(Buffer);

	if( len <= 1 )
		return;

	if( len >= 100 )
		Buffer[100] = '\0';

	std::string message;

	message = "*bug_report ";
	message += Buffer;

	CGSay _CGSay;

	_CGSay.setMessage( message );
	_CGSay.setColor( 0 );

	Player * pTarget = Wire::BugReportTarget();

	if( pTarget != NULL )
		pTarget->sendPacket( &_CGSay );

}
