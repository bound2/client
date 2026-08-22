# OpenDarkEden Client

이것은 **Dark Eden** 게임 클라이언트입니다 - Diablo와 유사한 아이소메트릭 MMORPG입니다. Dark Eden은 뱀파이어, 슬레이어, 오우스터를 플레이 가능한 종족으로 하는 호러 테마의 MMORPG입니다.

## 프로젝트 개요

Dark Eden은 Softon에서 원래 개발한 고전 한국 MMORPG입니다. 이 오픈소스 클라이언트 프로젝트는 게임 클라이언트를 현대화하고 유지보수하는 것을 목표로 합니다.

**빌드 시스템:** CMake
**주 언어:** C++
**플랫폼:** Windows(원본), 현대적인 크로스플랫폼 지원 노력 중

## 빌드 방법

`make`, `make debug`, `make release` 등의 명령어를 제공하는 cmake 래퍼 Makefile이 있습니다.

개발 시 가장 흔히 사용하는 명령어는 다음과 같습니다:

```
make debug-asan 
```

## 저장소 구조

```
client/
├── Client/              # 메인 게임 클라이언트 코드
│   ├── D3DLib/         # SDL 호환 스텁 (마이그레이션 이후)
│   ├── DXLib/          # Input, Sound, Music용 SDL 백엔드
│   ├── SpriteLib/      # 스프라이트 애니메이션 시스템 (SDL 백엔드)
│   ├── TextSystem/     # 텍스트 렌더링 시스템 (SDL + freetype2)
│   ├── TextLib/        # 텍스트 레이아웃 및 캐싱
│   ├── VolumeLib/      # 볼륨/충돌 감지
│   ├── framelib/       # 프레임 처리
│   ├── DEUtil/         # Dark Eden 유틸리티
│   ├── MZLib/          # 압축 라이브러리
│   ├── Packet/         # 네트워크 패킷 정의
│   └── *.cpp/*.h       # 메인 게임 로직 (GameMain, GameUI 등)
│
├── VS_UI/              # 사용자 인터페이스 프레임워크
│   └── src/
│       ├── header/     # UI 헤더 파일
│       ├── widget/     # UI 위젯 컴포넌트 (버튼, 스크롤바 등)
│       ├── Imm/        # Immersion 촉각 피드백 라이브러리
│       └── hangul/     # 한글 입력 지원
│
├── basic/              # 기본 유틸리티 라이브러리
│   ├── BasicMemory.h   # 메모리 관리
│   ├── BasicException.h # 예외 처리
│   ├── Typedef.h       # 타입 정의
│   └── PlatformUtil.h  # 플랫폼 유틸리티 (SDL/Windows 추상화)
│
├── build/              # CMake 빌드 출력
├── ../DarkEden/        # 게임 데이터 디렉토리 (런타임)
│   ├── Data/           # 게임 데이터 파일
│   │   ├── Info/       # 설정 파일
│   │   ├── Map/        # 맵 파일
│   │   └── ...
│   └── UserSet/        # 사용자 설정
│
└── demo/               # 데모 애플리케이션
```

## 주요 컴포넌트

