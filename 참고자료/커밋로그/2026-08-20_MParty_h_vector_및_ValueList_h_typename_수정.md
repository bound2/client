# 커밋 로그

- 일시: 2026-08-20
- 대상: `Client/MParty.h`, `Client/Packet/ValueList.h`
- 유형: fix (빌드 오류 수정, Client - `<vector>` include 누락 및 템플릿 의존 타입
  `typename` 누락)

## 원인

`work 4 error.log`의 `MParty.h(54)`/`MParty.cpp` 전체(50건 이상)와 `ValueList.h(34,35)`
오류는 서로 다른 파일이지만 같은 유형(전형적인 VC6 관용구가 표준 준수 파서에서
깨지는 경우)이라 함께 정리함.

1. **`MParty.h`**: `typedef std::vector<PARTY_INFO*> PARTY_VECTOR;`를 쓰면서
   `<vector>`를 include하지 않고 있었음(`MString.h`/`MTypeDef.h` 어디에도 없음).
   VC6은 다른 헤더를 통해 `<vector>`가 우연히 먼저 들어와 있으면 통과되지만,
   이 프로젝트의 include 순서가 바뀌며 `std::vector`가 미선언 상태로 남음
   (`error C2039: 'vector': 'std'의 멤버가 아닙니다`). 이 하나의 오류가
   `PARTY_VECTOR` 타입 자체를 무너뜨려 `MParty.cpp`의 `m_pInfo`/`iInfo` 관련
   오류 50여 건을 연쇄적으로 유발했음(2026-08-18의 `CPositionList.h`/
   `CPartManager.h` `typename` 수정과 같은 성격의 "하나 고치면 전부 풀리는" 사례).
2. **`ValueList.h`**: `Begin()`/`End()`의 반환 타입 `std::list<T>::const_iterator`가
   템플릿 매개변수 `T`에 의존하는 타입인데 `typename` 키워드가 없었음. VC6은
   관대하게 해석했지만 표준 준수 파서는 `const_iterator`를 정적 멤버로 오인해
   구문 오류(`error C2061`/`C2334`)를 냄.

## 커밋 메시지

```
fix: MParty.h의 <vector> include 누락 및 ValueList.h의 typename 누락 수정

MParty.h가 std::vector<PARTY_INFO*>를 typedef하면서 <vector>를 include하지
않아 PARTY_VECTOR 타입 자체가 깨지고, 이로 인해 MParty.cpp의 m_pInfo/iInfo
관련 오류 50여 건이 연쇄 발생하던 문제 수정. ValueList.h의
Begin()/End()에서 템플릿 의존 타입 std::list<T>::const_iterator에
typename 키워드를 추가(2026-08-18 CPositionList.h/CPartManager.h 수정과
동일 패턴).
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/MParty.h` | `#include <vector>` 추가 |
| `Client/Packet/ValueList.h` | `Begin()`/`End()` 반환 타입에 `typename` 추가 |

## 검증

- `MSBuild build/vs2019/DarkEden.vcxproj /p:Configuration=Debug /p:Platform=x64`
- `MParty.h`/`MParty.cpp`/`ValueList.h` 관련 오류 전부 0건으로 해소됨을 확인
