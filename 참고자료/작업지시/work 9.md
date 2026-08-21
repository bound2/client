

# 목표

`work 8.md`(어제 오후 마감 시점, 오류 616건)를 이어서 진행. 오늘(2026-08-21
저녁) 세션에서 **컴파일 오류를 전부 해소**하고 **링크 단계까지 진입**함.
큰 그림:

1. `__WIN32__`/`__WINDOWS__` 컴파일 정의 누락 발견 및 수정 (오류
   대부분의 공통 원인 - 아래 상세)
2. 그 위에서 드러난 개별 파일 오류들을 트리비얼한 것부터 순서대로 정리
3. **컴파일 오류 0건 도달**
4. 링크 오류로 넘어가서 다시 몇 단계 정리 (`iconv.lib`, 중복 컴파일,
   `atls.lib`, 표준 Windows 라이브러리 누락, `platform_get_ticks` 등
   플랫폼 추상화 함수 Windows 미구현)
5. 남은 링크 오류(약 50건, 71줄) - 전부 "진짜 기능이 없어서" 나는
   것들이라 상의 필요 (아래 "수정 대상" 참고)

같이 참고할 것:
1) vs6 에서 빌드 하려고 했던 본래 소스
"H:\Source\GithubDesktop\client-master_vs6"
2) 오늘 저녁 작업 상세 커밋 기록은 `git log`로 확인(이 문서 작성 시점
   기준 최신 커밋들, `e63fa12`부터 위로 20개 정도)

---

## 1. `__WIN32__`/`__WINDOWS__` - 오류 대부분의 진짜 원인

VC6 원본 `Client.dsp`는 항상 `/D "__WIN32__" /D "__WINDOWS__"`를 명시적으로
넘겼는데, CMake 이식 과정에서 이 두 정의가 통째로 빠져 있었음.

- **`__WIN32__`**: `Client/Packet/Exception.h`가 `#if defined(NDEBUG) ||
  defined(__WIN32__)`로 `__BEGIN_DEBUG`/`__END_DEBUG`(및 `Assert1.h`/
  `PacketAssert.h`의 `Assert()`) 매크로를 분기하는데, Windows Debug
  빌드에서는 둘 다 없어 매크로가 미정의 상태였음. Gpackets/Cpackets 다수,
  `ClientCommunicationManager.cpp`, `DatagramSocket.cpp` 등 오류 대부분의
  공통 원인이었음. **이 한 줄 추가로 오류 308건 → 112건.**
- **`__WINDOWS__`**: `Client/Packet/SocketAPI.cpp`/`PacketFileAPI.cpp` 등
  여러 파일이 `defined()` 없이 `#if __WINDOWS__`/`#elif __WINDOWS__`로
  분기하는데, 정의 안 된 매크로는 `#if`에서 0으로 취급되어 두 분기 다
  컴파일 안 되고 있었음(`fd`/`result`/`client`/`nSent` 등 그 분기 안에서만
  선언되는 지역변수가 전부 미선언 오류로 잡히던 원인).

두 매크로 다 `CMakeLists.txt`의 `DarkEden` 타겟에 `if(WIN32) ...
target_compile_definitions(DarkEden PRIVATE __WIN32__ __WINDOWS__)`로
추가함.

## 2. 컴파일 오류 0건까지 - 트리비얼 수정들

`__WIN32__`/`__WINDOWS__` 이후 남은 오류들을 파일별로 순서대로 정리.
(각각 별도 커밋 - `git log`로 커밋 메시지에 상세 설명 있음)

- `NicknameInfo.cpp` - `Assert.h` → `PacketAssert.h` 리네임 때 이 파일만
  빠짐
- `huffman.cpp` - VC6 시절 헤더명 `strstrea.h` → 표준 `<strstream>`
- `Client/DXLib/CDirectDraw.h`/`.cpp` - `CSDLGraphics`에 `m_ddsd` 멤버
  복원(SDL 이식 때 빠뜨림 - `CDirectDrawSurface`가 직접 참조)
- `reader.cpp`/`synfilt.cpp`/`subdecoder.cpp` - `mp3.cpp`/`soundbuf.cpp`와
  같은 이유로 VC6에서 DXLib.lib 전용이던 걸 CMake 글롭이 루트 사본까지
  같이 컴파일하고 있었음(실제 `<dsound.h>` 충돌) - 제외
