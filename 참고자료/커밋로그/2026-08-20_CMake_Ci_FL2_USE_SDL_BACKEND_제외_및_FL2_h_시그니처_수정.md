# 커밋 로그

- 일시: 2026-08-20
- 대상: `CMakeLists.txt`, `VS_UI/src/hangul/Fl2.h`, `VS_UI/src/VS_UI_Base.cpp`(검증만),
  `VS_UI/src/vs_ui_gamecommon2.cpp`, `VS_UI/src/header/VS_UI_ExtraDialog.h`,
  `VS_UI/src/VS_UI_ExtraDialog.cpp`, `VS_UI/src/VS_UI_Title.cpp`, `VS_UI/src/Vs_ui.cpp`,
  `VS_UI/src/widget/U_edit.cpp`
- 유형: fix (빌드 오류 수정, VS_UI - Windows에서도 SDL 백엔드가 강제되는데
  레거시 Win32 GDI/IME 구현이 여전히 함께 컴파일되던 문제)

## 배경

`work 4 error.log`에서 `VS_UI/src/hangul/Ci.cpp`(`CI_KOREAN`/`CI_CHINESE` 생성자·소멸자
`error C2600`), `VS_UI/src/hangul/FL2.cpp`(`'c'` 미선언, `IDirectDrawSurface` 미정의),
그리고 이 둘과 얽힌 `g_GetStringWidth`/`g_GetStringHeight` 오버로드 모호성 오류가
대량 발생했음.

`CMakeLists.txt` 54번째 줄에 "SDL backend is now mandatory on all platforms (Windows
native support removed)"라는 주석과 함께 `USE_SDL_BACKEND`가 항상 강제 `ON`으로
설정되어 있는데, 161~182번째 줄의 VS_UI 소스 필터링 로직은 `Ci.cpp`/`FL2.cpp`(레거시
Win32 GDI/IME 구현)를 `if(NOT WIN32)`일 때만 제외하고 있었음. 즉 Windows 빌드에서는
`USE_SDL_BACKEND`가 켜져 있어도 옛 코드가 그대로 함께 컴파일되어, 이미 존재하는
SDL 대체 구현(`VS_UI/src/hangul/Ci_macOS.cpp`, `Client/RenderingFunctions.cpp`)과
충돌했음:

- `Ci.cpp`의 `CI_KOREAN`/`CI_CHINESE` 생성자·소멸자가 `error C2600`(클래스에 선언되지
  않은 컴파일러 생성 특수 멤버 함수 정의 불가)로 실패
- `FL2.cpp`가 실제 `IDirectDrawSurface::GetDC()`/`ReleaseDC()`를 호출하는 방식이라
  `IDirectDrawSurface` 미정의 오류 발생
- `FL2.h`가 `g_GetStringWidth`/`g_GetStringHeight`를 `HFONT` 매개변수로 선언하는데,
  실제 구현체(`RenderingFunctions.cpp`)는 `void*`를 받아 오버로드 모호성(`C2668`) 발생
- `FL2.h`의 `g_SetFL2Surface`/`gpC_fl2_surface`도 `#ifdef PLATFORM_WINDOWS`일 때
  `LPDIRECTDRAWSURFACE7` 시그니처를 요구해, `CSpriteSurface*`를 넘기는 실제 호출부와
  불일치(`VS_UI_Base.cpp(343)`) — 2026-08-18 커밋로그에서 이미 "범위 밖"으로 남겨두었던
  항목이 이번 근본 원인 수정으로 함께 해소됨
- `PrintInfo::hfont`가 `HFONT`로 선언되어 있어, `TextSystem::EncodeFontSizeHandle()`이
  반환하는 `void*`를 대입할 수 없음(`VS_UI_Base.cpp(59)`)
- `VS_UI_ExtraDialog.h`/`.cpp`의 `m_temp_face`/`bmpSurface`가 `#ifdef PLATFORM_WINDOWS`일
  때 `CDirectDrawSurface`로 선언되어, `CSpriteSurface*`만 받는
  `CSpriteSurface::BltNoColorkey()`에 넘길 수 없음
- `vs_ui_gamecommon2.cpp`도 동일 패턴: `#ifdef PLATFORM_WINDOWS` 분기가 캐스트 없이
  `BltNoColorkey`를 호출해 실패(`#else` 분기에는 이미 올바른
  `reinterpret_cast<CSpriteSurface*>` 캐스트가 있었음)
