# 작업 필요 - 빈 스텁/우회로 컴파일만 통과시킨 부분 목록

이 문서는 "컴파일 오류를 없애기 위해 실제 기능 구현 없이 스텁(빈 함수)이나
우회 처리로 넘긴 부분"을 추적하기 위한 목록입니다. 빌드는 통과하지만 실제
동작(특히 Windows에서 화면이 그려지는지)은 보장되지 않는 부분들이니, 클라이언트를
실제로 띄워서 테스트할 때 우선적으로 확인해야 합니다.

---

## 1. 이번 세션(work 4 오류 수정, 커밋 `71370d6`)에서 내가 남긴 부분

### 1-1. `CSDLGraphics::GetDD()->RestoreDisplayMode()` 호출 제거 (기능 미구현 상태로 방치)

- 대상: `VS_UI/WinMain.cpp`(약 2350줄), `VS_UI/src/VS_UI_Title.cpp`(약 5512줄)
- 무엇을 했나: "외부 브라우저(Explorer.exe)로 홈페이지 URL을 열기 직전에
  전체화면 디스플레이 모드를 창모드로 복원한다"는 원래 로직의 호출부를
  **주석 처리만** 하고 대체 구현은 넣지 않음.
- 왜 그렇게 했나: `CSDLGraphics::GetDD()`(`Client/DXLib/CDirectDraw.h:207`)가
  **애초에 내가 만들기 전부터** `static inline LPDIRECTDRAW7 GetDD() { return
  nullptr; }`로 완전히 스텁화되어 있어서, 이 호출은 내가 손대기 전에도 항상
  널 포인터 역참조로 크래시하거나(런타임에 도달했다면) 컴파일조차 안 되는
  상태였음(`IDirectDraw`가 정의되지 않은 불완전 타입이라 `->` 호출 자체가
  구문 오류). 즉 "동작하던 기능을 스텁으로 되돌린" 게 아니라 "이미 죽어있던
  기능의 호출부를 안전하게 비활성화"한 것.
- **남은 일**: 전체화면 → 창모드 전환이 필요하다면, SDL2 기준으로는
  `SDL_SetWindowFullscreen(window, 0)` 같은 API로 새로 구현해야 함. 지금은
  이 두 지점 모두 그냥 아무 일도 하지 않고 바로 `Explorer.exe`를 실행함.

### 1-2. `Client/MTopView.h`의 `CD3DTextureEffect m_TextureEffect` 제거

- 이건 스텁이 아니라 **진짜 죽은 코드 삭제**임(참고용으로 구분해서 기록).
  `Client/D3DLib/CD3DTextureEffect.h/.cpp` 자체가 이미 삭제된 지 오래고, 유일한
  호출부 3곳(`DrawCreatureShadow.cpp`는 블록 주석 안, `MTopView.cpp` 2곳은
  어디서도 정의 안 되는 `__3D_IMAGE_OBJECT__` 안, `MTopViewDraw.inl` 매크로는
  호출부 자체가 주석 처리)이 전부 컴파일되지 않는 코드였음을 확인한 뒤 제거.
  실질적으로 되돌릴 기능이 없음(자세한 근거는
  `참고자료/커밋로그/2026-08-20_MTopView_h_CD3DTextureEffect_죽은_멤버_제거.md`).

그 외 이번 세션의 나머지 8개 수정(CMake 제외 조건, `MParty.h`/`ValueList.h`,
`RenderingFunctions.cpp` include, `Client.h`/`CWinUpdate.h` 헤더 충돌,
`SetSurfaceInfo`/`GetDDSD`, `WNDPROC` 캐스트, `LoadImageToSurface` 오버로드,
BOM 추가, `__builtin_expect` 제거)은 전부 **실제 동작 경로를 그대로 살리는**
수정이었고 스텁/우회 처리는 없음.

---

## 2. 지금 처리해야 하는 것 (work 4 error.log 최신본, 아직 미착수)

