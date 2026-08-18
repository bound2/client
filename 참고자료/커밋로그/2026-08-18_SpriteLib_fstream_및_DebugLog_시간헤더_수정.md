# 커밋 로그 (실제 커밋은 수행하지 않음)

- 일시: 2026-08-18
- 대상: `Client/SpriteLib/CTypePack.h`, `Client/DebugLog.cpp`
- 유형: fix (빌드 오류 수정)
- 실제 `git commit`은 수행하지 않았으며, 아래 로그는 참고용 기록입니다.

## 커밋 메시지 (초안)

```
fix: SpriteLib에서 fstream.h 및 PLATFORM_WINDOWS 판별 순서 오류 수정

- CTypePack.h: Windows 분기에서 구식 헤더 <fstream.h>를 include하여
  VS2019에서 C1083(파일을 열 수 없음)로 빌드가 중단되던 문제 수정.
  표준 <fstream>으로 교체하고, 언클래스 std:: 사용(ios::binary 등)을
  유지하기 위해 using namespace std; 를 Windows 분기에도 추가.

- DebugLog.cpp: PLATFORM_WINDOWS 매크로가 정의되기 전(Platform.h
  미포함 상태)에 #ifdef PLATFORM_WINDOWS를 검사하여, VS2019/Windows
  빌드에서도 항상 else 분기(POSIX 전용 <sys/time.h>)로 진입해
  C1083이 발생하던 문제 수정. 최상단에서 Platform.h를 먼저 include하여
  PLATFORM_WINDOWS가 검사 시점에 올바르게 정의되도록 순서 조정.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/SpriteLib/CTypePack.h` | `#include <fstream.h>` → `#include <fstream>` + `using namespace std;` (Windows 분기), UTF-8 BOM 적용 |
| `Client/DebugLog.cpp` | `#include "../../basic/Platform.h"`를 최상단으로 이동하여 `PLATFORM_WINDOWS` 판별 순서 수정, UTF-8 BOM 적용 |

## 빌드 검증

- `MSBuild build/vs2019/Client/SpriteLib/SpriteLib.vcxproj /p:Configuration=Debug /p:Platform=x64`
- 수정 전: `CTypePack.h(6,11): fatal error C1083: 'fstream.h': No such file or directory` (다수 .cpp에서 반복), `DebugLog.cpp(17,11): fatal error C1083: 'sys/time.h': No such file or directory`
- 수정 후: 위 두 fatal error는 더 이상 발생하지 않음.

## 참고 (범위 외 발견 사항 — 이번 수정에 포함하지 않음)

이번 수정으로 `CTypePack.h`가 정상적으로 컴파일되면서, 이전에는 fatal error에 가려져 있던
아래 파일들의 **별개 오류**가 새로 드러났습니다. 이번 작업 지시(요구사항 4: 재탐색 금지)
범위를 벗어나므로 수정하지 않았고, 다음 단계 작업 대상으로 남겨둡니다.

- `Client/SpriteLib/CShadowSprite.cpp` : `QWORD`, `qpDest` 미선언 식별자 오류 (1587줄 등)
- `Client/SpriteLib/CFilterPack.cpp` : `m_pFilters`, `m_nFilters` 미선언 식별자 오류, `CTypePack<CFilter>::LoadFromFile`/`SaveToFile` 시그니처 불일치
