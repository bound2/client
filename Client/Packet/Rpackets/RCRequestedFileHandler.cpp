//////////////////////////////////////////////////////////////////////
//
// Filename    : RCRequestedFileHandler.cc
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "Client_PCH.h"
#include "RCRequestedFile.h"
#include "RequestClientPlayer.h"
#include "RequestFileManager.h"
#include "ClientDef.h"

//////////////////////////////////////////////////////////////////////
//
// The peer here is another game client, so the filename is fully
// untrusted, and ReceiveFileInfo creates, truncates, renames and
// deletes whatever path it is given. The legitimate wire value is the
// sender's own relative profile path (CRRequestHandler sends
// ProfileManager::GetFilename, e.g. "profile\name.spk"), so interior
// separators must pass; what must never pass is anything that escapes
// the working directory -- a drive letter, an absolute path, or a
// ".." component. A dot is also required because StartReceive patches
// the extension in place via rfind(".").
//
//////////////////////////////////////////////////////////////////////
static bool isSafeRequestFilename(const std::string& filename)
{
	if (filename.empty())
		return false;

	if (filename.find(':') != std::string::npos)
		return false;

	if (filename[0] == '/' || filename[0] == '\\')
		return false;

	if (filename.find("..") != std::string::npos)
		return false;

	if (filename.find('.') == std::string::npos)
		return false;

	// Win32 quirks: a trailing dot or space is stripped by the filesystem
	// (aliasing a different name than the one checked), and a reserved
	// device base name (CON, NUL, COM1, ...) opens the device no matter
	// what extension follows it.
	char last = filename[filename.size()-1];
	if (last == '.' || last == ' ')
		return false;

	size_t base = filename.find_last_of("/\\");
	base = (base == std::string::npos) ? 0 : base+1;
	size_t baseEnd = filename.find('.', base);
	std::string baseName = filename.substr(base, baseEnd-base);
	for (size_t i=0; i<baseName.size(); i++)
		baseName[i] = toupper((unsigned char)baseName[i]);

	static const char* const reserved[] =
	{
		"CON", "PRN", "AUX", "NUL",
		"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
		"LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
	};
	for (size_t r=0; r<sizeof(reserved)/sizeof(reserved[0]); r++)
	{
		if (baseName == reserved[r])
			return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
//
// 클라이언트에서 서버로부터 메시지를 받았을때 실행되는 메쏘드이다.
//
//////////////////////////////////////////////////////////////////////
void RCRequestedFileHandler::execute ( RCRequestedFile * pPacket , Player * pPlayer )
	 
throw ( ProtocolException , Error )
{
	__BEGIN_TRY
	
#ifdef __GAME_CLIENT__

	RequestClientPlayer* pRequestClientPlayer = dynamic_cast<RequestClientPlayer*>( pPlayer );

	if (pRequestClientPlayer!=NULL
		&& g_pRequestFileManager!=NULL)
	{
		RequestReceiveInfo* pInfo = new RequestReceiveInfo( pRequestClientPlayer->getRequestServerName().c_str() );
				
		int listNum = pPacket->getListNum();

		for (int i=0; i<listNum; i++)
		{
			RCRequestedFileInfo* pFileInfo = pPacket->popInfo();

			if (pFileInfo!=NULL)
			{
				const std::string filename = pFileInfo->getFilename();

				// Dropping a single file would desynchronise the transfer
				// stream, so a hostile name aborts the whole exchange.
				// pInfo owns the ReceiveFileInfos started so far and their
				// destructors close the open streams, so it must be freed
				// before the throw.
				if (!isSafeRequestFilename( filename ))
				{
					DEBUG_ADD_FORMAT("[Error] RCRequestedFile: unsafe filename from peer: %s", filename.c_str());
					delete pFileInfo;
					delete pInfo;
					throw DisconnectException("unsafe requested filename");
				}

				ReceiveFileInfo* pReceiveFileInfo = new ReceiveFileInfo( filename.c_str(), pFileInfo->getRequestFileType() );

				pInfo->AddReceiveFileInfo( pReceiveFileInfo );

				pReceiveFileInfo->StartReceive( pFileInfo->getFileSize() );

				delete pFileInfo;
			}
		}
		
		if (g_pRequestFileManager->AddMyRequest(pInfo))
		{
			// 받기 시작할 준비를 한다.
			//pInfo->StartReceive( pFileInfo->getFileSize() );
		}
		else
		{
			//pRequestClientPlayer->disconnect( UNDISCONNECTED );
			throw DisconnectException("can't add myRequest");
		}
	}

#endif

	__END_CATCH
}
