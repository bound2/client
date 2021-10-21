//--------------------------------------------------------------------------
// MZLib.cpp
//--------------------------------------------------------------------------
#include "Client_PCH.h"
#include "MZLib.h"
#include <assert.h>
#include "zlib.h"

#ifdef __GAME_CLIENT__
	extern void	SetProgressBarCount(int count);
	extern void	UpdateProgressBar();
#endif

//--------------------------------------------------------------------------
// static
//--------------------------------------------------------------------------
char*	MZLib::m_pPackBuffer = NULL;
char*	MZLib::m_pFileBuffer = NULL;
char	MZLib::m_ZLibHeader[MZLIB_HEADER_SIZE+1] = "MZLIB-1102";


//--------------------------------------------------------------------------
//
//							MFileInfo
//
//--------------------------------------------------------------------------
MFileInfo::MFileInfo()
{
	m_StartBuffer = 0;
	m_CompressSize = 0;
	m_Filesize = 0;
}

MFileInfo::~MFileInfo()
{
}

bool
MFileInfo::Set( const char* filename )
{
	m_Filename = filename;

	std::ifstream file(filename, ios::binary);

	if (!file.is_open())
	{
		return false;
	}

	file.seekg( 0, ios::end );

	m_Filesize = file.tellg();
	
	file.close();

	m_StartBuffer = 0;

	return true;
}

//--------------------------------------------------------------------------
// Save To File
//--------------------------------------------------------------------------
void
MFileInfo::SaveToFile(std::ofstream& file)
{
	m_Filename.SaveToFile( file );
	file.write((const char*)&m_Filesize, 4);
	file.write((const char*)&m_StartBuffer, 4);
	file.write((const char*)&m_CompressSize, 4);
}

//--------------------------------------------------------------------------
// Load From File
//--------------------------------------------------------------------------
void
MFileInfo::LoadFromFile(std::ifstream& file)
{
	m_Filename.LoadFromFile( file );
	file.read((char*)&m_Filesize, 4);
	file.read((char*)&m_StartBuffer, 4);
	file.read((char*)&m_CompressSize, 4);
}

//--------------------------------------------------------------------------
//
//							MZLib
//
//--------------------------------------------------------------------------
MZLib::MZLib()
{
}

MZLib::~MZLib()
{
	ReleaseFileInfo();
	ReleasePackBuffer();
	ReleaseFileBuffer();
}

//--------------------------------------------------------------------------
// Release FileInfo
//--------------------------------------------------------------------------
void				
MZLib::ReleaseFileInfo()
{
	FILEINFO_LIST::iterator iInfo = m_listFileInfo.begin();

	while (iInfo != m_listFileInfo.end())
	{
		MFileInfo* pInfo = *iInfo;

		delete pInfo;

		iInfo++;
	}

	m_listFileInfo.clear();
}

//--------------------------------------------------------------------------
// Add FileInfo
//--------------------------------------------------------------------------
void				
MZLib::AddFileInfo(MFileInfo* pInfo)
{
	m_listFileInfo.push_back( pInfo );
}

//--------------------------------------------------------------------------
// Add File
//--------------------------------------------------------------------------
// �ϴ� sizeüũ�� ������ 
//--------------------------------------------------------------------------
bool				
MZLib::AddFile(const char* filename)
{
	MFileInfo* pFileInfo = new MFileInfo;
	
	if (!pFileInfo->Set( filename ))
	{
		delete pFileInfo;

		return false;
	}

	AddFileInfo( pFileInfo );

	return true;
}