아래 5개는 `error C2039`(해당 클래스의 멤버가 아님)로, **선언 자체가 헤더에
없어서** 컴파일이 실패하는 상태임. 즉 "스텁으로 넘긴 것"조차 아직 안 된
순수 미구현 상태:

| 위치 | 호출부 | 필요한 것 |
| --- | --- | --- |
| `VS_UI/WinMain.cpp(3494)` | `gC_DD.Init(hwnd, RESOLUTION_X, RESOLUTION_Y, CSDLGraphics::WINDOWMODE, false, true)` | `CSDLGraphics::Init()` 신설 |
| `VS_UI/WinMain.cpp(3523)` | `gC_DDSurface.InitBacksurface()` | `CSpriteSurface::InitBacksurface()` 신설 |
| `VS_UI/WinMain.cpp(1451)` | `gC_DDSurface.ShowFPS(0, 0, RGB(255,255,255), RGB(128,0,128))` | `CSpriteSurface::ShowFPS()` 신설 |
| `Client/ClientFunction.cpp(452)` | `m_p_DDSurface_back->GammaBox565(pRect, reverseAlpha)` | `CSpriteSurface::GammaBox565()` 신설 |
| `Client/ClientFunction.cpp(457)` | `m_p_DDSurface_back->GammaBox555(pRect, reverseAlpha)` | `CSpriteSurface::GammaBox555()` 신설 |

### 조사 결과 (구현 시 참고)

- **`gC_DD`**(`CSDLGraphics`)와 **`gC_DDSurface`**(`CSpriteSurface`)는
  `VS_UI/WinMain.cpp` 96번째 줄 부근의 전역 객체. `gC_DDSurface`는
  `gC_vs_ui.Init(&gC_DDSurface, ...)`(3528줄)를 통해 `Base::m_p_DDSurface_back`으로
  들어가는, **VS_UI 전체 렌더링이 실제로 그려지는 백버퍼 그 자체**임.
- `gC_DD.Init(hwnd, ...)`에 실제 Win32 `HWND`가 넘어옴 — 이 지점 이전
  (3448~3484줄)에서 `CreateWindowEx()`로 이미 진짜 네이티브 윈도우를 만들어
  놓은 상태. SDL2는 `SDL_CreateWindowFrom(void*)`로 기존 네이티브 핸들을
  감쌀 수 있음.
- **`Client/SDLMain.cpp`가 이미 완전히 동작하는 SDL 윈도우/렌더러 생성 +
  프레임 루프 코드를 갖고 있지만, 파일 전체가 `#ifndef PLATFORM_WINDOWS`로
  감싸여 있어 Windows 빌드에서는 통째로 빠짐.** 즉 Windows용
  `CSDLGraphics::Init()`을 새로 만들 때 이 파일의 `InitApp()`(90~170줄) 로직을
  참고/이식할 수 있음(`SDL_CreateWindow` 대신 `SDL_CreateWindowFrom(hwnd)`만
  다르게).
- `CSDLGraphics::Flip()`(`CDirectDraw.h:210`)도 **이미 빈 no-op 스텁**임.
  `Init()`을 구현해 SDL 윈도우/렌더러를 진짜로 만들어도, 화면 프레젠트
  (`SDL_RenderPresent` 호출)는 `Flip()`이 담당해야 하는데 지금은 아무것도 안 함.
  즉 이번 5건만 고쳐도 **Windows에서 실제로 화면이 갱신되려면 `Flip()`도
  함께 손봐야 할 가능성이 높음**(범위를 어디까지 잡을지 결정 필요).
