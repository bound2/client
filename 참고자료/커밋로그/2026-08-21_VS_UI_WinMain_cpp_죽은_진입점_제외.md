# 커밋 로그 (중요 - 아키텍처 결정)

- 일시: 2026-08-21
- 대상: `CMakeLists.txt`, `Client/SpriteLib/CSpriteSurface.h`,
  `Client/SpriteLib/CSpriteSurface_SDL.cpp`
- 유형: fix (빌드 오류 수정 + 아키텍처 정리, VS_UI/Client - 중복 `WinMain` 진입점
  중 죽은 쪽 제외)

## 배경

`work 4 error.log`에 마지막까지 남아있던 5건(`ShowFPS`, `CSDLGraphics::Init`,
`CSpriteSurface::InitBacksurface`, `GammaBox565`, `GammaBox555`)을 실제로
구현하려고 SDL 윈도우/렌더러 연결까지 준비하던 중, **이 5건을 다 고쳐서 컴파일이
통과해도 링크 단계에서 새로운 오류가 날 수밖에 없는 훨씬 근본적인 구조 문제**를
발견함.

### 발견한 문제: `WinMain`이 두 파일에 중복 정의되어 있었음

- `VS_UI/WinMain.cpp`(3189줄): `int WINAPI WinMain(HINSTANCE hInst, ...)`
- `Client/Client.cpp`(2936줄): `int PASCAL WinMain(HINSTANCE hInstance, ...)`

두 파일 다 `CMakeLists.txt`에서 **같은 `VS_UI` 정적 라이브러리**로 컴파일됨.
전역 변수 `HWND g_hWnd`, `RECT g_GameRect` 등도 두 파일 모두 각자 정의하고
있었음. 지금까지는 컴파일 오류(오늘 다루던 5건 포함)에 막혀 링크 단계까지
한 번도 도달한 적이 없어 드러나지 않았을 뿐임.

### 어느 쪽이 죽은 코드인가 — VC6 원본과 비교해 확인

사용자가 제공한 두 참고 경로를 분석함:
- `H:\VMWareSetting\...\client-master`(SDL 마이그레이션 이전 VC6 참고 소스)
- `H:\Source\GithubDesktop\client-master_vs6`(실제로 VC6에서 `fengshen.exe`를
  빌드하던 원본 소스, `.dsp`/`.dsw` 워크스페이스 포함)

`client-master_vs6/Client/Client.dsp`(실제 EXE 프로젝트)의 링크 설정을 보면
`..\VS_UI\Release\VS_UI.lib`를 **라이브러리로만** 링크하고, `Client.cpp`는
EXE 프로젝트 자신의 소스로 직접 포함됨. VC6 원본에서도 `Client.cpp`와
`VS_UI/WinMain.cpp` 둘 다 `WinMain`을 정의하고 있어 애초부터 이런 구조였던
것으로 보이며, 정적 라이브러리는 필요한 심볼만 선택적으로 링크되므로
`Client.cpp`(EXE에 직접 포함)가 먼저 `WinMain`을 채우면 `VS_UI.lib` 안의
`WinMain.cpp`는 애초에 한 번도 링크된 적이 없었을 가능성이 높음.

결정적 증거는 **실제 게임을 구동하는 호출이 어느 쪽에 있는가**였음:

- `Client/Client.cpp`의 `InitApp()`/`WinMain()`: `InitGame()`,
  `g_pTopView->Init()`, `g_pUpdate`(매 프레임 게임 업데이트),
  `ReleaseAllObjects()`를 전부 호출 — 실제 게임 로직을 구동하는 완전한
  진입점
- `VS_UI/WinMain.cpp`: 이 중 **어느 것도 호출하지 않음**
  (`InitGameStringTable()`만 호출). `gC_vs_ui.Process()/Show()/
  DrawMousePointer()`만 돌려서 UI(메뉴·타이틀 화면)만 그리는, 게임 본체는
  시작도 안 하는 얕은 셸/프로토타입이었음
- macOS/Linux용 `Client/SDLMain.cpp` 파일 머리말 주석에 **"Based on WinMain
  from Client.cpp (lines 3020-4500)"**라고 명시되어 있어, SDL 마이그레이션
  당시 실제 포팅 기준이 `Client.cpp`였음을 코드 자체가 증언함

반면 현재 `CMakeLists.txt`에는 (과거 세션이 남긴 것으로 보이는) "SDL2main이
`VS_UI/WinMain.cpp`의 진짜 WinMain과 충돌한다"는 주석이 있었는데, 이는
`Client.cpp`도 `WinMain`을 정의한다는 사실을 인지하지 못한 채 내려진 판단으로
보임. 실제로 확인해보니 `VS_UI/WinMain.cpp`가 정의하는 전역/함수들
(`gC_DD`, `gC_DDSurface`, `gbl_active`, `gbl_ui_input_state`, `ProgramLoop`
등)은 전부 자기 파일 안에서만 쓰이거나, `Client.cpp`/`GameInit.cpp`/
`GameUI.cpp` 쪽에 이미 동일한 진짜 구현이 따로 있어(`g_GameRect`, `g_MyFull`,
`UI_ResultReceiver` 등) 제외해도 안전함을 하나하나 대조해 확인함.

## 결정 및 조치

사용자와 상의해 **`VS_UI/WinMain.cpp`를 죽은 진입점으로 확정하고 빌드에서
제외**함(`Client/Client.cpp`가 유일한 실제 `WinMain`으로 남음).

1. `CMakeLists.txt`의 VS_UI 소스 필터에 `WIN32`일 때도
   `VS_UI/WinMain.cpp`를 제외하는 블록 추가(기존에는 `NOT WIN32`일 때만
   제외되고 있었음 - 이 파일이 SDL 마이그레이션 이전부터 macOS/Linux 빌드
   에서는 애초에 안 쓰였다는 뜻이기도 함)
