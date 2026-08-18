# 커밋 로그

- 일시: 2026-08-18
- 대상: VS_UI 프로젝트 - `CDirectDraw`/`AudioTypes`(HMMIO, MMCKINFO) 재정의 및 관련 헤더 섀도잉 일괄 수정
- 유형: fix (빌드 오류 수정, VS_UI)

## 배경

`VS_UI.vcxproj` 빌드 시 `Client/DXLib/CDirectDraw.h`(LPDIRECTDRAW7, DDPIXELFORMAT, DDSCAPS2,
DDSURFACEDESC2, DDGAMMARAMP)와 `basic/AudioTypes.h`(HMMIO, MMCKINFO)가 재정의 오류를
일으켰음. 원인은 두 갈래였음:

1. **실제 DirectX/멀티미디어 헤더가 여전히 include되고 있었음.** 이 프로젝트는 SDL2로
   완전히 마이그레이션되어 `DDPIXELFORMAT`/`HMMIO` 등은 SDL 기반 stand-in 타입으로
   대체되어 있는데, 레거시 코드 3곳이 아직도 진짜 `<DDraw.h>`/`<MMSystem.h>`/`<Digitalv.h>`를
   include하고 있었음.
2. **`Client/` 루트에 남아있는, 마이그레이션되지 않은 옛 헤더들이 같은 이름의 파일명
   충돌(섀도잉)로 인해 마이그레이션된 `Client/DXLib/` 버전 대신 선택되고 있었음.**
   `DXLib.h`와 `CDirectDrawSurface.h`가 `Client/`(구버전)와 `Client/DXLib/`(SDL 이식판)에
   동시에 존재하는데, 여러 파일이 상대/비한정 경로(`#include "DXLib.h"`)로 include하고
   있어서, include 검색 순서 또는 같은 디렉터리 우선 규칙에 의해 구버전이 선택됨.
3. 추가로, `Client/Packet/types/SystemTypes.h`가 `WIN32_LEAN_AND_MEAN` 없이 `<Windows.h>`를
   가장 먼저 include하고 있어서, 이 번역 단위에서 `<Windows.h>`가 (LEAN_AND_MEAN 없이)
   `mmsystem.h`/`mmiscapi.h`/`lzexpand.h` 등 부가 헤더까지 통째로 끌어들이고 있었음.
   (이후 `Platform.h`가 `WIN32_LEAN_AND_MEAN`을 설정해도 windows.h 자체 include guard 때문에
   이미 늦은 상태였음.)

`cl.exe /P`로 실제 전처리 결과를 직접 추적하여(오류 메시지의 `mmiscapi.h(261): 'HMMIO' 선언을
참조하십시오` 힌트로 시작해 `#line` 마커를 역추적) 정확한 원인 파일들을 특정함.

## 커밋 메시지