- `CSpriteSurface::Init(int, int)`/`InitOffsurface(int, int)`
  (`CSpriteSurface_SDL.cpp:65, 102`)는 이미 동작하는 함수라
  `InitBacksurface()`는 이걸 감싸는 얇은 래퍼로 구현 가능. 다만 현재
  `CSDLGraphics::GetScreenWidth()`/`GetScreenHeight()`가 **하드코딩된 800/600
  스텁**(`CDirectDraw.h:201-202`)이라, `Init()`에서 실제 해상도(`RESOLUTION_X`
  `= 1024`, `VS_UI_Base.h:160`)를 저장하도록 함께 고치지 않으면
  `InitBacksurface()`가 잘못된 크기로 만들어짐.
- `CSpriteSurface::Gamma4Pixel565`/`Gamma4Pixel555`(`CSpriteSurface_SDL.cpp:618`
  이하)는 **이미 동작하는 유틸리티 함수**(다만 원래 x86 어셈블리 최적화 버전
  대신 기본 RGB 스케일링으로 대체된 상태, 618줄 TODO 주석 있음). `GammaBox565`
  `/555`는 이 함수를 사각형 영역의 각 행에 적용하는 래퍼로 구현 가능해 보임.

이번에 실제로 구현할지, 아니면 이전처럼 "컴파일만 통과하는 최소 스텁"으로
넘길지는 별도로 상의해서 결정하기로 함(2026-08-21 대화에서 사용자에게
선택지 제시 중).

**후속 처리(같은 날 이후 세션)**: `VS_UI/WinMain.cpp`를 죽은 진입점으로 확정해
빌드에서 제외하면서, 위 5건 중 `ShowFPS`/`CSDLGraphics::Init`/
`InitBacksurface` 3건은 호출부 자체가 사라져 해소됨(자세한 내용은
`참고자료/커밋로그/2026-08-21_VS_UI_WinMain_cpp_죽은_진입점_제외.md`).
나머지 `GammaBox565`/`GammaBox555`는 VC6 원본을 참고해 실제로 구현함(같은
커밋).

---

## 2-1. `work 5` (DarkEden.exe 최초 링크) 오류 조사 중 남긴 스텁 - MIDI/MCI 배경음악

`Client/MMusic.cpp`(MIDI 기반 배경음악, `g_Music`)와 `Client/CMP3.cpp`(MCI
기반 MP3 재생, `g_pMP3`)는 둘 다 원래 실제 Win32 `<MMSystem.h>` API(MCI/
`midiOutOpen`/`mciSendString` 등)를 직접 호출하는 Windows 전용 구현을 갖고
있었음. 그런데 `basic/Platform.h`가 이미 "이 프로젝트는 real `<MMSystem.h>`를
어디서도 include하지 않는다"는 원칙 하에 `timeGetTime()`/`GetTickCount()`를
`platform_get_ticks()`로 매크로 치환해두고 있어서, 이 두 파일에서 real
`<MMSystem.h>`를 include하면 그 매크로와 충돌해 새 컴파일 오류가 남(예:
`timeGetTime(void)` 선언부가 인자 0개짜리 매크로와 충돌).

- **`CMP3`(MP3/MCI)**: 조사 결과, 이 클래스를 실제로 생성/재생하는 코드 경로는
  전부 `#ifdef __USE_MP3__`(`Client/SoundSetting.h:4`에서 영구적으로
  `//#define`으로 비활성화됨)로 감싸여 있어 **이미 도달 불가능한 상태**였음
  (실제 배경음악 재생은 `COGGSTREAM`/`g_pOGG` 기반 OGG 스트리밍 경로가 담당).
  다만 `g_pMP3->Stop()`/`SetVolume()` 등 일부 null-guard된 호출은 여전히
  살아있어 클래스 자체는 링크에 필요함. `CMP3.cpp`에는 이미 완전한 비-Windows
  스텁 구현(`#else` 분기)이 있었으므로, Windows에서도 이 스텁을 쓰도록
  `#ifdef PLATFORM_WINDOWS`를 `#if defined(PLATFORM_WINDOWS) &&
  defined(__USE_MP3__)`로 바꿈(같은 파일의 기존 플래그를 그대로 재사용 -
  `__USE_MP3__`가 켜지지 않는 한 real MCI 코드는 컴파일조차 안 됨).
