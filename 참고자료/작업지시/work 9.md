

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

## 5. 남은 링크 오류 전부 해소 - 빌드 성공

이후 대화(2026-08-21 밤 ~ 2026-08-22)에서 나머지 71건(고유 약 50개)도
전부 정리:

- **`CImm` (Immersion 포스피드백)** - `VS_UI/src/Imm/Imm*.h`는 Immersion
  Corporation IFC SDK 원본 헤더(저작권 표기 포함) 그대로였고, VC6 원본
  (`client-master_vs6/lib/IFC22.lib`)까지 확인해봤지만 **x86 전용
  prebuilt 바이너리**뿐 소스는 어디에도 없었음(x64로 링크 자체가
  불가능). `CImm.cpp`가 실제 Immersion 타입을 생성/호출하는 코드를 전부
  제거(생성자는 `m_pDevice=NULL`만, `Enable`/`Disable`/`Force*`는
  no-op)하고, `gpC_Imm` 전역이 Windows 분기에서 정의 자체가 없던 버그도
  같이 수정(50건).
- **`RequestServerPlayerManager` 클러스터** - CImm과 달리 **진짜 소스가
  있었음**. `RequestServerPlayerManager.cpp`의 `#ifdef PLATFORM_WINDOWS`
  (스레딩 include 몇 줄만 감쌀 의도)를 닫는 `#endif`가 없어서, 그 뒤
  파일 끝까지 - 생성자/`Init`/`Update`/`Disconnect`/전역까지 - 전부
  non-Windows 전용 `#elif` 분기 안에 들어가 있었음. `#endif` 추가로
  해소(8건).
- **`CAVI` (오프닝 동영상)** - 이것도 소스는 있었음(`Client/CAvi.cpp`).
  이전 세션이 "어디서도 생성 안 되는 죽은 코드"라 판단해 CMake에서
  제외해뒀는데 틀린 판단이었음(`COpeningUpdate::PlayMPG()`가 실제로
  씀). 재확인해보니 Windows 분기는 MCI Digital Video API(`MCI_DGV_*`)를
  썼는데, 이 드라이버(`MCIAVI.DRV`)가 애초에 64비트 Windows용으로 나온
  적이 없어서(SDK에서 빠진 게 아니라 OS 자체에서 없어진 기능) 실구현
  불가능 - CImm과 같은 결론으로 전 플랫폼 스텁화(5건).
  **주의**: `COpeningUpdate::PlayMPG()`가 `OpenMPG()` 실패 시 모달
  `MessageBox("Not Found <파일>")`를 띄우는 기존 로직이 있어서, 오프닝
  화면 진입 시(`GameMain.cpp`의 OPENING 모드, "test.mpg") 매번 이 팝업이
  뜰 가능성 있음 - 이번엔 안 건드림, 실행 확인 시 체크 필요.
- **`InitializeGL`** - `client-master_vs6/lib/GL.lib` 확인 결과 이것도
  x86 전용 prebuilt, 소스 없음. `GL_import.h`의 나머지 함수들
  (`rectangle`/`GL_RGB`/`Convert24RGBto16`/`Get_ColorkeyColor`)도 현재
  빌드에서 아무도 안 씀을 확인(SDL 시대 자체 구현으로 대체됐거나 유일한
  호출부가 이미 제외됨) - 스텁 함수 대신 `GameInit.cpp`의 호출 자체를
  제거(1건).
- **`Client/header.cpp`** - `reader.cpp`/`synfilt.cpp`/`subdecoder.cpp`와
  같은 부류(VC6에서 DXLib.lib 전용)를 이번에 같이 제외 처리하면서
  놓쳤던 것 - 마저 제외(1건).
- **JPEG** - `vcpkg install libjpeg-turbo:x64-windows`로 설치.
  `find_package(JPEG)`가 바로 찾아서 CMake 쪽 수정 없이 해소(13건).
  **주의**: 이 프로젝트는 `vcpkg.json` 매니페스트가 없어서(클래식 모드)
  이 설치는 vcpkg 공용 설치 디렉터리에만 반영되고 git에는 안 잡힘 -
  다른 머신에서 새로 빌드하려면 이 설치를 다시 해줘야 함.

**최종 결과: 링크 오류 0건. `DarkEden.exe` 빌드 성공**
(`build/vs2019/bin/Debug/DarkEden.exe`, LNK4217/LNK4286 경고만 있고
오류는 없음).

# 수정 대상

(현재 없음 - 컴파일/링크 오류는 전부 해소됨) 다음으로 볼 만한 것:

- 실제로 클라이언트를 띄워서 골든 패스 테스트(`CLAUDE.md`의 "UI/frontend
  변경 시 브라우저에서 테스트" 원칙과 같은 취지) - 서버 접속 없이 어디까지
  뜨는지, 오프닝 화면 진입 시 `CAVI` 스텁 관련 `MessageBox` 팝업이 실제로
  뜨는지 확인
- `참고자료/작업필요stub.md`에 쌓인 스텁 목록(CImm/CAVI/InitializeGL/
  WavePackFileManager/ProfileManager/SaveSurfaceToImage 등) 중 실제
  플레이에 필요한 게 있으면 우선순위 정해서 실구현 착수
- Release 빌드도 같은 방식으로 한번 돌려서 Debug에서만 통과하는 건
  아닌지 확인

# 요구사항
3. 코드 스타일: 
   - Visual Studio 2019 환경 , C++ 11 표준 사용중
   - Allman 스타일(중괄호 다음 줄), 헝가리언 표기법 준수.
   - 한 줄 제어문도 줄바꿈/중괄호 필수.
   - 인코딩: UTF-8 with BOM.   
4. 기존 로직 보존: 기존 프로젝트의 다른 코드는 건드리지 마세요(재탐색 금지).
5. 응답은 항상 한글로 해주세요.
