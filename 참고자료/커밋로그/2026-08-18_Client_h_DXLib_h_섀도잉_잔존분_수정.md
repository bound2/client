# 커밋 로그

- 일시: 2026-08-18
- 대상: `Client/Client.h`
- 유형: fix (빌드 오류 수정, VS_UI - CDirectDrawSurface.h m_ddsd 잔존분)

## 원인

지난 `ca514cc` 커밋에서 `DXLib.h`/`CDirectDrawSurface.h` 헤더 섀도잉을 여러 곳(`VS_UI_Base.h`,
`Ci.cpp`, `WinMain.cpp`, `UtilityFunction.cpp`, `MEventManager.h`)에서 고쳤지만,
`Client/Client.h`의 `#include "DXLib.h"` 한 곳을 놓쳤음. `Client.h`는 `Client.cpp`,
`MItem.cpp`, `MZone.cpp` 등 여러 VS_UI 소스가 include하는 핵심 헤더라, 이 한 곳이
빠지면서 `Client/CDirectDrawSurface.h`(구버전, `m_ddsd` 멤버 없음)가 다시 선택되고
있었음.

## 커밋 메시지

```
fix: Client.h의 DXLib.h 섀도잉 잔존분 수정

지난 CDirectDraw 섀도잉 일괄 수정(ca514cc)에서 놓친 Client.h의
#include "DXLib.h"를 "DXLib/DXLib.h"로 명시. Client.h를 include하는
Client.cpp/MItem.cpp/MZone.cpp 등에서 재발하던
CDirectDrawSurface.h(구버전)의 m_ddsd 미선언 오류 해소.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/Client.h` | `#include "DXLib.h"` → `#include "DXLib/DXLib.h"` |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `CDirectDrawSurface.h(126/131/141)`의 `error C2065: 'm_ddsd'` 3개 블록(9건)
- 수정 후: `m_ddsd` 관련 오류 0건. 전체 오류 379 → 370건으로 감소

## 참고 (범위 외 발견 사항 — 이번 수정에 포함하지 않음)

`Client/MGuildMarkManager.cpp`에서 별개의 `CDirectDrawSurface` 관련 오류가 새로 드러남:

```
MGuildMarkManager.cpp(112): error C2079: 'bmpSurface'은(는) 정의되지 않은 class
'CDirectDrawSurface'을(를) 사용합니다.
```

이건 섀도잉/재정의 문제가 아니라, 이 파일이 `CDirectDrawSurface`의 완전한 정의를 제공하는
헤더(`DXLib/CDirectDrawSurface.h`)를 아예 include하지 않고 있어서(`UtilityFunction.h`의
전방 선언만으로는 지역 변수 생성이 불가능) 생기는 순수 "include 누락" 문제. `m_ddsd`
요청 범위와는 다른 별개 항목이라 이번엔 손대지 않음.
