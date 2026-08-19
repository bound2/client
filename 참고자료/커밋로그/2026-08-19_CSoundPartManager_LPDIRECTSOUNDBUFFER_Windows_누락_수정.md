# 커밋 로그

- 일시: 2026-08-19
- 대상: `Client/CSoundPartManager.h`
- 유형: fix (빌드 오류 수정, VS_UI - LPDIRECTSOUNDBUFFER 미선언, 앞선 수정의 후속 보정)

## 원인

바로 앞서 진행한 `_DSBPOSITIONNOTIFY` 재정의 수정(커밋 2d12c9e)에서
`CSoundPartManager.h`의 `#ifdef PLATFORM_WINDOWS #include <DSound.h> #else
... #endif` 분기를 제거하고 항상 `basic/Platform.h`만 include하도록
바꿨는데, `basic/Platform.h`(196~207줄)의 `LPDIRECTSOUNDBUFFER` 전방 선언은
`#ifndef PLATFORM_WINDOWS`로 감싸져 있어 **Windows에서는 정의되지 않음**
(원래 Windows에서는 진짜 `<DSound.h>`가 이 타입을 제공한다고 가정하고
있었음). 앞선 수정으로 그 진짜 `<DSound.h>` include를 없앴는데
`LPDIRECTSOUNDBUFFER`를 대체할 선언을 추가하지 않아서, Windows 빌드에서
`error C2065: 'LPDIRECTSOUNDBUFFER': 선언되지 않은 식별자입니다`와 뒤이은
템플릿 인수 오류(`CPartManager<WORD, BYTE, LPDIRECTSOUNDBUFFER>`)가
새로 발생했음.

`Client/DXLib/CDirectSound.h`(18~21줄)처럼 SDL로 이미 마이그레이션된
헤더들은 이 타입을 플랫폼 구분 없이 직접 선언하고 있음 - `CSoundPartManager.h`도
같은 방식으로, `basic/Platform.h`에 의존하지 않고 파일 자체에서
`LPDIRECTSOUNDBUFFER`를 (원래 비-Windows 분기에 있던 것과 동일하게) 직접
전방 선언하도록 보정함.

## 커밋 메시지

```
fix: CSoundPartManager.h에 LPDIRECTSOUNDBUFFER 전방 선언 복원 (Windows 누락)

앞선 수정(2d12c9e)에서 real <DSound.h> include를 제거하며 basic/Platform.h만
남겼는데, Platform.h의 LPDIRECTSOUNDBUFFER 전방 선언은 #ifndef
PLATFORM_WINDOWS로 감싸져 있어 Windows에서는 정의되지 않아 'error C2065:
LPDIRECTSOUNDBUFFER': 선언되지 않은 식별자 오류가 새로 발생하던 문제 수정.
CSoundPartManager.h가 (기존 비-Windows 분기처럼) 이 타입을 플랫폼 구분 없이
직접 전방 선언하도록 보정.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/CSoundPartManager.h` | `#include "../../basic/Platform.h"` 뒤에 `#ifndef LPDIRECTSOUNDBUFFER typedef struct IDirectSoundBuffer* LPDIRECTSOUNDBUFFER; #endif` 추가(원래 비-Windows 분기에 있던 선언을 플랫폼 구분 없이 복원) |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `CSoundPartManager.h(17,59): error C2065: 'LPDIRECTSOUNDBUFFER'`,
  `error C2923`, `error C2955`
- 수정 후: `CSoundPartManager.h`/`LPDIRECTSOUNDBUFFER`/`DSound`/
  `DSBPOSITIONNOTIFY` 관련 오류 0건
- 전체 오류 315 → 312건으로 감소
