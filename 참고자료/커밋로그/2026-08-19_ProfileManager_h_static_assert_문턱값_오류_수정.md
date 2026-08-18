# 커밋 로그

- 일시: 2026-08-19
- 대상: `Client/ProfileManager.h`
- 유형: fix (빌드 오류 수정, VS_UI - ProfileManager.h static_assert)

## 원인

`ProfileManager.h`는 이미 `#ifdef PLATFORM_WINDOWS #include <Windows.h>`로 올바르게
작성되어 있었고, include 순서 문제가 아니었음. 실제 원인은 120번째 줄의
`static_assert(sizeof(CRITICAL_SECTION) >= 68, ...)`의 **비교 문턱값(68) 자체가
잘못된 값**이었음.

직접 `sizeof(CRITICAL_SECTION)`을 컴파일해서 측정한 결과, 진짜 Windows `<windows.h>`의
`CRITICAL_SECTION` 구조체는 x64에서 **40바이트**(x86에서는 24바이트)이며, 68바이트가
되는 경우는 존재하지 않음. 즉 이 assert는 진짜 타입을 정상적으로 쓰고 있어도 항상
실패하도록 되어 있던 잘못된 검증 코드였음. (참고로 만약 타입이 진짜로 불완전했다면
`sizeof()` 자체가 컴파일 오류를 냈을 것이지, 작은 값을 조용히 반환하지 않음 - 애초에
이런 방식의 완전성 검사 자체가 의도한 목적을 달성하기 어려운 설계였음.)

## 커밋 메시지

```
fix: ProfileManager.h의 CRITICAL_SECTION 크기 static_assert 문턱값 오류 수정

sizeof(CRITICAL_SECTION) >= 68 어서션이 실제로는 항상 실패하도록
되어 있던 문제 수정. 실측 결과 진짜 Windows <windows.h>의
CRITICAL_SECTION은 x64에서 40바이트(x86 24바이트)이며 68바이트는
어떤 경우에도 나오지 않는 값. 문턱값을 24로 낮춰 x86/x64 모두의
실제 크기를 만족하도록 수정.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/ProfileManager.h` | `static_assert(sizeof(CRITICAL_SECTION) >= 68, ...)` → `>= 24`로 수정, 주석에 실측값 근거 추가 |

## 검증

- `cl.exe`로 `sizeof(CRITICAL_SECTION)` 직접 측정 → 40 (x64)
- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `ProfileManager.h(120,41): error C2338: CRITICAL_SECTION is incomplete -
  Platform.h must be included before ProfileManager.h` 27건
- 수정 후: `ProfileManager` 관련 오류 0건. 전체 오류 659 → 634건으로 감소