```
fix: VS_UI CDirectDraw/AudioTypes 재정의 및 DXLib 헤더 섀도잉 일괄 수정

이 프로젝트는 SDL2로 마이그레이션되어 DDPIXELFORMAT/HMMIO 등은 SDL 기반
stand-in 타입(Client/DXLib/CDirectDraw.h, basic/AudioTypes.h)으로
대체되어 있는데, 다음 세 갈래 원인으로 진짜 Windows DirectX/멀티미디어
타입이 함께 섞여 들어와 재정의 오류(C2371/C2011)가 발생했음:

1. 레거시 코드가 여전히 실제 <DDraw.h>/<MMSystem.h>/<Digitalv.h>를
   include:
   - VS_UI/Client_PCH.h
   - VS_UI/src/hangul/Fl2.h (실제 ddraw.h 대신 자체 CDirectDraw.h 스텁
     사용하도록 변경)
   - basic/2d.h (사용되지 않는 include라 완전히 제거)

2. Client/ 루트에 남은 미마이그레이션 DXLib.h / CDirectDrawSurface.h가
   같은 이름의 Client/DXLib/ 이식판을 가리는 헤더 섀도잉. 비한정
   include를 "DXLib/DXLib.h", "DXLib/CDirectDrawSurface.h"로 명시:
   - VS_UI/src/header/VS_UI_Base.h
   - VS_UI/src/hangul/Ci.cpp
   - VS_UI/WinMain.cpp
   - Client/UtilityFunction.cpp
   - Client/MEventManager.h

3. Client/Packet/types/SystemTypes.h가 WIN32_LEAN_AND_MEAN 없이
   <Windows.h>를 가장 먼저 include하여, 이 번역 단위 전체에 fat
   windows.h(mmsystem.h 등 포함)가 퍼짐. WIN32_LEAN_AND_MEAN을 설정한
   뒤 include하도록 수정.

VS_UI.vcxproj 빌드 오류 707건 → 490건으로 감소, CDirectDraw/AudioTypes/
HMMIO/MMCKINFO 계열 재정의·미선언 오류는 전부 해소됨.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/Client_PCH.h` | Windows 분기에서 `<MMSystem.h>`/`<Digitalv.h>`/`<DDraw.h>` 제거 (`<windows.h>`/`<io.h>`/`<fcntl.h>`만 유지) |
| `VS_UI/src/hangul/Fl2.h` | `#include <ddraw.h>` → `#include "CDirectDraw.h"` (프로젝트 자체 SDL 스텁 사용) |
| `basic/2d.h` | 사용되지 않는 `#include <DDraw.h>` 완전 제거 |
| `VS_UI/src/header/VS_UI_Base.h` | `#include "DXLib.h"` → `#include "DXLib/DXLib.h"` (명시적 경로) |
| `VS_UI/src/hangul/Ci.cpp` | `#include "DXLib.h"` → `#include "DXLib/DXLib.h"` |
| `VS_UI/WinMain.cpp` | `#include "DXLib.h"` → `#include "DXLib/DXLib.h"` |
| `Client/UtilityFunction.cpp` | `#include "DXLib.h"` → `#include "DXLib/DXLib.h"` |
| `Client/MEventManager.h` | `#include "CDirectDrawSurface.h"` → `#include "DXLib/CDirectDrawSurface.h"` |
| `Client/Packet/types/SystemTypes.h` | `<Windows.h>` include 앞에 `WIN32_LEAN_AND_MEAN` 정의 + `_WINDOWS_` 가드 추가 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`를
  수정마다 반복 실행하며 단계적으로 검증 (7회 반복 리빌드로 원인을 하나씩 제거)
- `cl.exe /P`로 `VS_UI/src/VS_UI_Base.cpp`를 VS_UI.vcxproj의 실제 include
  경로/매크로 그대로 전처리하여, 오류 메시지가 가리키는 `mmiscapi.h`가 실제로 어느
  `#include <Windows.h>` 경로를 통해 들어오는지 `#line` 마커로 역추적함
- 최종 결과: 전체 오류 707건 → 490건. `CDirectDraw`/`AudioTypes`/`HMMIO`/`MMCKINFO`/`m_ddsd`
  관련 재정의·미선언 오류는 0건 (`VS_UI_Base.cpp(343)`의 `g_SetFL2Surface` 시그니처 불일치
  1건만 남음 — 아래 참고)

## 참고 (범위 외 발견 사항 — 이번 수정에 포함하지 않음)

- `VS_UI_Base.cpp(343,36): error C2664: g_SetFL2Surface(LPDIRECTDRAWSURFACE7)` — 헤더
  재정의 문제가 아니라, `FL2.cpp`(한글 IME 후보창 렌더링)가 아직 진짜 DirectDraw Surface의
  `GetDC()`/`ReleaseDC()`를 호출하는 방식으로 SDL 마이그레이션이 안 되어 있는 별개의 기능
  격차임. 호출부는 이미 `CSpriteSurface*`를 넘기고 있어 `FL2.cpp` 쪽에서 `CSpriteSurface`에
  대응하는 GDI 텍스트 렌더링 경로를 새로 구현해야 함 — 실제 기능 구현 판단이 필요해 이번
  작업(재탐색 금지) 범위에 포함하지 않음.
- `Client/` 루트에는 `DXLib.h`/`CDirectDrawSurface.h` 외에도 `CDirectSound.h`,
  `CDirectSoundStream.h`, `CDirectMusic.h`, `CDirectInput.h` 등 미마이그레이션 구버전이
  여전히 남아있음(`Client/DXLib.h`가 계속 이들을 include). 이번엔 VS_UI가 실제로 걸려 넘어진
  경로만 고쳤고, 구버전 파일 자체를 삭제/정리하지는 않음. 다른 타겟(DarkEden 등)이 향후
  같은 섀도잉에 걸릴 수 있음.
- VS_UI 잔여 오류 490건 중 나머지(예: `MinTr.h` 구문 오류, `CImm.h` 누락,
  `VS_UI_Filepath.h` 인코딩 문제, `SXml.cpp`의 `assert`/`ofstream` 오류 등)는 이번 작업과
  무관한 별개 항목으로 다음 단계 후보임.