- `WavePackFileManager.h`/`.cpp` - `LoadFromFileData()`가 항상 NULL을
  반환하는 `CSDLAudio::GetDS()`를 호출해서 실행되면 항상 크래시했을
  코드 - NULL 반환 스텁으로 정리, 실제 `<DSound.h>` include도 제거
- `StringStream.h`/`.cpp` - x64 `SOCKET`(64비트)이 `uint`/`ulong` 사이에서
  모호해지는 문제 - `ulonglong` 오버로드 추가
- `RequestClientPlayerManager.cpp` - `dwChildThreadID` 중복 선언 제거
- `GameTime.cpp`/`ClientCommunicationManager.cpp` - BOM 없어서 한글
  리터럴이 깨져 보이던 문제(C2001) - UTF-8 with BOM으로 저장
- `CrashReport.cpp` - `ADDRESS64`/`KDHELP64`/`STACKFRAME64` 등 VC6 시절
  수동 재정의가 최신 SDK `<imagehlp.h>`와 중복 - 제거. `Eip`/`Ebp`(x86
  전용) → `Rip`/`Rbp`(x64), `ios::binary | )` 구문 오류 수정
- `PCConfigTable.cpp` - `<vector>`/`<algorithm>` include 누락
- `GameMain.cpp` - Windows 분기에 `<fcntl.h>` 누락(`_O_RDONLY`),
  `SaveSurfaceToImage()` 호출 제거(SDL 백엔드에서 타입 안 맞음, 아래
  "3. 정리한 스텁들" 참고)
- `COGGSTREAM.H` - `SOUND_ERR_OK`/`SOUND_PLAY_ONCE` 상수 정의 추가
- `MTopView.cpp`/`MTopViewDraw.inl` - 옛 클래스명 `CDirectDraw::Is565()` →
  `CSDLGraphics::Is565()`, `COGGSTREAM` 생성자 인자 누락 2곳
- `basic/Platform.h` - `SetSurfaceInfo(S_SURFACEINFO*, const
  S_SURFACEINFO*)` 오버로드가 `#ifndef PLATFORM_WINDOWS`로 막혀있어서
  Windows에서 못 찾던 문제 - 가드 제거(SDL 백엔드에서는 Windows도
  이 오버로드가 필요)
- `MInternetConnection.cpp` - `<WinInet.h>` include 누락
- `SDLMain.cpp` - **버그 발견**: 파일 맨 앞 `#ifndef PLATFORM_WINDOWS`가
  그 앞에 아무 include도 없이 검사되고 있어서 `PLATFORM_WINDOWS`가 항상
  미정의 상태로 취급되어, macOS/Linux 전용으로 의도된 이 파일이 Windows
  빌드에도 그대로 컴파일되고 있었음. `SocketAPI.h`처럼 자체 플랫폼 감지를
  앞에 추가해서 진짜로 Windows에서 빠지도록 수정.
- `basic/DLL.h` - **버그 발견**: `export`는 예약된 C++ 키워드라 매크로화가
  금지되어 있는데(`<xkeycheck.h>`가 명시적으로 검사, C1189), `_CPP`가
  무조건 정의돼 있어서 Windows에서 항상 `#define export
  __declspec(dllexport)` 분기를 타고 있었음 - 표준 헤더가 include되는
  TU마다(basic/SpriteLib) 빌드가 막힘. `DllExport`/`DllImport`를 중간
  매크로 없이 바로 정의하도록 변경.
- `LeakMemoryDumper.h` - placement `operator new` 첫 매개변수가 x64에서
  `size_t`가 아니면 안 됨(C2821) - 수정
- `GCRankBonusInfo.cpp` - `write(size_t)`가 x64에서 모호(C2668) - `read()`가
  `BYTE`로 읽는 것과 맞춰 캐스트
- `ProfileManager.cpp` - `MakeProfiles()`의 Windows 분기(`CDirectDrawSurface`
  실사용)가 SDL 백엔드에서 타입이 안 맞아서, 이미 있던 non-Windows
  스텁으로 통일
- `UIMessageManager.cpp` - `CSDLGraphics::GetDD()->RestoreDisplayMode()`
  호출 2곳 제거(`참고자료/작업필요stub.md` 1-1 항목과 같은 케이스, 이
  두 지점만 빠져있었음)