- `U_edit.cpp`가 `SDL_Renderer* g_pSDLRenderer`를 `#ifdef PLATFORM_MACOS` 밖에서
  extern 선언하면서, `<SDL2/SDL.h>` include는 `PLATFORM_MACOS` 안에만 있어
  `SDL_Renderer` 타입 자체가 미선언
- `VS_UI_Title.cpp`/`Vs_ui.cpp`의 `gpC_Imm->Enable(true)` 호출이 인수 없는
  `CImm::Enable()`에 인수를 넘겨 `C2660`

## 커밋 메시지

```
fix: Windows/SDL 백엔드 강제 전환에 따른 Ci.cpp/FL2.cpp 레거시 GDI 구현 제외 및
연쇄 시그니처 불일치 수정

USE_SDL_BACKEND가 모든 플랫폼(Windows 포함)에서 강제 ON인데, VS_UI 소스 필터링이
Ci.cpp/FL2.cpp(레거시 Win32 GDI/IME 구현)를 NOT WIN32일 때만 제외하고 있어
Windows 빌드에서 이미 존재하는 SDL 대체 구현(Ci_macOS.cpp, RenderingFunctions.cpp)과
충돌하던 문제 수정. CMakeLists.txt의 제외 조건을 USE_SDL_BACKEND 기준으로 변경하고,
FL2.h의 g_GetStringWidth/g_GetStringHeight/g_SetFL2Surface/PrintInfo::hfont를
실제 구현(void* 기반)에 맞춰 정리. 함께 드러난 CSpriteSurface/CDirectDrawSurface
타입 불일치(VS_UI_ExtraDialog, vs_ui_gamecommon2), SDL_Renderer 미선언(U_edit.cpp),
CImm::Enable() 인수 불일치(VS_UI_Title.cpp, Vs_ui.cpp)도 함께 수정.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `CMakeLists.txt` | `Ci.cpp`/`FL2.cpp` 제외 조건을 `if(NOT WIN32)`에서 별도의 `if(USE_SDL_BACKEND)` 블록으로 분리(다른 `NOT WIN32` 전용 항목은 그대로 유지) |
| `VS_UI/src/hangul/Fl2.h` | `g_GetStringWidth`/`g_GetStringHeight` 매개변수를 `HFONT`→`void*`로 변경. `g_SetFL2Surface`/`gpC_fl2_surface`의 `#ifdef PLATFORM_WINDOWS`(`LPDIRECTDRAWSURFACE7`) 분기 제거, `void*` 버전만 유지. 이제 불필요해진 `CDirectDraw.h` include 제거. `PrintInfo::hfont`를 `HFONT`→`void*`로 변경 |
| `VS_UI/src/header/VS_UI_ExtraDialog.h` | `m_temp_face`의 `#ifdef PLATFORM_WINDOWS`(`CDirectDrawSurface`) 분기 제거, `CSpriteSurface`로 통일 |
| `VS_UI/src/VS_UI_ExtraDialog.cpp` | 로컬 변수 `bmpSurface`도 동일하게 `CSpriteSurface`로 통일(`m_temp_face`와 타입 일치 필요) |
| `VS_UI/src/vs_ui_gamecommon2.cpp` | `BltNoColorkey` 호출의 `#ifdef PLATFORM_WINDOWS`(캐스트 없음) 분기 제거, `#else`에 있던 `reinterpret_cast<CSpriteSurface*>` 버전만 유지 |
| `VS_UI/src/widget/U_edit.cpp` | `extern SDL_Renderer* g_pSDLRenderer;`를 `#ifdef PLATFORM_MACOS` 블록 안(해당 타입의 유일한 `<SDL2/SDL.h>` include 위치)으로 이동 |
| `VS_UI/src/VS_UI_Title.cpp`, `VS_UI/src/Vs_ui.cpp` | `gpC_Imm->Enable(true)` → `gpC_Imm->Enable()` (해당 함수는 원래부터 인수를 받지 않음) |

## 검증

- `cmake .`로 `build/vs2019` 재생성 확인: `Ci.cpp`/`FL2.cpp`는 VS_UI 소스 목록에서
  빠지고 `Ci_macOS.cpp`만 포함됨을 확인
- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64`
- 이 항목들이 유발하던 오류(`CI_KOREAN`/`CI_CHINESE` `C2600`, `FL2.cpp`의 `'c'`
  미선언·`IDirectDrawSurface` 미정의, `g_GetStringWidth`/`g_GetStringHeight`
  `C2668`, `g_SetFL2Surface`/`HFONT` 변환 `C2440`, `BltNoColorkey` `C2664`,
  `SDL_Renderer` 미선언, `CImm::Enable` `C2660`) 전부 0건으로 해소됨을 확인
