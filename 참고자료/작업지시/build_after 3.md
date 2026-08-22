

# 목표



ProfileManager.CPP 의 아래 부분에서 비정상 종료가 발생하고 있어.
원인이 뭘까? 수정좀 한번 해줄래?


while (_findnext( hFile, &FileData ) == 0);


//----------------------------------------------------------------------
// Init Profiles
//----------------------------------------------------------------------
// 프로그램이 실행될 때 한번 실행시켜주면 된다.
//
// Profile/*.bmp 를 읽어서 Profile/*.spr로 바꿔주면 된다.
//----------------------------------------------------------------------
void		
ProfileManager::InitProfiles()
{
	//-----------------------------------------------------------------
	// Profile Directory가 없으면 생성한다.
	//-----------------------------------------------------------------
	char CWD[_MAX_PATH];

	if (_getcwd( CWD, _MAX_PATH )!=NULL)
	{	
		if (_chdir( g_pFileDef->getProperty("DIR_PROFILE").c_str()) == 0)
		{
			// 있다면.. 다시 원래 DIR로..
			_chdir( CWD );
		}
		else
		{
			// DIR_PROFILE이 없다면.. 생성..
#ifdef PLATFORM_WINDOWS
			_mkdir( g_pFileDef->getProperty("DIR_PROFILE").c_str() );
#else
			mkdir( g_pFileDef->getProperty("DIR_PROFILE").c_str(), 0755 );
#endif
		}		
	}

	char profileFiles[256];
	sprintf(profileFiles, "%s\\*.*", g_pFileDef->getProperty("DIR_PROFILE").c_str());
	
	char bmpFilename[256];

	struct _finddata_t	FileData;
	long				hFile;

	//-----------------------------------------------------------------
	// *.spr file을 찾는다.
	//-----------------------------------------------------------------
	if ( (hFile = _findfirst( profileFiles, &FileData )) != -1L )
	{
		CSpritePack SPK;

		// [0]은 작은거 (30, 38)
		// [1]은 큰거 (110, 139)
		SPK.Init( 2);

		// A CDirectDrawSurface-based branch used to run here on Windows,
		// with a CSpriteSurface `surface` used to Blt/Lock the loaded BMP
		// into SPK[]. With SPRITELIB_BACKEND_SDL (the only backend this
		// project builds, Windows included) CSpriteSurface no longer
		// inherits from CDirectDrawSurface and that branch never
		// type-checked; it's now a stub (see the notes further below), so
		// that intermediate surface isn't needed here any more either.
		const POINT bigSize = { 55, 70 };
		const POINT smallSize = { 30, 38 };

		// Note: SDL backend doesn't have InitOffsurface, surface will be created when needed

		do
		{
			sprintf(bmpFilename, "%s\\%s", g_pFileDef->getProperty("DIR_PROFILE").c_str(), FileData.name);
	
			//---------------------------------------------------------
			// bmp를 읽어서 sprite로 바꾼다.
			//---------------------------------------------------------
			char charName[256], spkFilename[256], spkiFilename[256];
			int lenFilename = strlen(FileData.name);

			// "이름.bmp"
			if (lenFilename< 8)
			{
				continue;
			}

			strncpy( charName, FileData.name, lenFilename-4 );	// .bmp를 짜른다.
			charName[lenFilename-4] = '\0';

			// CDirectDrawSurface-based loading used to run here on Windows,
			// but with SPRITELIB_BACKEND_SDL CSpriteSurface no longer
			// inherits from CDirectDrawSurface (LoadImageToSurface() has no
			// overload for it), so that branch never type-checked - see the
			// stub notes further below. Profile image loading is not yet
			// implemented on the SDL backend (Windows included); this is a
			// non-critical feature (profile character portraits).
			// TODO: Implement SDL_image based loading
			WORD* lpSurface;
			unsigned short pitch;

			// Create temporary surfaces for the profile
			RECT bmpRect = { 0, 0, smallSize.x, smallSize.y };
			RECT bmpRectBig = { 0, 0, bigSize.x, bigSize.y };

			// For now, just initialize empty sprites
			// The profile will load but without character portrait image
			lpSurface = new WORD[smallSize.x * smallSize.y];
			memset(lpSurface, 0, smallSize.x * smallSize.y * 2);
			pitch = smallSize.x * 2;
			SPK[0].SetPixelNoColorkey(lpSurface, pitch, smallSize.x, smallSize.y);
			delete[] lpSurface;

			lpSurface = new WORD[bigSize.x * bigSize.y];
			memset(lpSurface, 0, bigSize.x * bigSize.y * 2);
			pitch = bigSize.x * 2;
			SPK[1].SetPixelNoColorkey(lpSurface, pitch, bigSize.x, bigSize.y);
			delete[] lpSurface;

			// filename.spk
			int lenBmpFilename = strlen(bmpFilename);
			strncpy(spkFilename, bmpFilename, lenBmpFilename-3);
			spkFilename[lenBmpFilename-3] = '\0';
			strcat(spkFilename, "spk");

			// filename.spki
			strcpy(spkiFilename, spkFilename);
			strcat(spkiFilename, "i");

			std::ofstream	spkFile(spkFilename, ios::binary);
			std::ofstream	spkiFile(spkiFilename, ios::binary);
			SPK.SaveToFile( spkFile, spkiFile );
			spkFile.close();
			spkiFile.close();

			g_pProfileManager->AddProfile( charName, spkFilename );
		}
		while (_findnext( hFile, &FileData ) == 0);

		_findclose( hFile );
	}
}




# 요구사항
3. 코드 스타일: 
   - Visual Studio 2019 환경 , C++ 11 표준 사용중
   - Allman 스타일(중괄호 다음 줄), 헝가리언 표기법 준수.
   - 한 줄 제어문도 줄바꿈/중괄호 필수.
   - 인코딩: UTF-8 with BOM.   
4. 기존 로직 보존: 기존 프로젝트의 다른 코드는 건드리지 마세요(재탐색 금지).
5. 응답은 항상 한글로 해주세요.