**결과: `DarkEden.vcxproj` 컴파일 오류 0건.**

## 3. 링크 단계 - 여기까지 정리함

컴파일이 뚫리자 바로 링크 오류가 나왔음(`work 5.md` 문서 제목이 원래
"DarkEden exe 최초 링크 오류"였던 걸 오늘 처음으로 실제로 만남).

- `iconv.lib` 못 찾음(LNK1104) - vcpkg에 실제로 설치되어 있는데 CMake가
  `find_package` 없이 리터럴 문자열 `"iconv"`만 링커에 넘기고 있어서
  기본 검색 경로에서 못 찾고 있었음 - `find_package(Iconv)`로 전체
  경로를 가진 `Iconv::Iconv` 타겟을 쓰도록 수정
- `MItemTable_bak-2007-5-7.cpp`/`CSpritePal.cpp`(루트) 중복 컴파일 -
  `.bak`/`_bak-` 제외 필터가 `if(NOT WIN32)` 안에 있어서 Windows에는 적용
  안 되고 있었음(밖으로 이동); `CSpritePal.cpp`는 VC6에서
  `SpriteLib.dsp` 전용이었는데 CMake 글롭으로 루트 사본도 같이
  컴파일되고 있었음(`mp3.cpp` 등과 같은 문제) - 제외
- `CheckMacScreenMode()` 중복 정의 - `Globals.cpp`의 스텁과
  `PacketFunction.cpp`의 실구현이 Windows에서 같이 컴파일되고 있었음 -
  스텁을 `#ifndef PLATFORM_WINDOWS`로 제한
- `atls.lib` 못 찾음 - `VS_UI_WebBrowser.h`가 쓰는 ATL 헤더는 이미 다른
  툴셋 버전에서 찾아오도록 되어 있었는데(이전 세션에서 처리해둠), 링커가
  찾는 lib 디렉터리는 추가 안 돼 있었음 - 같은 툴셋의 `atlmfc/lib/x64`
  추가
- `ws2_32`/`winmm`/`wininet`/`comctl32`/`netapi32` 등 표준 Windows
  라이브러리가 링크 목록에 전혀 없어서, Winsock/mmio/WinInet/
  InitCommonControls/Netbios API를 실제로 쓰는 코드가 전부 미확인 외부
  기호였음 - 추가
- `platform_get_ticks` 등 `basic/PlatformSDL.cpp`의 플랫폼 추상화
  함수들 - **버그 발견**: 파일 전체가 `#ifndef PLATFORM_WINDOWS`인데
  Windows용 구현이 어디에도 없었음. `__WIN32__`/`__WINDOWS__` 수정으로
  그동안 컴파일 안 되던 코드가 풀리면서 이 함수들을 실제로 호출하는
  코드가 처음으로 컴파일된 것. Time/Keyboard/Error/Init-Shutdown류
  (스칼라 반환, 플랫폼 무관 안전)는 Windows에도 적용. Thread/Mutex/
  Event/Dynamic-Library류는 `Platform.h`에서 `platform_thread_t` 등이
  `PLATFORM_WINDOWS`에서 진짜 Win32 `HANDLE`/`HMODULE`로 정의되어
  있어서(`MWorkThread.cpp`가 이미 그렇게 씀) SDL 버전(`SDL_Thread*` 등)을
  그대로 쓸 수 없었음 - 실제로 필요한
  `platform_thread_create`/`wait`/`close`/`platform_event_close`만 진짜
  Win32 API로 새로 구현. 나머지(`platform_mutex_*`,
  `platform_event_create/wait/signal/reset`, `platform_lib_*`,
  `platform_get_executable_dir`, `platform_create_directory`, Registry/
  Configuration 함수들)는 현재 아무도 안 써서 Windows에서 미구현 상태로
  남김(뒤의 두 개는 `PATH_MAX`/`dirname()`/`readlink()` 등 POSIX 전용이라
  그냥 열면 컴파일도 안 됨).
- `g_bRunning` - `SDLMain.cpp`(오늘 Windows에서 빠지도록 고침)에서만
  정의되고 있었는데 `DXLibBackendSDL.cpp`(dxlib, Windows에도 빌드됨)가
  참조만 하고 정의가 없어졌음 - `DXLibBackendSDL.cpp`에
  `PLATFORM_WINDOWS` 분기로 직접 정의 추가

