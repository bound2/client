# 커밋 로그

- 일시: 2026-08-19
- 대상: `Client/MSkillManager.h`
- 유형: fix (빌드 오류 수정, VS_UI/Client - MSkillManager.h 구문 오류)

## 원인

451번째 줄 `typedef std::vector<ACTIONINFO> SKILL_STEP_LIST;`에서 `std::vector`를
사용하는데, 이 파일은 `<fstream>`/`<list>`/`<map>`만 include하고 있고 `<vector>`를
include하지 않고 있었음. 컴파일러가 `vector`를 템플릿으로 인식하지 못해
`std::vector<ACTIONINFO>` 전체가 파싱에 실패했고(`구문 오류: ';'이(가) '<' 앞에
없습니다`), 이 typedef가 깨지면서 이를 사용하는 `SKILL_STEP_MAP`(454줄),
`GetSkillStepList()`(528줄), `m_mapSkillStep`(587줄)까지 연쇄적으로 구문 오류가
발생했음.

## 커밋 메시지

```
fix: MSkillManager.h에 누락된 <vector> include 추가

SKILL_STEP_LIST가 std::vector<ACTIONINFO>를 쓰는데 <vector>가
include되어 있지 않아 파싱에 실패하고, 이를 사용하는
SKILL_STEP_MAP/GetSkillStepList()/m_mapSkillStep까지 연쇄적으로
구문 오류가 발생하던 문제 수정.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/MSkillManager.h` | `#include <vector>` 추가 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `MSkillManager.h(451,454,528,587)` 등에서 `C2143`/`C4430`/`C2065`/`C2059`/`C2238`/
  `C3646`/`C2062` 등 20여 건
- 수정 후: `MSkillManager` 관련 오류 0건. 전체 오류 634 → 545건으로 감소