- **`MMusic`(MIDI)**: `g_Music.Play()`는 `GameMain.cpp`/`UIMessageManager.cpp`
  에서 "사용자가 PlayWaveMusic(WAV/OGG 음악)을 끈 경우"의 폴백 경로로 여전히
  **런타임에 도달 가능**함 - `CMP3`와 달리 진짜 죽은 코드는 아님. 다만 real
  MIDI 재생을 SDL 기반으로 재구현하는 건 이번 컴파일 오류 수정 범위를 크게
  벗어나는 별도 기능 작업이라 판단해, `CMP3`와 동일한 패턴으로 새 플래그
  `__USE_REAL_MIDI__`(영구 미정의)를 도입하고 이미 있던 비-Windows 스텁
  구현으로 Windows도 라우팅함.
- **결과**: 이 폴백 경로를 타는 사용자는 Windows에서도 MIDI 배경음악이 아무
  소리도 내지 않음(상태 추적은 정상 동작 - `IsPlay()`/`IsPause()` 등은 그대로
  참/거짓을 반환하지만 실제 재생은 no-op). **남은 일**: `__USE_REAL_MIDI__`를
  실제로 구현하려면 SDL2/SDL_mixer 기반 MIDI 재생(예: 폰트 기반 MIDI 신디사이저
  연동)이 필요함.

---

## 3. 조사 중 발견한, 내가 만들지 않은 기존 스텁 (Windows 렌더링 파이프라인 관련)

`work 4` 오류를 조사하며 알게 된, 이미 프로젝트에 존재하던 미구현 상태.
이번 작업 범위는 아니지만 "화면이 실제로 그려지는가"와 직결되므로 함께 기록.

### `Client/DXLib/CDirectDraw.h`의 `CSDLGraphics` — 클래스 대부분이 no-op 스텁

```cpp
static inline void  Flip() { }
static inline void  FlipToGDISurface() { }
static inline void  OnMove() { }
static inline bool  RestoreAllSurfaces() { return true; }
static inline void  ReleaseSurface() { }
static inline void  ReleaseAll() { }
static inline void  SetGammaRamp(WORD step = (WORD)-1) { }
static inline void  RestoreGammaRamp() { }
static inline void  SetAddGammaRamp(WORD, WORD, WORD) { }
static inline void  SetDisplayMode(WORD, WORD, WORD, DWORD, DWORD) { }
static inline void  RestoreDisplayMode() { }
static inline bool  IsFullscreen()  { return true; }   // 하드코딩
static inline WORD  GetScreenWidth()  { return 800; }  // 하드코딩
static inline WORD  GetScreenHeight() { return 600; }  // 하드코딩
static inline bool  IsSupportGammaControl() { return false; }
static inline HWND  GetHwnd()  { return nullptr; }     // 하드코딩
static inline LPDIRECTDRAW7 GetDD() { return nullptr; } // 하드코딩 (1-1 항목의 원인)
```

### `Client/SpriteLib/CSpriteSurface_SDL.cpp` — TODO로 남은 메서드들

- `InitFromFile()` (81줄): BMP 파일 로드 미구현, 항상 `false` 반환
- `GDI_Text()` (118줄): 텍스트 렌더링 미구현 (SDL2_ttf 연동 필요)
- `Line()` (249줄): 대각선 미구현, 수평/수직선만 지원
- `BltHalf`, `BltDarkness`, `BltBrightness`, `BltDarknessFilter`,
  `ChangeBrightnessBit`, `BltColorAlpha` (270~359줄): 전부 빈 함수
- `memcpyHalf`, `memcpyAlpha`, `memcpyColor`, `memcpyScale`,
  `memcpyDarkness`, `memcpyBrightness` (366~394줄): 전부 빈 함수