**결과: 링크 오류 152건 → 71건(고유 기호 약 50개).**

# 수정 대상

남은 링크 오류는 전부 "선언/호출은 있는데 구현이 아예 없는" 것들이라
성격이 다름 - 다음 세션에서 하나씩 어떻게 할지(실구현/스텁/제거) 상의
필요:

- **`CImm` (Immersion 진동/포스피드백 라이브러리)** - `CImmDevice`/
  `CImmPeriodic`/`CImmProject`/`CImmEffect`가 전부 `__declspec(dllimport)`
  로 선언되어 있는데 실제 Immersion I-Force SDK(2005년대 조이스틱/스티어링
  휠 포스피드백 하드웨어 SDK, 폐쇄소스)가 이 환경에 없음. `gpC_Imm`
  전역도 미정의. VS_UI 위젯 다수(`VS_UI_PointExchange`, `VS_UI_Tutorial`,
  `u_window` 등)에서 참조. 현실적으로 되살릴 방법이 없어 보임(SDK
  자체가 없음) - `CImm` 관련 호출부를 전부 no-op으로 스텁화하는 방향이
  유력해 보이지만, 얼마나 넓게 퍼져있는지 먼저 조사 필요.
- **`CAVI` (AVI/MPG 오프닝 동영상 재생)** - `COpeningUpdate::PlayMPG()`가
  `CAVI` 클래스(생성자/`OpenMPG`/`Play`/`Close`)를 쓰는데 구현이 어디에도
  없음. 오프닝 동영상 재생 기능 자체가 미구현 상태로 보임 - 스텁 처리할지
  SDL 기반으로 새로 구현할지 결정 필요.
- **`InitSocket`/`UpdateSocketInput`/`RequestDisconnect`/`GetLocalIP`/
  `WindowProc`/`RequestServerPlayerManager`(생성자/소멸자/`Init`/`Update`/
  `Disconnect`)/`g_pRequestServerPlayerManager`** - `GameInit.cpp`/
  `GameMain.cpp`/`RequestFunction.cpp`가 참조하는 별도 네트워킹
  서브시스템인데 구현이 전혀 없음. 실제 사용 중인 `SocketAPI`/
  `RequestClientPlayerManager` 계열과 별개의, 아마도 이식이 안 끝난
  레거시/중복 경로로 보임 - 실제로 필요한 기능인지, 아니면 죽은 코드라
  호출부를 제거해도 되는지 조사 필요.
- **`InitializeGL`/`InitSurface`** - `GameInit.cpp`가 참조하는 또 다른
  `__declspec(dllimport)` GL_import류 함수. `basic/GL_import.h`에 선언만
  있고(`SetSurfaceInfo(DDSURFACEDESC2*)`와 같은 부류) 구현하는 .cpp가
  없음 - 이것도 죽은 legacy DLL 의존성으로 보임.
- **JPEG 라이브러리** - `UtilityFunction.cpp`의 `LoadJPG`/`SaveJPG`가
  `jpeg_*`(libjpeg) 함수를 쓰는데 vcpkg에 `libjpeg`/`libjpeg-turbo`가
  설치되어 있지 않음(`find_package(JPEG QUIET)`가 계속 못 찾고 있었음,
  세션 시작 때부터 나던 CMake 경고). `vcpkg install libjpeg-turbo` 같은
  설치가 필요하거나, JPG 저장/로드를 스텁 처리할지 결정 필요.
- **`ReadHeader`/`MP3_ReadHeader`** - `header.obj`(DXLib mp3 디코더
  일부)가 참조하는 함수 하나, 구현 없음. 범위 작아 보임(1건) - 다음
  세션에서 바로 조사 가능.

# 요구사항
3. 코드 스타일: 
   - Visual Studio 2019 환경 , C++ 11 표준 사용중
   - Allman 스타일(중괄호 다음 줄), 헝가리언 표기법 준수.
   - 한 줄 제어문도 줄바꿈/중괄호 필수.
   - 인코딩: UTF-8 with BOM.   
4. 기존 로직 보존: 기존 프로젝트의 다른 코드는 건드리지 마세요(재탐색 금지).
5. 응답은 항상 한글로 해주세요.
