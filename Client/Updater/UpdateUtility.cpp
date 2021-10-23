//----------------------------------------------------------------------
// UpdateUtility.cpp
//----------------------------------------------------------------------
#include "Client_PCH.h"
#include <io.h>
#include <direct.h>
#include "UpdateUtility.h"
//#include "SPKFileLib.h"
#include "CFileIndexTable.h"


#define	NULL	0

//----------------------------------------------------------------------
// Pack�� Size�� byte ��
//----------------------------------------------------------------------
typedef	unsigned short		TYPE_PACKSIZE;
#define	SIZE_PACKSIZE		2

// �ѹ��� ī���ϴ� byte��
#define	SIZE_BUFFER			4096


//----------------------------------------------------------------------
// Has Permission
//----------------------------------------------------------------------
// Directory�� File ���ٽÿ�.. �ٸ� Directory�� �ִ°� �ǵ��̸� �ȵȴ�.
//----------------------------------------------------------------------
bool
UUFHasPermission(const char* filename)
{	
	//------------------------------------------------------------
	// filename�� ���� ��� 
	//------------------------------------------------------------
	if (filename==NULL)
	{	
		return false;
	}

	//------------------------------------------------------------
	// ù ���ڰ� "\"�̸� �ȵȴ�. 
	// (�ֻ��� ���丮�� ����...�����Ѱ�? - -;; �׳� �� �غ�..)
	//------------------------------------------------------------
	if (filename[0]=='\\' || filename[0]=='/')
	{
		return false;
	}

	//------------------------------------------------------------
	// ":"�� ���� �ȵȴ�. (����̺긦 �ٲ� �� �ִ�.)
	//------------------------------------------------------------
	if (strchr( filename, ':' )!=NULL)
	{
		return false;
	}
	
	//------------------------------------------------------------
	// ".." �� ���� �ȵȴ�. (���� ���丮�̹Ƿ�)
	//------------------------------------------------------------
//	if (strstr( filename, ".." )!=NULL)
//	{
//		return false;
//	}
   

	return true;
}