//--------------------------------------------------------------------------
// Compress
//--------------------------------------------------------------------------
bool				
MZLib::Compress(const char* filename)
{
	int numSize = m_listFileInfo.size();

	if (numSize==0)
	{
		return false;
	}

	std::ofstream packFile(filename, ios::binary | ios::trunc);

	//--------------------------------------------------------------
	// header ����
	//--------------------------------------------------------------
	packFile.write((const char*)m_ZLibHeader, MZLIB_HEADER_SIZE);

	// fileInfo�� ����� ��ġ
	long fpFileInfoFP = packFile.tellp();	
	packFile.write((const char*)&fpFileInfoFP, 4);


	//--------------------------------------------------------------
	// ��ü filesize ���
	//--------------------------------------------------------------
	FILEINFO_LIST::iterator iInfo = m_listFileInfo.begin();

	while (iInfo != m_listFileInfo.end())
	{
		MFileInfo* pInfo = *iInfo;

		//--------------------------------------------------------------
		// ����
		//--------------------------------------------------------------
		// compress(buffer, bufferSize, data, dataSize)
		//
		// [���� ��]
		// buffer�� data�� ����Ǿ� ����ȴ�.
		// bufferSize�� ����� buffer�� size�� �ȴ�.
		// 
		long filesize = pInfo->GetFilesize();
		unsigned long compLen = max((float)filesize*1.5f , filesize+4096);
		unsigned long beforeCompLen = compLen;
		
		InitPackBuffer( compLen );
		InitFileBuffer( filesize );
		ReadBufferFromFile( pInfo->GetFilename() );

		int err = compress((Bytef*)m_pPackBuffer, &compLen, (const Bytef*)m_pFileBuffer, filesize);

		if (beforeCompLen==compLen)
		{
			#ifdef _AFXDLL
				char str[256];
				sprintf(str, "[Error] Not Enough Buffer: %s", pInfo->GetFilename());
				MessageBox( NULL, str, "Error", MB_OK );
			#endif

			return false;
		}

		long startBuffer = packFile.tellp();
		WriteBufferToFile( packFile, compLen );

		pInfo->SetBufferInfo( startBuffer, compLen );

		iInfo++;
	}

	//--------------------------------------------------------------
	// FileInfo�� packFile���� ����
	//--------------------------------------------------------------
	long fpFileInfo = packFile.tellp();	
	WriteFileInfoToFile( packFile );

	// fileInfo�� ����� ��ġ�� �����Ѵ�.
	packFile.seekp( fpFileInfoFP );
	packFile.write((const char*)&fpFileInfo, 4);	

	packFile.close();

	return true;

}

//--------------------------------------------------------------------------
// Uncompress
//--------------------------------------------------------------------------
bool
MZLib::Uncompress(const char* filename)
{
	std::ifstream packFile(filename, ios::binary);

	if (!packFile.is_open())
	{
		return false;
	}

	//-------------------------------------------------------------
	// Header �б�
	//-------------------------------------------------------------
	char str[MZLIB_HEADER_SIZE+1];

	packFile.read((char*)str, MZLIB_HEADER_SIZE);
	str[MZLIB_HEADER_SIZE] = '\0';

	if (strcmp(str, m_ZLibHeader)!=0)
	{
		return false;
	}

	//-------------------------------------------------------------
	// FileInfo�б�
	//-------------------------------------------------------------
	long fpFileInfo;
	packFile.read((char*)&fpFileInfo, 4);
	long fpCurrent = packFile.tellg();

	packFile.seekg( fpFileInfo );
	if (!ReadFileInfoFromFile( packFile ))
	{
		return false;
	}

	packFile.seekg( fpCurrent );	// �ʿ� ����..

	//-------------------------------------------------------------
	// ������ fileinfo�� �̿��ؼ� ���� ����
	//-------------------------------------------------------------
	FILEINFO_LIST::iterator iInfo = m_listFileInfo.begin();

#ifdef __GAME_CLIENT__
	SetProgressBarCount( m_listFileInfo.size() );
#endif

	while (iInfo != m_listFileInfo.end())
	{
		MFileInfo* pInfo = *iInfo;

		const char* writeFilename = pInfo->GetFilename();
		long startBuffer	= pInfo->GetStartBuffer();
		long bufferLen		= pInfo->GetCompressSize();
		unsigned long uncompLen		= pInfo->GetFilesize();

		packFile.seekg( startBuffer );
		InitPackBuffer( bufferLen );
		InitFileBuffer( uncompLen );
		ReadBufferFromFile( packFile, bufferLen );

		uncompress((Bytef*)m_pFileBuffer, &uncompLen, (const Bytef*)m_pPackBuffer, bufferLen);

		std::ofstream file( writeFilename, ios::binary | ios::trunc );
		file.write((const char*)m_pFileBuffer, uncompLen);
		file.close();

		#ifdef __GAME_CLIENT__
			UpdateProgressBar();
		#endif

		iInfo++;
	}

	packFile.close();

	return true;
}


