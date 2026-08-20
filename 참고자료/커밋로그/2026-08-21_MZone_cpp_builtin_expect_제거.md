# 커밋 로그

- 일시: 2026-08-21
- 대상: `Client/MZone.cpp`
- 유형: fix (빌드 오류 수정, Client - GCC/Clang 전용 내장 함수를 MSVC에서 사용)

## 원인

`work 4 error.log`의 `MZone.cpp(3957): error C3861: '__builtin_expect':
식별자를 찾을 수 없습니다`는 `__builtin_expect`가 GCC/Clang 전용 컴파일러
내장 함수(분기 예측 힌트)이고 MSVC에는 대응 함수가 없어서 발생함. 코드
주석("We use __builtin_expect to hint that validation usually succeeds")에서
알 수 있듯 순수 최적화 힌트일 뿐 분기 조건의 의미(`pNewEffect != NULL`)에는
아무 영향이 없어, 힌트 없이 조건식만 남기면 로직 변경 없이 크로스플랫폼
호환됨. 이 프로젝트 전체에서 `__builtin_expect` 사용처는 이 한 곳뿐이라
별도의 크로스플랫폼 매크로를 새로 만들 필요는 없다고 판단함.

## 커밋 메시지

```
fix: MZone.cpp의 GCC/Clang 전용 __builtin_expect를 MSVC 호환되도록 제거

__builtin_expect(pNewEffect != NULL, 1)는 순수 분기 예측 힌트로 조건의
의미에 영향이 없는데, MSVC에는 대응 함수가 없어 C3861이 발생하던 문제
수정. 조건식만 남겨 로직 변경 없이 MSVC에서도 컴파일되도록 함. 프로젝트
전체에서 이 한 곳에만 쓰이고 있어 별도 매크로는 추가하지 않음.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/MZone.cpp` | `if (__builtin_expect(pNewEffect != NULL, 1))` → `if (pNewEffect != NULL)` |

## 검증

- `MSBuild build/vs2019/DarkEden.vcxproj /p:Configuration=Debug /p:Platform=x64`
- `MZone.cpp(3957)`의 `C3861` 오류 0건으로 해소됨을 확인