- `ClippingRectToPoint()` (400줄): 실제 클리핑 없이 항상 `true` 반환
- `Gamma4Pixel565`/`Gamma4Pixel555` (618줄~): 동작은 하지만 원래 x86 어셈블리
  최적화 버전 대신 기본 RGB 스케일링 사용 중(TODO 주석 있음)

### `Client/SDLMain.cpp` — Windows에서 통째로 빠짐

파일 전체가 `#ifndef PLATFORM_WINDOWS` / `#endif`로 감싸여 있어, Windows
빌드에서는 이 파일의 SDL 윈도우 생성/렌더러/프레임 루프 코드가 전혀
컴파일되지 않음. Windows에서는 `VS_UI/WinMain.cpp`의 `WinMain()`이 유일한
진입점인데, 여기서 필요로 하는 `CSDLGraphics::Init()`이 아직 없어(위 2번
항목) 사실상 Windows 빌드는 창은 뜨더라도 SDL 렌더러가 붙지 않은 상태로
추정됨(직접 실행해서 확인 필요).

## 3. work 8 오류 수정 (2026-08-21)에서 내가 남긴 부분

### 3-1. `WavePackFileInfo::LoadFromFileData()` — 항상 `NULL` 반환하도록 스텁화

- 대상: `Client/WavePackFileManager.cpp`
- 무엇을 했나: `DSBUFFERDESC`를 만들어 `g_SDLAudio.GetDS()->CreateSoundBuffer(...)`를
  호출하고 `buffer->Lock()/Unlock()`으로 WAV 데이터를 채워넣던 원래 구현을
  제거하고, 파일을 읽지 않고 바로 `NULL`을 반환하도록 바꿈.
- 왜 그렇게 했나: `CSDLAudio::GetDS()`(`Client/DXLib/CDirectSound.cpp:273`)가
  **애초에 내가 만들기 전부터** 항상 `NULL`로 초기화되고 재할당되지 않는
  `m_pDS`를 그대로 반환하는 스텁이라(실제 재생은 SDL_mixer로 처리),
  이 함수가 실행됐다면 `nullptr->CreateSoundBuffer(...)`로 항상 크래시했을
  것임. 즉 1-1 항목(`CSDLGraphics::GetDD()`)과 같은 성격의, 내가 손대기
  전부터 이미 죽어있던 코드.
  또한 이 함수가 실제 `<DSound.h>`를 필요로 하는 유일한 지점이었는데, 같은
  번역 단위에서 `basic/AudioTypes.h`가 먼저 처리된 뒤(Client_PCH.h 경유)
  `<DSound.h>`가 include되면 `_DSBPOSITIONNOTIFY`가 중복 정의되는 문제
  (`error C2011`, work5 사운드 서브시스템 정리 때 다룬 것과 같은 부류)가
  있어서, 이 함수를 스텁화하면서 `WavePackFileManager.h`의
  `#include <DSound.h>`도 제거하고 `LPDIRECTSOUNDBUFFER`를 opaque
  pointer typedef로 직접 선언하도록 바꿈(`CSoundPartManager.h`/
  `MZoneSoundManager.h`에 이미 있던 것과 같은 패턴).
- **남은 일**: WAV 파일을 실제 재생 가능한 사운드 버퍼로 로드하는 기능이
  필요하다면, SDL_mixer 기준(`Mix_LoadWAV`류)으로 새로 구현해야 함. 지금은
  이 경로를 타면 그냥 사운드 없이 `NULL`을 돌려받음.

### 3-2. `GameMain.cpp`의 스크린샷 JPG 저장 호출 제거 (기능 미구현 상태로 방치)

- 대상: `Client/GameMain.cpp`(약 3540줄), `TakeScreenShot()` 계열 함수
- 무엇을 했나: `SaveSurfaceToImage(str, *g_pBack)` 호출을, 이미 non-Windows
  경로에 있던 것과 같은 "not yet implemented" `printf`로 교체(Windows/
  non-Windows 공통 경로가 됨).