### 렌더링 시스템 (SDL2 기반)
- **D3DLib/**: DirectX에서 SDL로의 마이그레이션을 위한 최소한의 호환 스텁
  - `CDirect3D`: D3D 호환 인터페이스를 제공하는 스텁 클래스
  - `DX3D.h`: 하위 호환성을 위한 메인 헤더
  - **참고**: 실제 렌더링은 SDL 백엔드를 사용하는 SpriteLib에서 처리됨

- **SpriteLib/**: SDL 백엔드를 사용하는 스프라이트 애니메이션 시스템
  - `CSprite_SDL.cpp`: SDL 스프라이트 구현
  - `CSpriteSurface_SDL.cpp`: SDL 서피스 구현
  - `SpriteLibBackendSDL.h/cpp`: SDL 백엔드 유틸리티
  - 여러 픽셀 포맷 지원 (555, 565, 4444 등)

- **TextSystem/**: 현대적인 텍스트 렌더링 (SDL + freetype2)
  - 국제화를 위한 UTF-8 지원
  - 기존 Windows GDI 렌더링을 대체
  - `TextService.cpp`: 메인 텍스트 서비스
  - `TextBackendSDL.cpp`: SDL 텍스트 백엔드

### 입력 및 사운드 (`Client/DXLib/`)
- **CDirectInput**: SDL 기반 키보드/마우스 입력 처리
- **CDirectSound/SoundStream**: SDL_mixer 기반 사운드 재생
- **CDirectMusic**: SDL_mixer 기반 음악 재생 (MP3, OGG 지원)
- **백엔드**: DirectX 호환 인터페이스를 가진 SDL2 구현
- **허프만 압축**: 네트워크 데이터 압축 (플랫폼 독립적)

### 스프라이트 시스템 (`Client/SpriteLib/`)
- SDL 백엔드를 사용하는 스프라이트 애니메이션 및 렌더링
- 텍스처 파트 관리
- 팔레트 조작
- 여러 픽셀 포맷 지원 (555, 565, 4444 등)
- **마이그레이션 상태**: DirectX 구현 제거됨, SDL 백엔드 활성화됨

### UI 프레임워크 (`VS_UI/`)
- **위젯 시스템**: 버튼, 스크롤바, 다이얼로그
- **스킨 매니저**: UI 테마
- **입력 에디터**: 한글 IME를 지원하는 텍스트 입력
- **게임 UI**: 종족별 UI (슬레이어, 뱀파이어, 오우스터)
- **다이얼로그**: 상점, 창고, 교환, 스킬트리 등

### 게임 로직 (`Client/`)
- **GameMain**: 메인 게임 루프
- **GameUI**: 게임 UI 매니저
- **MZone**: 존/맵 관리
- **MCreature**: 크리처/NPC 시스템
- **MPlayer**: 플레이어 캐릭터 관리
- **MItem**: 아이템 시스템
- **MSkill**: 스킬 시스템
- **Effect System**: 시각 효과 생성기

### 네트워크 (`Client/Packet/`)
- 클라이언트-서버 통신을 위한 패킷 정의
- 로그인, 게임플레이, 채팅 패킷

## 빌드 요구사항

### 최신 CMake 빌드 (권장):
이 프로젝트는 SDL2 백엔드를 사용하는 크로스플랫폼 빌드를 위해 CMake를 사용합니다.

**의존성:**
1. **CMake 3.20+** - 빌드 시스템
2. **SDL2** - 그래픽, 입력, 플랫폼 추상화
3. **SDL2_image** - 이미지 로딩 지원
4. **SDL2_ttf** - TrueType 폰트 렌더링 (freetype2)
5. **SDL2_mixer** (선택사항) - 오디오 재생
6. **C++11 호환 컴파일러** - Clang, GCC, MSVC

**플랫폼 지원:**
- ✅ macOS (테스트됨)
- ✅ Linux (동작해야 함)
- ⚠️ Windows (레거시 VC6 빌드 또는 최신 MSVC 사용)

### 원본 VC6 빌드 (레거시):
1. **Visual C++ 6.0** (원본 개발 환경)
2. **DirectX 9 SDK** - [미러](https://github.com/opendarkeden/client/raw/data/dx90bsdk.zip)에서 받기
3. **xerces-c 3.2.3** - XML 파싱 라이브러리 [미러](https://github.com/opendarkeden/client/raw/data/xerces-c-3.2.3.zip)

**참고:** VC6 빌드는 더 이상 사용되지 않습니다. SDL2를 사용하는 최신 CMake 빌드를 사용하세요.

## 게임 실행

1. 게임 데이터를 `DarkEden/` 디렉토리에 압축 해제
2. `DarkEden/Data/Info/GameClient.inf`에서 서버 IP 설정
3. 모드 인자와 함께 실행:
   - `0000000001` - 창 모드
   - `0000000002` - 전체화면
   - `0000000003` - 창 모드 1024x768
   - `0000000004` - 전체화면 1024x768

## 게임 특징

- **세 종족**: 슬레이어(인간), 뱀파이어, 오우스터
- **아이소메트릭 뷰**: 2D 스프라이트 기반 아이소메트릭 그래픽
- **스킬 시스템**: 종족별 스킬트리
- **아이템 시스템**: 장비, 소모품, 퀘스트 아이템
- **파티/길드**: 소셜 기능
- **PvP**: 플레이어 간 전투
- **던전**: 인스턴스 기반 던전 (Bathory 등)

## 설정 파일

- `GameClient.inf` - 서버 연결 설정
- `Language.inf` - 언어 설정
- `config.txt` - 일반 설정

## SDL 마이그레이션 상태

이 프로젝트는 크로스플랫폼 지원을 위해 Windows + DirectX에서 SDL2로 성공적으로 마이그레이션되었습니다.

### 완료된 마이그레이션
- ✅ **그래픽**: Direct3D → SDL2 렌더링
- ✅ **입력**: DirectInput → SDL2 이벤트 처리
- ✅ **사운드**: DirectSound → SDL_mixer
- ✅ **음악**: DirectMusic → SDL_mixer
- ✅ **텍스트**: Windows GDI → SDL2 + freetype2 (TextSystem)
- ✅ **플랫폼 추상화**: Win32 API → SDL2 + basic 라이브러리

### 아키텍처
- **DXLib**: SDL 백엔드를 가진 DirectX 호환 인터페이스 제공
- **D3DLib**: 최소한의 호환 스텁 (CDirect3D 클래스)
- **SpriteLib**: SDL 기반 스프라이트 렌더링
- **TextSystem**: 현대적인 UTF-8 텍스트 렌더링
- **basic/**: 크로스플랫폼 플랫폼 유틸리티

### 알려진 제약사항
- 일부 DirectX 전용 기능은 스텁 처리됨 (예: CDirect3D::GetDevice()는 nullptr 반환)
- 코드에 `CDirect3D::IsHAL()` 체크가 많이 남아있음 (SDL에서는 항상 true)
- 호환성을 위해 여러 픽셀 포맷 변형(555, 565)이 남아있음
- 참고용으로 일부 주석 처리된 D3D 코드가 남아있음

### 향후 작업
- [ ] 오래된 DirectX 코드 제거/주석 처리
- [ ] 픽셀 포맷 처리 단순화
- [ ] 코드베이스 전반의 `CDirect3D` 의존성 축소
- [ ] 픽셀 포맷 변형을 통합할 수 있다면 스프라이트 클래스 통합

## 개발 노트

- 코드베이스는 헝가리안 표기법을 사용합니다
- 현재 코드베이스에는 영어 | 한국어 | 중국어 주석이 혼재되어 있으며, **영어**만 사용해야 하므로 가능할 때마다 업데이트할 것
- 원본 코드는 약 2000~2010년대에 작성됨
- 일부 레거시 복사 방지 코드 존재 (EXECryptor, ACProtect) - 작동하지 않음
- GameGuard 안티치트 통합

## 관련 프로젝트

- **서버**: https://github.com/opendarkeden/server
- **Docker 설치 가이드**: 배포 방법은 서버 저장소 참고
