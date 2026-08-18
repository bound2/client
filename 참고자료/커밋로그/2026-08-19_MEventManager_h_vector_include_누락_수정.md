# 커밋 로그

- 일시: 2026-08-19
- 대상: `Client/MEventManager.h`
- 유형: fix (빌드 오류 수정, VS_UI/Client - MEventManager.h 구문 오류)

## 원인

`MSkillManager.h`(직전 커밋 `8e47ab4`)와 동일한 패턴. 105번째 줄
`std::vector<int> m_StringsID;`에서 `std::vector`를 쓰는데, 이 파일은 `<map>`만
include하고 `<vector>`를 include하지 않고 있었음.

## 커밋 메시지

```
fix: MEventManager.h에 누락된 <vector> include 추가

m_StringsID가 std::vector<int>를 쓰는데 <vector>가 include되어
있지 않아 C2143/C4430/C2039/C2238이 발생하던 문제 수정.
MSkillManager.h(8e47ab4)와 동일한 패턴.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/MEventManager.h` | `#include <vector>` 추가 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `MEventManager.h(105)`의 `C2039`/`C2238`/`C4430`/`C2143` 8건
- 수정 후: `MEventManager` 관련 오류 0건. 전체 오류 545 → 537건으로 감소
