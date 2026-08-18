

# 목표

빌드는 정상적으로 된다. 앞으로는 아래 요구사항을 따라주고 
맨 아래 SpriteLib 빌드 오류좀 수정해줘 



# 수정 대상



# 요구사항
3. 코드 스타일: 
   - Visual Studio 2019 환경 , C++ 11 표준 사용중
   - Allman 스타일(중괄호 다음 줄), 헝가리언 표기법 준수.
   - 한 줄 제어문도 줄바꿈/중괄호 필수.
   - 인코딩: UTF-8 with BOM.   
4. 기존 로직 보존: 기존 프로젝트의 다른 코드는 건드리지 마세요(재탐색 금지).
5. 응답은 항상 한글로 해주세요.
6. commit 은 하지 않고 소스 경로내에 참고자료/커밋로그 내에 기록해주세요.

# 출력 형식
1. 수정된 코드 블록
2. 적용 후 커밋 로그 (실제 커밋은 하지 마세요)
3. 부모 다이얼로그에서 데이터를 전달하는 호출부 예시 코드


# 오류 로그 



빌드 시작...
1>------ 빌드 시작: 프로젝트: SpriteLib, 구성: Debug x64 ------
1>SpriteLibBackendSDL.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\SpriteLibBackendSDL.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\SpriteLibBackendSDL.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\SpriteLibBackendSDL.cpp(907,9): warning C4996: 'fopen': This function or variable may be unsafe. Consider using fopen_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\SpriteLibBackendSDL.cpp(927,23): warning C4996: 'strdup': The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: _strdup. See online help for details.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\SpriteLibBackendSDL.cpp(1152,8): warning C4101: 'file' :참조되지 않은 지역 변수입니다.
1>CSpriteSurface_SDL.cpp
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface_SDL.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CSpriteSurface_Adapter.cpp
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface_Adapter.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CSpriteSurface_Effects.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface_Effects.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface_Effects.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CSprite_SDL.cpp
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite_SDL.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CSprite.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CFilter.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CIndexSprite.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CIndexSprite.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CIndexSprite.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CAlphaSprite.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CSprite555.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite555.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite555.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CSprite565.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite565.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite565.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CAlphaSprite555.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite555.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite555.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CAlphaSprite565.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite565.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite.h(19,14): warning C4091: '': 변수를 선언하지 않으면 'std::basic_ofstream<char,std::char_traits<char>>' 왼쪽은 무시됩니다.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite.h(20,14): warning C4091: '': 변수를 선언하지 않으면 'std::basic_ifstream<char,std::char_traits<char>>' 왼쪽은 무시됩니다.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite565.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite565.h(16,14): warning C4091: '': 변수를 선언하지 않으면 'std::basic_ofstream<char,std::char_traits<char>>' 왼쪽은 무시됩니다.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite565.h(17,14): warning C4091: '': 변수를 선언하지 않으면 'std::basic_ifstream<char,std::char_traits<char>>' 왼쪽은 무시됩니다.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSprite565.cpp(32,15): warning C4101: 'i' :참조되지 않은 지역 변수입니다.
1>CAlphaSpritePal.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSpritePal.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSpritePal.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CIndexSprite555.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CIndexSprite555.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CIndexSprite555.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CIndexSprite565.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CIndexSprite565.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CIndexSprite565.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CSpritePalBase.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpritePalBase.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpritePalBase.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CSpritePack.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpritePack.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>CSpritePackList.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpritePackList.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpritePackList.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CAlphaSpritePack.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSpritePack.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CAlphaSpritePack.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CShadowSprite.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CShadowSprite.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CShadowSprite.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>코드를 생성하고 있습니다...
1>컴파일하고 있습니다...
1>CFileIndexTable.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CFileIndexTable.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CFileIndexTable.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>CFilter.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CFilter.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CFilter.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CFilterPack.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CFilterPack.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CFilter.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CStorageSurface.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CStorageSurface.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CStorageSurface.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>DebugLog.cpp
1>H:\Source\GithubDesktop\dk_client\Client\DebugLog.cpp(17,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'sys/time.h': No such file or directory
1>CSpriteDef.cpp
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\TArray.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteDef.h(14,14): warning C4091: '': 변수를 선언하지 않으면 'std::basic_ifstream<char,std::char_traits<char>>' 왼쪽은 무시됩니다.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteDef.h(15,14): warning C4091: '': 변수를 선언하지 않으면 'std::basic_ofstream<char,std::char_traits<char>>' 왼쪽은 무시됩니다.
1>CSpriteOutlineManager.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSprite.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteOutlineManager.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CSpriteSet.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSet.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSurface.h(50,1): warning C4005: 'SPRITESURFACE_STANDALONE': 매크로 재정의
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpriteSet.cpp : message : 'SPRITESURFACE_STANDALONE'의 이전 정의를 참조하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>CSpritePal.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CSpritePal.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\CTypePack.h(6,11): fatal error C1083: 포함 파일을 열 수 없습니다. 'fstream.h': No such file or directory
1>MPalette.cpp
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\MPalette.cpp(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\../basic/Platform.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet/Exception.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/PlayerTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/CreatureTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types\ModifyDef.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ItemTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ZoneTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/QuestTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/ShopTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\Packet\types/GuildTypes.h(1,1): warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.
1>H:\Source\GithubDesktop\dk_client\Client\SpriteLib\MPalette.cpp(3,10): warning C4067: 전처리기 지시문 다음에 예기치 않은 토큰이 있습니다. 줄 바꿈 문자가 필요합니다.
1>코드를 생성하고 있습니다...
1>"SpriteLib.vcxproj" 프로젝트를 빌드했습니다. - 실패
========== 빌드: 성공 0, 실패 1, 최신 1, 생략 0 ==========

