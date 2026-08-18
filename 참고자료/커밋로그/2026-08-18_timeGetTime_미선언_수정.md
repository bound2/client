# 커밋 로그

- 일시: 2026-08-18
- 대상: `basic/Platform.h`
- 유형: fix (빌드 오류 수정, VS_UI/Client - timeGetTime 미선언)

## 원인

`timeGetTime()`은 `<windows.h>`가 아니라 `<mmsystem.h>`(winmm)에 선언되는 함수인데,
지난 커밋(`ca514cc`)에서 `HMMIO`/`MMCKINFO` 재정의 문제를 해결하기 위해 실제
`<mmsystem.h>` include 경로를 모두 제거하면서 `timeGetTime`도 함께 사라졌음.

`Platform.h`에는 이미 `#define timeGetTime() platform_get_ticks()` 매크로가 있었지만,
`#ifndef PLATFORM_WINDOWS` 블록 안(비Windows 전용)에 있어서 Windows 빌드에는 적용되지
않고 있었음. `Client/MItem.h`, `VS_UI_GameCommon.h`가 `timeGetTime()`을 직접 호출하는
코드라 `error C3861: 'timeGetTime': 식별자를 찾을 수 없습니다`가 다수 발생.

`platform_get_ticks()`는 `basic/PlatformSDL.cpp`에서 `SDL_GetTicks()`로 구현되어
Windows를 포함한 모든 플랫폼에서 이미 정상 동작 확인됨.

## 커밋 메시지

```
fix: Windows에서 timeGetTime 미선언 오류 수정

timeGetTime()은 <mmsystem.h>(winmm) 전용 함수인데, HMMIO/MMCKINFO
재정의 문제 수정 과정에서 실제 <mmsystem.h> include를 모두 제거하며
함께 사라짐. mmsystem.h를 다시 끌어오는 대신, 이미 모든 플랫폼에서
동작하는 platform_get_ticks()(SDL_GetTicks() 기반)로 연결하는
timeGetTime() 매크로를 PLATFORM_WINDOWS에도 추가.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `basic/Platform.h` | `#ifdef PLATFORM_WINDOWS` 블록을 추가해 `timeGetTime() -> platform_get_ticks()` 매크로를 Windows에도 정의 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `MItem.h(2343,102)`, `VS_UI_GameCommon.h(3850,56)`의
  `error C3861: 'timeGetTime': 식별자를 찾을 수 없습니다` 79건
- 수정 후: `timeGetTime` 관련 오류 0건. 전체 오류 558 → 469건으로 감소