- 왜 그렇게 했나: `SaveSurfaceToImage()`(`UtilityFunction.cpp:495`)는
  `CDirectDrawSurface&`를 받는데, `SPRITELIB_BACKEND_SDL`(현재 이 프로젝트가
  Windows 포함 항상 쓰는 유일한 백엔드)에서는 `CSpriteSurface`가
  `CDirectDrawSurface`를 상속하지 않는 독립 클래스로 바뀜
  (`CSpriteSurface.h`의 `SPRITESURFACE_STANDALONE` 분기). `g_pBack`은
  `CSpriteSurface*`라서 이 호출은 SDL 백엔드에서 애초에 타입이 맞은 적이
  없었음(non-Windows 경로는 이미 같은 이유로 "not yet implemented"로
  처리되어 있었음 - Windows 경로만 안 맞춰져 있었던 것).
- **남은 일**: 스크린샷 저장이 필요하다면 `CSpriteSurface` 기준으로 새로
  구현해야 함(BMP는 `SaveToBMP`가 있지만 `Get_BPP()`/`Lock()`이 없고
  `LockSDL()`로 이름이 바뀌어 있어 JPG 경로는 그대로 못 씀 - stb_image_write
  같은 걸로 새로 짜는 게 나을 수 있음).

### 3-3. (정정) `Client/SDLMain.cpp` — 실제로는 Windows 빌드에도 컴파일되고 있었음

- 위 "2." 항목의 "Windows에서 통째로 빠짐"이라는 서술이 **틀렸음**을 이번에
  발견함. 파일 맨 앞(19번째 줄)의 `#ifndef PLATFORM_WINDOWS`가 그 앞에
  `Client_PCH.h`(또는 `basic/Platform.h`)를 전혀 include하지 않은 채로
  검사를 하고 있었음 - `PLATFORM_WINDOWS`는 CMake `/D` 플래그가 아니라
  `basic/Platform.h`에서 `_WIN32`/`_WIN64`로 정의되는 매크로라서, 이 파일
  자신은 플랫폼과 무관하게 항상 "정의 안 됨" 상태로 그 줄을 만났음. 그 결과
  `#ifndef` 분기가 **항상** 참이 되어, macOS/Linux 전용으로 의도된 이
  파일이 Windows 빌드에도 그대로 컴파일되고 있었음(`getcwd()` 등
  POSIX-only 심볼 미선언으로 실제 오류가 남).
- 고친 내용: `#ifndef PLATFORM_WINDOWS` 검사 앞에 `Client/Packet/SocketAPI.h`와
  같은 방식의 자체적인 `#if defined(_WIN32) || defined(_WIN64)` 플랫폼 감지를
  추가해서, 이 파일 혼자서도 올바르게 플랫폼을 판단하도록 함.
- 결과: 이제 이 파일은 원래 의도대로 Windows 빌드에서 완전히 빠짐(위 "2."
  항목이 애초에 서술하려던 상태가 이제야 실제로 맞음).

### 3-5. `CAVI` (오프닝 동영상 재생) - 스텁화

- 대상: `Client/CAvi.cpp`
- 무엇을 했나: `OpenMPG()`/`OpenAVI()`/`Close()`/`Play()`/`Stop()`을 전부
  플랫폼 상관없이 항상 실패/no-op으로 통일(기존에 있던 non-Windows 전용
  스텁과 같은 내용을 Windows에도 적용).
