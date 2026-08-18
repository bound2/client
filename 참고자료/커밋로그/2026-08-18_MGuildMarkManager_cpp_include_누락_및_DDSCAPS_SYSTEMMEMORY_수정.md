# 커밋 로그

- 일시: 2026-08-18
- 대상: `Client/MGuildMarkManager.cpp`, `basic/Platform.h`
- 유형: fix (빌드 오류 수정, VS_UI - MGuildMarkManager.cpp)

## 원인

1. `Client/MGuildMarkManager.cpp`가 `CDirectDrawSurface`를 지역 변수로 직접 생성해서
   쓰는데(`CDirectDrawSurface bmpSurface;`), 완전한 클래스 정의를 제공하는 헤더를
   include하지 않고 있었음. `UtilityFunction.h`가 제공하는 `class CDirectDrawSurface;`
   전방 선언만으로는 참조/포인터 매개변수 선언은 가능해도 지역 객체 생성은 불가능해서
   `error C2079: 정의되지 않은 class`가 발생.
2. include를 추가하고 나니 `DDSCAPS_SYSTEMMEMORY` 상수가 미선언 상태로 남음.
   `basic/Platform.h`에 정의는 있었지만, `QWORD`/`timeGetTime`과 같은 패턴으로
   `#ifndef PLATFORM_WINDOWS` 전용 블록 안에 있어 Windows에서는 빠져 있었음. 이 상수는
   실제 Win32 API가 아니라 이 프로젝트 자체 DirectDraw 스텁 상수라 `<windows.h>`가
   정의해 준 적이 없음.

## 커밋 메시지

```
fix: MGuildMarkManager.cpp의 CDirectDrawSurface.h 누락 및 DDSCAPS_SYSTEMMEMORY 미선언 수정

MGuildMarkManager.cpp가 CDirectDrawSurface를 지역 변수로 생성하는데
완전한 정의를 제공하는 헤더를 include하지 않고 있어 C2079가 발생하던
문제 수정 - DXLib/CDirectDrawSurface.h를 명시적으로 include.

이어서 드러난 DDSCAPS_SYSTEMMEMORY 미선언은 QWORD/timeGetTime과 같은
패턴: 프로젝트 자체 DirectDraw 스텁 상수인데 #ifndef PLATFORM_WINDOWS
블록에 갇혀 있던 것을 모든 플랫폼에서 정의되도록 이동.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/MGuildMarkManager.cpp` | `#include "DXLib/CDirectDrawSurface.h"` 추가 |
| `basic/Platform.h` | `DDSCAPS_SYSTEMMEMORY` 정의를 `#ifndef PLATFORM_WINDOWS` 블록 밖으로 이동, 모든 플랫폼에서 정의 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `MGuildMarkManager.cpp(112,121,823,832)`의 `error C2079`(정의되지 않은 class),
  `error C2664`(인수 변환 불가) 등
- 중간: include 추가 후 `MGuildMarkManager.cpp(123,834)`의
  `error C2065: 'DDSCAPS_SYSTEMMEMORY': 선언되지 않은 식별자입니다` 발견
- 수정 후: `MGuildMarkManager` 관련 오류 0건. 전체 오류 370 → 359건으로 감소