2. "SDL2main이 `VS_UI/WinMain.cpp`와 충돌한다"고 되어 있던 주석 2곳을
   "`Client/Client.cpp`와 충돌한다"로 정정(실제 `if(NOT WIN32) ...
   SDL2main endif()` 로직 자체는 그대로 유효 - 원인 파일만 바로잡음)

이 제외 덕분에, `work 4 error.log`의 5건 중 `ShowFPS`/`CSDLGraphics::Init`/
`InitBacksurface` 3건은 **호출부 자체가 사라져 더 이상 구현할 필요가 없어짐**
(호출하던 유일한 곳이 `VS_UI/WinMain.cpp`였음을 확인함).

## 남은 2건: `GammaBox565`/`GammaBox555` 실제 구현

`Client/ClientFunction.cpp`의 `DrawAlphaBox()`(반투명 색상 박스를 그리는,
실제로 살아있는 게임 UI 효과 함수)가 여전히 이 둘을 호출하고 있어 실제
구현이 필요했음. VC6 원본(`Client/DXLib/CDirectDrawSurface.cpp`)의
`GammaBox565`/`GammaBox555` 구현을 참고해 `CSpriteSurface`로 포팅함:

- `pRect`를 서피스 크기로 클리핑(SDL 이식판은 `m_ClipLeft`/`Top`/`Right`/
  `Bottom` 멤버가 선언만 되어있고 어디서도 초기화/설정되지 않아 — 확인 결과
  값이 항상 쓰레기값 — 원본처럼 이 멤버들을 그대로 쓰면 안 됨을 확인함.
  대신 이미 이 클래스의 `GetClipRight()`/`GetClipBottom()` 스텁이 쓰고 있는
  것과 같은 방식으로 서피스 실제 폭/높이로 클리핑함)
- 클리핑된 사각형의 각 행에 대해 이미 구현되어 있던
  `Gamma4Pixel565()`(RGB 스케일링 기반, 원래 x86 어셈블리 최적화 버전을
  대체)를 호출
- `GammaBox555`는 이 SDL 백엔드가 항상 RGB565 서피스만 쓰는 것에 맞춰(이미
  `Gamma4Pixel555`가 `Gamma4Pixel565`의 얇은 별칭으로 구현되어 있던 것과
  동일한 패턴으로) `GammaBox565`를 그대로 호출하는 얇은 래퍼로 구현

## 커밋 메시지

```
fix: VS_UI/WinMain.cpp 죽은 진입점 제외 + GammaBox565/555 구현

VS_UI/WinMain.cpp와 Client/Client.cpp가 각각 WinMain/g_hWnd/g_GameRect 등을
중복 정의하고 있었음(컴파일 오류에 막혀 링크 단계까지 도달한 적이 없어
지금까지 드러나지 않았음). VC6 원본(client-master_vs6)의 실제 링크 구조와,
InitGame()/g_pTopView->Init()/g_pUpdate를 실제로 호출하는 쪽이
Client.cpp뿐이라는 점(SDLMain.cpp도 "Based on WinMain from Client.cpp"라고
명시)을 근거로, VS_UI/WinMain.cpp를 죽은 프로토타입으로 확정하고 WIN32
빌드에서도 제외함. SDL2main 충돌 관련 CMake 주석도 함께 정정.

이 제외로 work 4 error.log의 잔여 5건 중 ShowFPS/CSDLGraphics::Init/
InitBacksurface 3건은 호출부 자체가 사라져 해소됨. 실제로 살아있는
ClientFunction.cpp의 DrawAlphaBox()가 여전히 필요로 하는 GammaBox565/555는
VC6 원본(CDirectDrawSurface::GammaBox565/555)을 참고해 CSpriteSurface에
포팅(서피스 폭/높이로 클리핑 + 기존 Gamma4Pixel565 반복 호출).
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `CMakeLists.txt` | VS_UI 소스 필터에 `if(WIN32) list(FILTER ... EXCLUDE REGEX "VS_UI/WinMain\\.cpp") endif()` 추가(근거 주석 포함). SDL2main 충돌 주석 2곳을 `Client/Client.cpp` 기준으로 정정 |
| `Client/SpriteLib/CSpriteSurface.h` | `GammaBox565(RECT*, int)`, `GammaBox555(RECT*, int)` 선언 추가 |
| `Client/SpriteLib/CSpriteSurface_SDL.cpp` | `GammaBox565`/`GammaBox555` 구현 추가(VC6 원본 포팅) |

## 검증

- `cmake .`로 `build/vs2019` 재생성: `VS_UI/WinMain.cpp`가 소스 목록에서
  빠진 것을 확인
- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64`
  → **오류 0건으로 완전히 빌드됨**(라이브러리 링크까지 성공)
- `MSBuild build/vs2019/DarkEden.vcxproj /p:Configuration=Debug
  /p:Platform=x64`(최종 실행파일 링크) 검증 진행 중

## 참고 (범위 외 — 이번 작업에 포함하지 않음)

- `Client/DXLib/CDirectDrawSurface.h/.cpp`(`dxlib` 라이브러리 소속)도 별도로
  `GammaBox565`/`GammaBox555`/`InitBacksurface`/`ShowFPS`를 선언하고 있음.
  이건 `CSpriteSurface`와는 다른, 이번에 손대지 않은 별개의(현재 실사용
  경로에서는 참조되지 않는) 클래스라 그대로 둠
- `Client/CDirectDrawSurface.h/.cpp`(레거시, `Client/` 루트),
  `Client/WinLib/CWinMain.cpp` 등 미마이그레이션 구버전 파일들도 여전히
  트리에 남아있음 - 이번 작업 범위 밖
