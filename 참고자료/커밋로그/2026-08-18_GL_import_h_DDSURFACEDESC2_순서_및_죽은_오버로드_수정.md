# 커밋 로그

- 일시: 2026-08-18
- 대상: `basic/GL_import.h`, `VS_UI/src/header/VS_UI_Base.h`
- 유형: fix (빌드 오류 수정, VS_UI - GL_import.h 구문 오류)

## 원인

`basic/GL_import.h`가 `#ifdef PLATFORM_WINDOWS` 아래에서 `DDSURFACEDESC2`/`DDSURFACEDESC`
타입을 매개변수로 쓰는 `SetSurfaceInfo` 오버로드 2개를 선언하는데:

1. `DDSURFACEDESC2`는 `Client/DXLib/CDirectDraw.h`가 정의하는 SDL 스텁 타입인데,
   `VS_UI_Base.h`에서 `GL_import.h`가 `DXLib/DXLib.h`보다 먼저 include되고 있어서,
   이 시점엔 아직 `DDSURFACEDESC2`가 정의되지 않은 상태였음. 이전엔 (지난 커밋에서 제거한)
   실제 `<DDraw.h>`가 훨씬 앞단(`Client_PCH.h`)에서 먼저 include되면서 우연히 이 순서
   문제를 가려주고 있었음.
2. `DDSURFACEDESC`(2가 안 붙은 원본 DirectDraw 서술자)는 SDL 마이그레이션 과정에서
   대응하는 스텁 타입이 아예 만들어지지 않았고, 실제 호출부도 전부 `DDSURFACEDESC2`를
   반환하는 `GetDDSD()`만 사용하고 있어 이 오버로드는 애초에 컴파일될 수 없는 죽은
   선언이었음.

타입이 없는 상태에서 `const DDSURFACEDESC2 *p_ddsd` 같은 매개변수를 파싱하려니 컴파일러가
`DDSURFACEDESC2`를 암시적 `int`로 간주하고 그 뒤의 `*`를 곱셈 연산자로 오인해
`C4430`(형식 지정자 없음)과 `C2143`(`,` 앞에 구문 오류) 오류가 대량으로 발생했음.

## 커밋 메시지

```
fix: GL_import.h의 DDSURFACEDESC2 순서 문제 및 죽은 DDSURFACEDESC 오버로드 제거

VS_UI_Base.h에서 GL_import.h가 DXLib/DXLib.h보다 먼저 include되어,
GL_import.h가 선언하는 SetSurfaceInfo(S_SURFACEINFO*, DDSURFACEDESC2*)
오버로드 시점에 DDSURFACEDESC2 타입이 아직 정의되지 않아 C4430/C2143이
대량 발생하던 문제 수정. DXLib/DXLib.h include를 GL_import.h보다
앞으로 옮김.

DDSURFACEDESC(2가 없는 버전) 오버로드는 SDL 마이그레이션에서 대응 타입이
만들어진 적이 없고 실제 호출부도 없는 죽은 선언이라 제거.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/src/header/VS_UI_Base.h` | `#include "DXLib/DXLib.h"`를 `#include "GL_import.h"`보다 앞으로 이동 |
| `basic/GL_import.h` | 사용처 없는 `SetSurfaceInfo(S_SURFACEINFO*, const DDSURFACEDESC*)` 죽은 선언 제거 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `GL_import.h(28,81)`/`GL_import.h(29,80)`의 `error C4430`/`C2143` 다수(파급되어
  67건)
- 수정 후: `GL_import.h` 관련 오류 0건. 전체 오류 573건 → 439건으로 감소

## 참고 (범위 외 발견 사항 — 이번 수정에 포함하지 않음)

`GL_import.h`가 선언하는 함수들은 `DllImport`(Windows에서 `__declspec(dllimport)`로 확장)로
선언되어 있는데, 이를 실제로 구현하던 `basic/GL_import.cpp`는 `basic/CMakeLists.txt`에
등록되어 있지 않아 빌드되지 않음. 즉 이번 수정은 컴파일 단계 오류만 해소한 것이고,
실제로 `SetSurfaceInfo`/`FillRect` 등을 호출하는 코드가 링크 단계까지 가면 미해결 외부
심볼 오류가 날 수 있음. 이 legacy DLL 계층을 실제로 SDL 구현으로 이어줄지, 호출부를
정리할지는 더 큰 판단이 필요해 이번 작업(재탐색 금지) 범위에 포함하지 않음.
