# 커밋 로그

- 일시: 2026-08-19
- 대상: `Client/Packet/Assert.{h,cpp}` → `PacketAssert.{h,cpp}` 리네이밍 및 122개 파일의 include 갱신
- 유형: refactor (구조적 위험 요소 제거 - 표준 헤더 이름 충돌, `PacketFileAPI.h`와 동일 패턴)

## 배경

`u_window.cpp`/`u_scrollbar.cpp`/`pi_core.cpp` 등 VS_UI 위젯 파일들에서
`assert(...)` 호출이 전부 `error C3861: 'assert': 식별자를 찾을 수 없습니다`로 실패하는
문제를 조사함. 각 파일은 `Client_PCH.h`를 통해 표준 `<cassert>`를 정상적으로
include하고 있었음에도 오류가 발생해, `cl.exe /P`로 전처리 결과를 직접 추적함.

원인: `Client/Packet/Assert.h`(예외 기반 커스텀 `Assert()` 매크로를 제공하는 프로젝트
자체 파일)이 표준 C++ 헤더 `<assert.h>`와 파일명이 겹침. `Client/Packet`이 VS_UI
타겟의 컴파일러 include 경로(`/I`)에 포함되어 있어서, `<cassert>`가 내부적으로 수행하는
`#include <assert.h>`(꺾쇠괄호, 시스템 검색)가 진짜 CRT 헤더보다 먼저 이 프로젝트
파일을 찾아버림. `Assert.h`는 대문자 `Assert` 매크로만 제공하고 소문자 `assert`는
전혀 건드리지 않지만, 애초에 진짜 `<assert.h>`가 처리될 기회를 잃어버려서 표준
`assert` 매크로 자체가 정의되지 않았던 것. `fileapi.h`(→`PacketFileAPI.h`, 커밋
`fee134f`)와 완전히 동일한 유형의 헤더 이름 충돌.

`Assert.h`를 include하는 파일이 122개로 매우 많았지만(대부분 `Client/Packet/` 내부
파일로 원래도 명시적 include 경로가 필요 없었음), 이름 충돌 자체를 없애는 것이 가장
안전하고 확실한 해법이라 판단해 리네이밍하고 전체 include를 스크립트로 일괄 치환함
(전부 동일한 문자열 치환, 로직 변경 없음).

## 커밋 메시지

```
refactor: Assert.h/.cpp를 PacketAssert.h/.cpp로 리네이밍 (표준 <cassert> 충돌 예방)

Client/Packet/Assert.h(.cpp)의 파일명이 표준 C++ 헤더 <assert.h>와
겹쳐서, Client/Packet이 include 경로에 있는 VS_UI 타겟에서
<cassert>의 내부 #include <assert.h>가 이 프로젝트 파일로 가로채여
표준 assert() 매크로가 아예 정의되지 않는 문제가 있었음
(PacketFileAPI.h와 동일 패턴). PacketAssert.h/.cpp로 리네이밍하고
이를 include하던 122개 파일의 include문을 스크립트로 일괄 갱신.
git mv로 이력 보존, 로직 변경 없음.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/Packet/Assert.h` → `PacketAssert.h` | `git mv`로 리네이밍, 상단 주석에 리네이밍 사유 추가 |
| `Client/Packet/Assert.cpp` → `PacketAssert.cpp` | `git mv`로 리네이밍, 상단 주석 갱신 |
| 그 외 122개 `.cpp`/`.h` 파일 | `#include "Assert.h"` → `#include "PacketAssert.h"` (스크립트 일괄 치환) |

## 검증

- `cmake .` (build/vs2019 재구성)으로 파일명 변경 반영
- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `assert` 관련 `error C3861` 다수 (여러 VS_UI 위젯 파일에 산재)
- 수정 후: `assert` 관련 오류 0건. **전체 오류 537 → 189건으로 대폭 감소** (지금까지
  단일 수정 중 가장 큰 파급 효과 - `<cassert>` 셰도잉이 VS_UI 전역에 폭넓게 영향을
  미치고 있었음이 확인됨)
