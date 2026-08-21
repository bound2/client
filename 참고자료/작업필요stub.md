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