//----------------------------------------------------------------------
// Create Directory
//----------------------------------------------------------------------
// dirName : ���� directory �ؿ� �����ҷ��� directory �̸�
//----------------------------------------------------------------------
// ���ο� Directory�� �����Ѵ�.
//----------------------------------------------------------------------
bool
UUFCreateDirectory(const char* dirName)
{
	// Directory ����
	if (UUFHasPermission( dirName ))
	{
		// ����� ������ ���
		if (_mkdir( dirName )==0)
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------
// Remove Directory
//----------------------------------------------------------------------
// dirName : ������� directory �̸�
//----------------------------------------------------------------------
// Directory�� �����.
//----------------------------------------------------------------------
bool
UUFDeleteDirectory(const char* dirName)
{
	if (UUFHasPermission( dirName ))
	{
		// ����� ���� ���
		if (_rmdir( dirName )==0)
		{
			return true;
		}
		// ���� �־ �� ������ ���..
		else //if (errno!=ENOENT)
		{
			char CWD[_MAX_PATH];

			// ���� directory�� ����صд�.				
			GetCurrentDirectory(_MAX_PATH, CWD);
			
			if (_chdir( dirName ) == 0)
			{
				//---------------------------------------------------
				// file�ϳ��ϳ��� �����ش�. T_T;
				//---------------------------------------------------
				struct _finddata_t	FileData;
				long				hFile;

				// ��� ȭ���� �о�´�.
				if( (hFile = _findfirst( "*.*", &FileData )) != -1L )					
				{
					do
					{
						// .���� �����ϴ� �� ���� �ʿ� ����..
						if (FileData.name[0] != '.')
						{
							remove( FileData.name );
						}
					}
					while (_findnext( hFile, &FileData ) == 0);

					// ��
					_findclose( hFile );			
				}
				else
				{
					return false;
				}
				
//				_chdir( "..\\" );
				SetCurrentDirectory(CWD);
				if (_rmdir( dirName )==0)
				{
					// �� ��������.
					return true;
				}
			}		
			else
			{
				return false;
			}
		}		
	}

	return false;
}

//----------------------------------------------------------------------
// Remove Files
//----------------------------------------------------------------------
// dirName : ������� directory �̸�
//----------------------------------------------------------------------
// Directory���� ���ϵ��� �����.
//----------------------------------------------------------------------
bool
UUFDeleteFiles(const char *path, const char *fileext)
{
	char cwd[512];
	_getcwd(cwd, 512);

	if (_chdir( path ) == 0)
	{
		//---------------------------------------------------
		// file�ϳ��ϳ��� �����ش�. T_T;
		//---------------------------------------------------
		struct _finddata_t	FileData;
		long				hFile;
		
		// ��� ȭ���� �о�´�.
		if( (hFile = _findfirst( fileext, &FileData )) != -1L )					
		{
			do
			{
				// .���� �����ϴ� �� ���� �ʿ� ����..
				if (FileData.name[0] != '.')
				{
					remove( FileData.name );
				}
			}
			while (_findnext( hFile, &FileData ) == 0);
			
			// ��
			_findclose( hFile );			
		}
		
	}		
	_chdir(cwd);

	return true;
}

//----------------------------------------------------------------------
// Copy File
//----------------------------------------------------------------------
// SourceFile : ����
// TargetFile : ���� filename
//----------------------------------------------------------------------
// SourceFile�� TargetFile�� copy�ؼ� ���ο� file�� �����Ѵ�.
//----------------------------------------------------------------------
bool
UUFCopyFile(const char* FilenameSource, const char* FilenameTarget)
{				
	if (UUFHasPermission( FilenameTarget ) && UUFHasPermission( FilenameSource))				
	{
		std::ifstream fileSource(FilenameSource, std::ios::binary);
		std::ofstream fileTarget(FilenameTarget, std::ios::binary);
		
		// �߰�
		char buffer[SIZE_BUFFER];
		int n;
		
		//---------------------------------------------------------------
		// addFile�� �о originalFile�� ���� ���δ�.
		//---------------------------------------------------------------
		while (1)
		{
			fileSource.read(buffer, SIZE_BUFFER);
			
			n = fileSource.gcount();

			if (n > 0)
			{		
				fileTarget.write(buffer, n);
			}
			else
			{
				break;
			}
		}

		fileSource.close();
		fileTarget.close();		

		return true;
	}

	return false;
}
					
//----------------------------------------------------------------------
// Move File
//----------------------------------------------------------------------
// SourceFile : ����
// TargetFile : ���� filename
//----------------------------------------------------------------------
// SourceFile�� TargetFile�� rename�ؼ� ���ο� file�� �����Ѵ�.
//----------------------------------------------------------------------
bool
UUFMoveFile(const char* FilenameSource, const char* FilenameTarget)
{				
	if (UUFHasPermission( FilenameTarget ) && UUFHasPermission( FilenameSource))				
	{
		std::ifstream file;

		file.open(FilenameTarget, std::ios::binary);
		
		if (file.is_open())
		{
			file.close();

			// ���� �ִ��� ������ �����.
			if (remove( FilenameTarget )!=0)
			{
				// �� ���� ���
				return false;
			}
		}		
	
		// ȭ���̸��� �ٲ۴�.				
		if (rename( FilenameSource, FilenameTarget )==0)
		{
			return true;
		}		
	}

	return false;
}

//----------------------------------------------------------------------
// Delete File
//----------------------------------------------------------------------
// TargetFile : ������ filename
//----------------------------------------------------------------------
// TargetFile�� �����.
//----------------------------------------------------------------------
bool
UUFDeleteFile(const char* FilenameTarget)
{
	// file�� �����.
	if (UUFHasPermission( FilenameTarget ))
	{
		if (remove( FilenameTarget )==0)
		{
			return true;
		}
	}

	return false;
}


//----------------------------------------------------------------------
// Rename File
//----------------------------------------------------------------------
// SourceFile : old filename
// TargetFile : new filename
//----------------------------------------------------------------------
// SourceFile�� TargetFile�� �̸��� �ٲ۴�.
// Directory name�� �ٲ� �� �ִ�.
//----------------------------------------------------------------------
bool
UUFRenameFile(const char* FilenameSource, const char* FilenameTarget)
{
	// ȭ���̸��� �ٲ۴�.
	if (UUFHasPermission( FilenameTarget ))
	{
		if (rename( FilenameSource, FilenameTarget )==0)
		{
			return true;
		}
	}

	return false;
}


//----------------------------------------------------------------------
// Append Pack
//----------------------------------------------------------------------
// SourceFile		: Pack(to add)
// TargetFile		: Pack(original)
// SourceIndexFile	: PackIndex(to add)
// TargetIndexFile	: PackIndex(original)
//----------------------------------------------------------------------
// TargetFile�� ���� SourceFile�� �߰��Ѵ�.
// ��, TargetFile�� ��ü ������ �߰��� �� ��ŭ �����ؾ� �Ѵ�.    
//----------------------------------------------------------------------
bool
UUFAppendPack(const char* FilenameAdd, const char* FilenameOriginal,
			const char* FilenameIndexAdd, const char* FilenameIndexOriginal)
{	
	//--------------------------------------------------
	// Index�� �߰��ϴ°�?
	//--------------------------------------------------
	bool bAppendIndex;

	if (FilenameIndexAdd==NULL && FilenameIndexOriginal==NULL)
	{
		bAppendIndex = false;
	}
	else
	{
		bAppendIndex = true;
	}

	//--------------------------------------------------
	// permission üũ
	//--------------------------------------------------
	if (UUFHasPermission( FilenameOriginal ) 
		&& UUFHasPermission( FilenameAdd)

		// index�� ���ų�.. �������� permission�־�� �Ѵ�.
		&& (!bAppendIndex ||
			UUFHasPermission( FilenameIndexOriginal ) 
			&& UUFHasPermission( FilenameIndexAdd )
		))		
	{
		//---------------------------------------------------------------
		// ������ Filesize�� ������?
		//---------------------------------------------------------------
		//if (!IsFileSizeBeforeOK())
		//{
			// Update�Ϸ��� �ϴµ� ���ϴ� Filesize�� �ƴϸ�.. �ɰ��ϴ�!!						
		//	return false;
		//}

		//---------------------------------------------------------------
		//
		//					Pack�� �߰��Ѵ�.
		//
		//---------------------------------------------------------------
		// �߰��� �� �ְ� �Ѵ�.
		std::ifstream addFile(FilenameAdd, std::ios::binary);
		std::fstream originalFile(FilenameOriginal, std::ios::in | std::ios::out | std::ios::binary);

		TYPE_PACKSIZE	sourceCount, targetCount;
		
		//---------------------------------------------------------------
		// source�� ������ ������ �д�.
		//---------------------------------------------------------------
		addFile.read((char*)&sourceCount, SIZE_PACKSIZE);

		//---------------------------------------------------------------
		// target File Pointer�� ������..
		//---------------------------------------------------------------
		originalFile.seekp(0, std::ios::end);

		//---------------------------------------------------------------
		// Original File�� ũ�⸦ ����صд�.
		//---------------------------------------------------------------
		long originalPackFileSize = originalFile.tellp();

		// �߰�
		char buffer[SIZE_BUFFER];
		int n;
		//---------------------------------------------------------------
		// addFile�� �о originalFile�� ���� ���δ�.
		//---------------------------------------------------------------
		while (1)
		{
			addFile.read(buffer, SIZE_BUFFER);
			
			n = addFile.gcount();

			if (n > 0)
			{		
				originalFile.write(buffer, n);
			}
			else
			{
				break;
			}
		}

		//---------------------------------------------------------------
		// ������ ��������ش�. (originalFile + addFile)
		//---------------------------------------------------------------
		originalFile.seekg(0, std::ios::beg);
		originalFile.read((char*)&targetCount, SIZE_PACKSIZE);

		targetCount += sourceCount;

		originalFile.seekp(0, std::ios::beg);				
		originalFile.write((const char*)&targetCount, SIZE_PACKSIZE);

		// ��
		addFile.close();
		originalFile.close();

		//---------------------------------------------------------------
		// ������ Filesize�� ������?
		//---------------------------------------------------------------
		//if (!IsFileSizeAfterOK())
		//{
			// Update�� �ߴµ� ���ϴ� Filesize�� �ƴϸ�.. �ɰ��ϴ�!!
			// �ɰ��� ����̴�.
		//	return false;
		//}

		
		//---------------------------------------------------------------
		//
		//					Pack Index�� �߰��Ѵ�.
		//
		//---------------------------------------------------------------	
		
		// Index�� �߰��ҷ��� ��쿡��..
		if (bAppendIndex)
		{
			//---------------------------------------------------------------
			// ������ Filesize�� ������?
			//---------------------------------------------------------------
			//if (!IsFileSizeBeforeOK())
			//{
				// Update�Ϸ��� �ϴµ� ���ϴ� Filesize�� �ƴϸ�.. �ɰ��ϴ�!!						
			//	return false;
			//}


			// �߰��� �� �ְ� �Ѵ�.
			std::ifstream addIndexFile(FilenameIndexAdd, std::ios::binary);
			std::fstream originalIndexFile(FilenameIndexOriginal, std::ios::in | std::ios::out | std::ios::binary);	

			TYPE_PACKSIZE	targetCount;

			//---------------------------------------------------------------
			// source index�� load�Ѵ�.
			//---------------------------------------------------------------
			CFileIndexTable	sourceIndexFile;
			sourceIndexFile.LoadFromFile( addIndexFile );

			//---------------------------------------------------------------
			// target File Pointer�� ������..
			//---------------------------------------------------------------
			originalIndexFile.seekp(0, std::ios::end);
			
			//---------------------------------------------------------------
			// OriginalFile�� ũ�⿡������ file pointer�� ���۵ǹǷ�
			//---------------------------------------------------------------
			long targetEnd = originalPackFileSize - SIZE_PACKSIZE;

			//---------------------------------------------------------------
			// Source�� �� file position�� 
			// targetEnd��ŭ �������Ѽ� originalIndexFile�� �߰��Ѵ�.
			//---------------------------------------------------------------
			long sourceIndex;
			for (int i=0; i<sourceIndexFile.GetSize(); i++)
			{
				sourceIndex = targetEnd + sourceIndexFile[i];
				originalIndexFile.write((const char*)&sourceIndex, 4);
			}								

			//---------------------------------------------------------------
			// ������ ��������ش�. (originalIndexFile + addIndexFile)
			//---------------------------------------------------------------
			originalIndexFile.seekg(0, std::ios::beg);
			originalIndexFile.read((char*)&targetCount, SIZE_PACKSIZE);

			targetCount += sourceIndexFile.GetSize();

			originalIndexFile.seekp(0, std::ios::beg);				
			originalIndexFile.write((const char*)&targetCount, SIZE_PACKSIZE);

			// ��
			addIndexFile.close();
			originalIndexFile.close();

			//---------------------------------------------------------------
			// ������ Filesize�� ������?
			//---------------------------------------------------------------
			//if (!IsFileSizeAfterOK())
			//{
				// Update�� �ߴµ� ���ϴ� Filesize�� �ƴϸ�.. �ɰ��ϴ�!!
				// �ɰ��� ����̴�.
			//	return false;
			//}	
		}

		return true;
	}

	return false;
}



//----------------------------------------------------------------------
// Append Info
//----------------------------------------------------------------------
// FilenameAdd		: �߰��� Information File
// FilenameOriginal : ������ Information File
//----------------------------------------------------------------------
// TargetFile�� ���� SourceFile�� �߰��Ѵ�.
// ��, TargetFile�� ��ü ������ �߰��� �� ��ŭ �����ؾ� �Ѵ�.    
//----------------------------------------------------------------------
bool
UUFAppendInfo(const char* FilenameAdd, const char* FilenameOriginal)
{
	if (UUFHasPermission( FilenameOriginal ) && UUFHasPermission( FilenameAdd))				
	{			
		//------------------------------------------
		// ������ Filesize�� ������?
		//------------------------------------------
		//if (!IsFileSizeBeforeOK())
		//{
			// Update�Ϸ��� �ϴµ� ���ϴ� Filesize�� �ƴϸ�.. �ɰ��ϴ�!!						
		//	return false;
		//}

		// �߰��� �� �ְ� �Ѵ�.
		std::ifstream sourceFile(FilenameAdd, std::ios::binary);
		std::fstream targetFile(FilenameOriginal, std::ios::in | std::ios::out | std::ios::binary);
		

		int sourceCount, targetCount;
		// source�� ������ ������ �д�.
		sourceFile.read((char*)&sourceCount, 4);

		// target File Pointer�� ������..
		targetFile.seekp(0, std::ios::end);

		// �߰�
		char buffer[SIZE_BUFFER];
		int n;
		// SourceFile�� �о TargetFile�� ���� ���δ�.
		while (1)
		{
			sourceFile.read(buffer, SIZE_BUFFER);
			
			n = sourceFile.gcount();

			if (n > 0)
			{		
				targetFile.write(buffer, n);
			}
			else
			{
				break;
			}
		}

		// ������ ��������ش�. (targetFile + sourceFile)
		targetFile.seekg(0, std::ios::beg);
		targetFile.read((char*)&targetCount, 4);

		targetCount += sourceCount;

		targetFile.seekp(0, std::ios::beg);				
		targetFile.write((const char*)&targetCount, 4);

		// ��
		sourceFile.close();
		targetFile.close();


		//------------------------------------------
		// ������ Filesize�� ������?
		//------------------------------------------
		//if (!IsFileSizeAfterOK())
		//{
			// Update�� �ߴµ� ���ϴ� Filesize�� �ƴϸ�.. �ɰ��ϴ�!!
			// �ɰ��� ����̴�.
		//	return false;
		//}
	}

	return true;
}		

//----------------------------------------------------------------------
// Update SpritePack
//----------------------------------------------------------------------
// SpritePack���� Ư���� sprite�鸸 ��ü�Ѵ�.
//----------------------------------------------------------------------
/*
bool		
UUFUpdateSpritePack(const char* FilenameNew, 
					const char* FilenameID, 
					const char* FilenameOriginal)
{
	if (UUFHasPermission( FilenameNew ) 
		&& UUFHasPermission( FilenameID )
		&& UUFHasPermission( FilenameOriginal ) )
	{
		int i;
		//-----------------------------------------------------
		// FilenameNew�� Ȯ���ڸ� ã�´�.
		//-----------------------------------------------------
		int len = strlen(FilenameNew);
		char fileExtNew[10];
		fileExtNew[0] = '\0';
		for (i=len-1; i>=0; i--)
		{
			if (FilenameNew[i]=='.')
			{
				// �ҹ��ڷ� ����
				char temp[80];
				strcpy(temp, FilenameNew+i+1);
				strcpy(fileExtNew, _strlwr( temp ));
				break;
			}
		}

		//-----------------------------------------------------
		// FilenameOriginal�� Ȯ���ڸ� ã�´�.
		//-----------------------------------------------------
		len = strlen(FilenameOriginal);
		char fileExtOriginal[10];
		char filenameOriginal[80];
		fileExtOriginal[0] = '\0';
		for (i=len-1; i>=0; i--)
		{
			if (FilenameOriginal[i]=='.')
			{
				// �ҹ��ڷ� ����
				char temp[80];
				strcpy(temp, FilenameOriginal+i+1);
				strcpy(fileExtOriginal, _strlwr( temp ));
				
				// filename�� ���Ѵ�.
				for (int j=0; j<i; j++)
				{
					filenameOriginal[j] = FilenameOriginal[j];
				}
				filenameOriginal[i] = NULL;

				break;
			}
		}

		//-----------------------------------------------------
		// �� ȭ���� Ȯ���ڰ� �ٸ� ���� 
		// Update�Ұ�!
		//-----------------------------------------------------
		if (strcmp(fileExtNew, fileExtOriginal)!=0)
		{
			return false;
		}

		//-----------------------------------------------------
		//
		// id-set
		//
		//-----------------------------------------------------
		CSpriteIDTable	SIDT;
		class ifstream idFile(FilenameID, ios::binary);
		SIDT.LoadFromFile( idFile );
		idFile.close();

		//-----------------------------------------------------
		//
		//   SPK File Load
		//
		//-----------------------------------------------------
		CSpritePackBase* pSPKNew;
		CSpritePackBase* pSPKOriginal;
		
		//-----------------------------------------------------
		// SpritePack
		//-----------------------------------------------------
		if (strcmp(fileExtNew, "spk")==0)
		{
			pSPKNew = new CSpritePack;
			pSPKOriginal = new CSpritePack;
		}
		//-----------------------------------------------------
		// AlphaSpritePack
		//-----------------------------------------------------
		else if (strcmp(fileExtNew, "aspk")==0)
		{
			pSPKNew = new CAlphaSpritePack;
			pSPKOriginal = new CAlphaSpritePack;
		}
		//-----------------------------------------------------
		// IndexSpritePack
		//-----------------------------------------------------
		else if (strcmp(fileExtNew, "ispk")==0)
		{
			pSPKNew = new CIndexSpritePack;
			pSPKOriginal = new CIndexSpritePack;
		}
		//-----------------------------------------------------
		// ShadowSpritePack
		//-----------------------------------------------------
		else if (strcmp(fileExtNew, "sspk")==0)
		{
			pSPKNew = new CShadowSpritePack;
			pSPKOriginal = new CShadowSpritePack;
		}
		else
		{
			return false;
		}
		
		//-----------------------------------------------------
		//		
		// New SPK File
		//
		//-----------------------------------------------------
		class ifstream newSPKFile(FilenameNew, ios::binary);		
		pSPKNew->LoadFromFile( newSPKFile );
		newSPKFile.close();

		//-----------------------------------------------------
		//
		// Original SPK File
		//
		//-----------------------------------------------------		
		class ifstream originalSPKFile(FilenameOriginal, ios::binary);				
		pSPKOriginal->LoadFromFile( originalSPKFile );
		originalSPKFile.close();


		//-----------------------------------------------------
		//
		//   �κ������� ��ü�Ѵ�.
		//
		//-----------------------------------------------------
		for (i=0; i<SIDT.GetSize(); i++)
		{
			//-----------------------------------------------------
			// new���� i��°�� 
			// original���� SIDT[i]��° sprite�� �ȴ�.
			//-----------------------------------------------------
			*((*pSPKOriginal)[ SIDT[i] ]) = *((*pSPKNew)[i]);
		}

		//-----------------------------------------------------
		// Save : �ٲ� ���� �ٽ� �����Ѵ�.
		//-----------------------------------------------------
		char indexFilename[80];
		sprintf(indexFilename, "%s.i%s", filenameOriginal, fileExtNew);

		class ofstream originalSPKFile2(FilenameOriginal, ios::binary);
		class ofstream originalSPKIndexFile2(indexFilename, ios::binary);
	
		pSPKOriginal->SaveToFile( originalSPKFile2, originalSPKIndexFile2 );

		originalSPKFile2.close();
		originalSPKIndexFile2.close();

		//-----------------------------------------------------
		// �޸𸮿��� ����
		//-----------------------------------------------------
		delete pSPKNew;
		delete pSPKOriginal;		
	}

	return true;
}

//----------------------------------------------------------------------
// Delete SpritePack
//----------------------------------------------------------------------
// SpritePack���� Ư���� Sprite���� �����Ѵ�.
//----------------------------------------------------------------------
bool		
UUFDeleteSpritePack(const char* FilenameID, const char* FilenameOriginal)
{
	if (UUFHasPermission( FilenameID )
		&& UUFHasPermission( FilenameOriginal ) )
	{
		//-----------------------------------------------------
		// FilenameOriginal�� Ȯ���ڸ� ã�´�.
		//-----------------------------------------------------
		int len = strlen(FilenameOriginal);
		char fileExtOriginal[10];
		char filenameOriginal[80];
		fileExtOriginal[0] = '\0';
		for (int i=len-1; i>=0; i--)
		{
			if (FilenameOriginal[i]=='.')
			{
				// �ҹ��ڷ� ����
				char temp[80];
				strcpy(temp, FilenameOriginal+i+1);
				strcpy(fileExtOriginal, _strlwr( temp ));
				
				// filename�� ���Ѵ�.
				for (int j=0; j<i; j++)
				{
					filenameOriginal[j] = FilenameOriginal[j];
				}
				filenameOriginal[i] = NULL;

				break;
			}
		}

		//-----------------------------------------------------
		//
		//   SPK File Load
		//
		//-----------------------------------------------------
		CSpritePackBase* pSPKOriginal;
		
		//-----------------------------------------------------
		// SpritePack
		//-----------------------------------------------------
		if (strcmp(fileExtOriginal, "spk")==0)
		{
			pSPKOriginal = new CSpritePack;
		}
		//-----------------------------------------------------
		// AlphaSpritePack
		//-----------------------------------------------------
		else if (strcmp(fileExtOriginal, "aspk")==0)
		{
			pSPKOriginal = new CAlphaSpritePack;
		}
		//-----------------------------------------------------
		// IndexSpritePack
		//-----------------------------------------------------
		else if (strcmp(fileExtOriginal, "ispk")==0)
		{
			pSPKOriginal = new CIndexSpritePack;
		}
		//-----------------------------------------------------
		// ShadowSpritePack
		//-----------------------------------------------------
		else if (strcmp(fileExtOriginal, "sspk")==0)
		{
			pSPKOriginal = new CShadowSpritePack;
		}
		else
		{
			return false;
		}
		

		//-----------------------------------------------------
		//
		// Original SPK File
		//
		//-----------------------------------------------------		
		class ifstream originalSPKFile(FilenameOriginal, ios::binary);				
		pSPKOriginal->LoadFromFile( originalSPKFile );
		originalSPKFile.close();

		
		//-----------------------------------------------------
		//
		// id-set
		//
		//-----------------------------------------------------
		CSpriteIDTable	SIDT;
		class ifstream idFile(FilenameID, ios::binary);
		SIDT.LoadFromFile( idFile );
		idFile.close();

		//-----------------------------------------------------
		//
		//   �����Ѵ�. == �ٽ� Save�Ҷ� ����..
		//
		//-----------------------------------------------------
		//-----------------------------------------------------
		// Save : �ٲ� ���� �ٽ� �����Ѵ�.
		//-----------------------------------------------------
		char indexFilename[80];
		sprintf(indexFilename, "%s.i%s", filenameOriginal, fileExtOriginal);

		class ofstream spkFile(FilenameOriginal, ios::binary);
		class ofstream indexFile(indexFilename, ios::binary);
	
		//--------------------------------------------------
		// index file�� �����ϱ� ���� ����
		//--------------------------------------------------
		long*	pIndex = new long [pSPKOriginal->GetSize()];

		//--------------------------------------------------
		// Size ����
		//--------------------------------------------------
		TYPE_SPRITEID size = pSPKOriginal->GetSize() - SIDT.GetSize();

		spkFile.write((const char *)&size, SIZE_SPRITEID); 
		indexFile.write((const char *)&size, SIZE_SPRITEID); 

		//--------------------------------------------------
		// SpritePack���� SIDT�� ���� �͸� �����Ѵ�.
		//--------------------------------------------------
		int k=0;
		for (i=0; i<pSPKOriginal->GetSize(); i++)
		{
			//--------------------------------------------------
			// �ִ��� ������ ����..
			//--------------------------------------------------
			bool bDelete = false;
			// ��.. �̷� ������ �˰����� ���ٴ�..
			// ��. ������.. - -;;
			for (int j=0; j<SIDT.GetSize(); j++)
			{
				if (i == SIDT[j])
				{
					bDelete = true;
					break;
				}
			}

			//--------------------------------------------------
			// ������ �ϴ� ���̸� skip�Ѵ�.
			//--------------------------------------------------
			if (bDelete)
			{
				continue;
			}

			//--------------------------------------------------
			// SpritePack file�� �������� index�� ����
			//--------------------------------------------------
			pIndex[k++] = spkFile.tellp();

			//--------------------------------------------------
			// Sprite����
			//--------------------------------------------------
			((*pSPKOriginal)[i])->SaveToFile(spkFile);		// CSprite����				
		}

		if (k!=size)
		{
			// �ɰ��� ����. - -;;
			return false;
		}

		//--------------------------------------------------
		// index ����
		//--------------------------------------------------
		for (i=0; i<size; i++)
		{
			indexFile.write((const char*)&pIndex[i], 4);
		}

		delete [] pIndex;


		spkFile.close();
		indexFile.close();

		//-----------------------------------------------------
		// �޸𸮿��� ����
		//-----------------------------------------------------
		delete pSPKOriginal;		
	}

	return true;
}

//----------------------------------------------------------------------
// Make SpritePack Index
//----------------------------------------------------------------------
// SpritePack�� Index�� �����Ѵ�.
//----------------------------------------------------------------------
bool		
UUFMakeSpritePackIndex(const char* Filename)
{
	if (UUFHasPermission( Filename ) )
	{
		int i;
		//-----------------------------------------------------
		// Filename�� Ȯ���ڸ� ã�´�.
		//-----------------------------------------------------
		int len = strlen(Filename);
		char filename[80];
		char fileExt[10];
		fileExt[0] = '\0';
		for (i=len-1; i>=0; i--)
		{
			if (Filename[i]=='.')
			{
				// �ҹ��ڷ� ����
				char temp[80];
				strcpy(temp, Filename+i+1);
				strcpy(fileExt, _strlwr( temp ));

				// filename�� ���Ѵ�.
				for (int j=0; j<i; j++)
				{
					filename[j] = Filename[j];
				}
				filename[i] = NULL;

				break;
			}
		}

		//-----------------------------------------------------
		//
		//   SPK File Load
		//
		//-----------------------------------------------------
		CSpritePackBase* pSPK;
		
		//-----------------------------------------------------
		// SpritePack
		//-----------------------------------------------------
		if (strcmp(fileExt, "spk")==0)
		{
			pSPK = new CSpritePack;			
		}
		//-----------------------------------------------------
		// AlphaSpritePack
		//-----------------------------------------------------
		else if (strcmp(fileExt, "aspk")==0)
		{
			pSPK = new CAlphaSpritePack;			
		}
		//-----------------------------------------------------
		// IndexSpritePack
		//-----------------------------------------------------
		else if (strcmp(fileExt, "ispk")==0)
		{
			pSPK = new CIndexSpritePack;			
		}
		//-----------------------------------------------------
		// ShadowSpritePack
		//-----------------------------------------------------
		else if (strcmp(fileExt, "sspk")==0)
		{
			pSPK = new CShadowSpritePack;			
		}
		else
		{
			return false;
		}
		
		//-----------------------------------------------------
		//		
		//  SPK File
		//
		//-----------------------------------------------------
		class ifstream SPKFile(Filename, ios::binary);		
		pSPK->LoadFromFile( SPKFile );
		SPKFile.close();

		//-----------------------------------------------------
		// Save : �ٽ� �����Ѵ�. --> Index����
		//-----------------------------------------------------
		char indexFilename[80];
		sprintf(indexFilename, "%s.%si", filename, fileExt);

		class ofstream SPKFile2(Filename, ios::binary);
		class ofstream SPKIndexFile2(indexFilename, ios::binary);
	
		pSPK->SaveToFile( SPKFile2, SPKIndexFile2 );

		SPKFile2.close();
		SPKIndexFile2.close();

		//-----------------------------------------------------
		// �޸𸮿��� ����
		//-----------------------------------------------------
		delete pSPK;
	}

	return true;
}
*/