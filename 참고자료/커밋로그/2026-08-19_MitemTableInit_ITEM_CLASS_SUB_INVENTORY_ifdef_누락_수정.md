# 커밋 로그

- 일시: 2026-08-19
- 대상: `VS_UI/MitemTableInit.cpp`
- 유형: fix (빌드 오류 수정, VS_UI - ITEM_CLASS_SUB_INVENTORY 미선언)

## 원인

`Client/ItemClassDef.h(136)`의 `ITEM_CLASS_SUB_INVENTORY`는 `//add by zdj` 주석과
함께 주석 처리되어 있고, 이 서브 인벤토리("2x4 교관/2x4 Pack") 기능은 코드베이스
전반에서 `#ifdef __TEST_SUB_INVENTORY__`로 감싸 조건부로만 컴파일되도록 일관되게
처리되어 있음(`Client/MItem.h`의 `MSubInventory` 클래스, `Client/MItem.cpp`의
`s_NewItemClassTable` 항목, `CGameUpdate.cpp`/`MSkillManager.cpp`/`PacketFunction.cpp`/
`UIMessageManager.cpp` 등 25개 파일 전부 동일 패턴). 그런데 `VS_UI/MitemTableInit.cpp`
(12843~12874줄)만 이 `#ifdef __TEST_SUB_INVENTORY__` 가드 없이 `InitClass(
ITEM_CLASS_SUB_INVENTORY, 1 )`를 무조건 호출하고 있어서, 주석 처리된 열거형 값을
그대로 참조해 `error C2065: 'ITEM_CLASS_SUB_INVENTORY': 선언되지 않은 식별자입니다`가
발생했음(같은 줄이 30건 가까이 반복).

`__TEST_SUB_INVENTORY__`는 현재 프로젝트 어디에도 `#define`되어 있지 않아(소스,
CMakeLists.txt, vcxproj 모두 확인) 이 기능은 현재 빌드에서 비활성 상태임 - 즉
열거형 값을 다시 활성화하는 대신, 나머지 코드와 동일하게 `#ifdef` 가드를 추가하는
것이 기존 로직과 일치하는 최소 수정임. 열거형 자체를 되살릴 경우 `MAX_ITEM_CLASS`
값이 밀리면서 `Client/MItem.cpp`의 `s_NewItemClassTable[MAX_ITEM_CLASS]` 배열
(팩토리 함수 테이블, 해당 기능 활성화 시에만 항목 추가됨)과 어긋나 널 함수 포인터
호출 위험이 생기므로 다루지 않음.

## 커밋 메시지

```
fix: MitemTableInit.cpp의 ITEM_CLASS_SUB_INVENTORY 초기화에 __TEST_SUB_INVENTORY__ ifdef 가드 추가

ITEM_CLASS_SUB_INVENTORY는 ItemClassDef.h에서 주석 처리되어 있고, 코드베이스
전반에서 #ifdef __TEST_SUB_INVENTORY__로 감싸 조건부로만 사용하는데
MitemTableInit.cpp의 InitClass(ITEM_CLASS_SUB_INVENTORY, 1) 블록만 이 가드
없이 무조건 컴파일되어 'ITEM_CLASS_SUB_INVENTORY': 선언되지 않은 식별자
오류가 발생하던 문제 수정. 나머지 25개 파일과 동일한 패턴으로 가드를 추가함.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/MitemTableInit.cpp` | `InitClass( ITEM_CLASS_SUB_INVENTORY,1 )` 및 뒤따르는 `m_pTypeInfo[ITEM_CLASS_SUB_INVENTORY]` 초기화 블록(12843~12874줄)을 `#ifdef __TEST_SUB_INVENTORY__` / `#endif`로 감쌈. 내용 변경 없음, 로직 삭제 없음 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `MitemTableInit.cpp`에서 `ITEM_CLASS_SUB_INVENTORY': 선언되지 않은
  식별자입니다` 관련 `error C2065` 30건
- 수정 후: `ITEM_CLASS_SUB_INVENTORY`/`MitemTableInit.cpp` 관련 오류 0건
- 전체 오류 360 → 330건으로 감소 (이번 건은 다른 오류를 가리고 있던 게 아니라
  그 자체가 30건짜리 독립 오류 덩어리였음)
- 남은 선두 오류: `SXml.cpp(416)` - `std::ofstream`에 `==` 비교 연산자 없음,
  `DSound.h(457)` - `_DSBPOSITIONNOTIFY` 재정의(DXLib 사운드 헤더 충돌 추정),
  `WinMain.cpp` - `gC_DD`/`g_pSDLInput` 관련 다수 오류 및 문자열 리터럴 미종료