- 왜 그렇게 했나: 원래 Windows 분기는 MCI Digital Video API
  (`MCI_DGV_OPEN_PARMS` 등, `MCIAVI.DRV` 드라이버 기반의 옛 AVI/MPEG
  재생 방식)를 썼는데, 이 구조체들이 최신 Windows 10 SDK
  `<mmsystem.h>`에 더 이상 선언되어 있지 않아 컴파일이 안 됨. 게다가
  이 MCI 드라이버 자체가 애초에 64비트 Windows용으로 나온 적이 없어서,
  설령 구조체를 직접 정의해서 컴파일만 통과시키더라도 x64에서는
  `mciSendCommand()`가 그냥 실패했을 것 - 헤더에서 빠진 게 아니라 OS
  자체에서 없어진 기능이라 실구현이 불가능함(CImm/Immersion과 같은
  성격).
  이전 세션에 "CAvi 클래스는 어디서도 생성 안 됨(죽은 코드)"이라는
  이유로 아예 컴파일에서 제외돼 있었는데, 오늘 재확인해보니 그 판단이
  틀렸음 - `COpeningUpdate::PlayMPG()`(`COpeningUpdate.cpp`)가 실제로
  `CAVI`를 생성하고 `OpenMPG()`/`Play()`를 호출하고 있었음(이전엔 다른
  컴파일 오류들에 가려서 이 링크 오류가 안 보였을 뿐). CMake 제외
  대신 클래스 자체를 스텁화하는 쪽으로 처리.
- **남은 일(주의)**: `COpeningUpdate::PlayMPG()`가 `OpenMPG()` 실패 시
  `MessageBox(g_hWnd, "Not Found <파일명>", "Error!", MB_OK)`로 **모달
  팝업**을 띄우는 기존 로직이 있음. `OpenMPG()`가 이제 항상 실패를
  반환하므로, `GameMain.cpp`에서 오프닝 화면으로 진입할 때마다
  ("test.mpg" 재생 시도, `SetMode`의 OPENING 분기) 이 팝업이 매번 뜰 것
  으로 보임. `CAVI` 자체의 스텁화 범위 밖이라 이번엔 안 건드렸지만,
  실제로 오프닝 화면에 도달하는 경로라면 `PlayMPG()`/그 호출부도 같이
  손봐야 함(예: 팝업 없이 조용히 건너뛰게).


### 3-4. `ProfileManager.cpp`의 프로필 이미지 로딩 - Windows 분기도 SDL 스텁으로 통일

- 대상: `Client/ProfileManager.cpp`, `MakeProfiles()`(BMP -> SPK 변환 부분)
- 무엇을 했나: `#ifdef PLATFORM_WINDOWS`(실제 BMP 로드 -> `CDirectDrawSurface`
  로 Blt/Lock해서 SPK에 픽셀 채워넣기)/`#else`(이미 있던 "not yet
  implemented" 스텁 - 빈 프로필만 생성) 두 분기를 `#else` 쪽 스텁 하나로
  통일.
- 왜 그렇게 했나: `CSpriteSurface surface`/`CDirectDrawSurface bmpSurface`를
  쓰던 Windows 분기가, `SPRITELIB_BACKEND_SDL`(Windows 포함 현재 유일한
  백엔드)에서 `CSpriteSurface`가 더 이상 `CDirectDrawSurface`를 상속하지
  않는 독립 클래스로 바뀌면서 애초에 타입이 맞은 적이 없었음
  (`LoadImageToSurface(bmpFilename, bmpSurface)`도 `CDirectDrawSurface&`
  오버로드만 있어서 안 맞았음). non-Windows 분기는 이미 이 문제를 알고
  "TODO: SDL_image 기반으로 새로 구현 필요"로 스텁 처리되어 있었으므로,
  같은 처리를 Windows에도 그대로 적용.
- **남은 일**: 프로필 캐릭터 초상화 BMP 로딩이 필요하다면 `CSpriteSurface`
  기준으로 새로 구현해야 함(SDL_image 등으로 로드 후 `SPK[].SetPixelNoColorkey()`
  에 픽셀을 채워넣는 경로). 지금은 항상 빈(검은) 프로필 이미지가 생성됨.