//--------------------------------------------------------------------------
// Release PackBuffer
//--------------------------------------------------------------------------
void			
MZLib::ReleasePackBuffer()
{
	if (m_pPackBuffer!=NULL)
	{
		delete [] m_pPackBuffer;
		m_pPackBuffer = NULL;
	}
}

//--------------------------------------------------------------------------
// Release FileBuffer
//--------------------------------------------------------------------------
void
MZLib::ReleaseFileBuffer()
{
	if (m_pFileBuffer!=NULL)
	{
		delete [] m_pFileBuffer;
		m_pFileBuffer = NULL;
	}
}

//--------------------------------------------------------------------------
// Init PackBuffer
//--------------------------------------------------------------------------
void
MZLib::InitPackBuffer(long bytes)
{
	ReleasePackBuffer();

	m_pPackBuffer = new char [bytes];
}

//--------------------------------------------------------------------------
// Init FileBuffer
//--------------------------------------------------------------------------
void
MZLib::InitFileBuffer(long bytes)
{
	ReleaseFileBuffer();

	m_pFileBuffer = new char [bytes];
}

//--------------------------------------------------------------------------
// Read Buffer From File
//--------------------------------------------------------------------------
bool
MZLib::ReadBufferFromFile(const char* filename)
{
	std::ifstream file(filename, ios::binary);

	if (!file.is_open())
	{
		return false;
	}

	file.seekg( 0, ios::end );
	long filesize = file.tellg();
	file.seekg( 0 );

	file.read((char*)m_pFileBuffer, filesize);

	long readLen = file.gcount();

//	assert(readLen==filesize);

	file.close();

	return true;
}

//--------------------------------------------------------------------------
// Write Buffer To File
//--------------------------------------------------------------------------
void
MZLib::WriteBufferToFile(std::ofstream& packFile, long bytes)
{
	packFile.write((const char*)m_pPackBuffer, bytes);
}

//--------------------------------------------------------------------------
// Read Buffer From File
//--------------------------------------------------------------------------
void
MZLib::ReadBufferFromFile(std::ifstream& packFile, long bytes)
{
	packFile.read((char*)m_pPackBuffer, bytes);
}

//--------------------------------------------------------------------------
// Write Buffer To File
//--------------------------------------------------------------------------
void
MZLib::WriteFileInfoToFile(std::ofstream& packFile)
{
	long numSize = m_listFileInfo.size();

	packFile.write((const char*)&numSize, 4);	

	FILEINFO_LIST::iterator iInfo = m_listFileInfo.begin();

	while (iInfo != m_listFileInfo.end())
	{
		MFileInfo* pInfo = *iInfo;

		pInfo->SaveToFile( packFile );

		iInfo++;
	}
}

//--------------------------------------------------------------------------
// Read Buffer To File
//--------------------------------------------------------------------------
bool				
MZLib::ReadFileInfoFromFile(std::ifstream& packFile)
{
	ReleaseFileInfo();	
	
	long numSize;
	
	packFile.read((char*)&numSize, 4);

	for (int i=0; i<numSize; i++)
	{
		MFileInfo* pInfo = new MFileInfo;

		pInfo->LoadFromFile( packFile );

		AddFileInfo( pInfo );
	}

	return true;
}