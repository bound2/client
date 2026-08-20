# 커밋 로그

- 일시: 2026-08-21
- 대상: `Client/UtilityFunction.h`, `Client/UtilityFunction.cpp`
- 유형: fix (빌드 오류 수정, Client - `LoadImageToSurface`의 SDL 오버로드가
  `NOT WIN32`일 때만 선언/컴파일되던 문제)

## 원인

앞선 CMake/Ci.cpp/FL2.cpp 수정에서 `VS_UI_ExtraDialog.cpp`의 `m_temp_face`/
`bmpSurface`를 `CSpriteSurface`로 통일한 뒤 재빌드하자, 새로운 오류
`error C2664: 'bool LoadImageToSurface(const char *,CDirectDrawSurface &)':
인수 2을(를) 'CSpriteSurface'에서 'CDirectDrawSurface &'(으)로 변환할 수
없습니다`가 드러남.

`Client/UtilityFunction.h`를 확인한 결과, `LoadImageToSurface`의
`CSpriteSurface&` 오버로드 선언이 `#ifndef PLATFORM_WINDOWS`로 감싸져 있었고
(전방 선언 `class CSpriteSurface;`도 마찬가지), `Client/UtilityFunction.cpp`의
실제 구현도 `#ifndef PLATFORM_WINDOWS`로 감싸져 있었음. 이 프로젝트는
`USE_SDL_BACKEND`가 `PLATFORM_WINDOWS`를 포함해 모든 플랫폼에서 강제 `ON`이라
(`CMakeLists.txt:54`), Windows 빌드에서도 `CSpriteSurface&` 오버로드가
필요한데 선언·구현 모두 빠져 있어 `CDirectDrawSurface&` 오버로드 하나만
남아있던 상태였음 — 이번 세션의 다른 여러 항목과 동일한 "`NOT WIN32`
전용으로 남아있던 SDL 대체 구현" 패턴.

## 커밋 메시지

```
fix: LoadImageToSurface(CSpriteSurface&) 오버로드가 NOT WIN32일 때만
선언/컴파일되던 문제 수정

USE_SDL_BACKEND가 Windows를 포함한 모든 플랫폼에서 강제 ON인데,
LoadImageToSurface의 CSpriteSurface& 오버로드(선언·구현·전방선언 모두)가
#ifndef PLATFORM_WINDOWS로 감싸져 있어 Windows 빌드에서는
CDirectDrawSurface& 오버로드만 남아있던 문제 수정. 가드를 제거해
두 오버로드 모두 항상 선언/컴파일되도록 함.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/UtilityFunction.h` | `class CSpriteSurface;` 전방 선언과 `LoadImageToSurface(const char*, CSpriteSurface&)` 선언을 감싸던 `#ifndef PLATFORM_WINDOWS`/`#endif` 제거 |
| `Client/UtilityFunction.cpp` | `LoadImageToSurface(const char*, CSpriteSurface&)` 구현부를 감싸던 `#ifndef PLATFORM_WINDOWS`/`#endif` 제거 (`#include "SpriteLib/CSpriteSurface.h"`도 함께 무조건 include로 변경) |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64`
- `VS_UI_ExtraDialog.cpp(2726)`의 `LoadImageToSurface` 관련 `C2664` 오류
  0건으로 해소됨을 확인
