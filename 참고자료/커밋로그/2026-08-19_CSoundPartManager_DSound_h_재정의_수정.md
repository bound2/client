# 커밋 로그

- 일시: 2026-08-19
- 대상: `Client/CSoundPartManager.h`
- 유형: fix (빌드 오류 수정, VS_UI - DSound.h의 _DSBPOSITIONNOTIFY 재정의)

## 원인

`Client/CSoundPartManager.h`(10~11줄)는 `PLATFORM_WINDOWS`일 때 Windows SDK의
진짜 `<DSound.h>`를 include하고 있었음. 반면 `basic/AudioTypes.h`(91줄)는
DirectSound 의존성을 제거하기 위한 SDL 기반 대체 구조체로 `_DSBPOSITIONNOTIFY`를
플랫폼 구분 없이 정의하고 있음. `Client.h` -> `CSoundPartManager.h` 경로로 진짜
`<DSound.h>`가 include되면서, 같은 번역 단위에서 `basic/AudioTypes.h`의
`_DSBPOSITIONNOTIFY`와 충돌해 `DSound.h(457): error C2011: '_DSBPOSITIONNOTIFY':
'struct' 형식 재정의`가 발생했음.

`VS_UI/Client_PCH.h`에는 이미 "이 프로젝트는 더 이상 real DirectX/DirectSound를
사용하지 않으며, basic/AudioTypes.h와 Client/DXLib/CDirectDraw.h가 SDL 기반
대체 타입을 정의하므로 실제 헤더를 include하면 안 된다"는 취지의 주석과 함께
`<MMSystem.h>`/`<DDraw.h>`를 의도적으로 배제하는 조치가 되어 있었음(81f02ad).
`CSoundPartManager.h`의 `<DSound.h>` include는 이 마이그레이션 원칙을 따르지
않은 잔존 코드였고, 같은 파일에 이미 있던 `#else`(비-Windows) 분기가 정확히
필요한 대체 구현(`basic/Platform.h` include + `LPDIRECTSOUNDBUFFER` 전방 선언)
이었으므로 Windows에서도 동일한 경로를 쓰도록 통일함. `basic/Platform.h`가
`LPDIRECTSOUNDBUFFER`를 이미 `#ifndef` 가드로 전방 선언하고 있어 별도 선언은
불필요해 제거함.

## 커밋 메시지

```
fix: CSoundPartManager.h의 실제 <DSound.h> include 제거 (_DSBPOSITIONNOTIFY 재정의 수정)

Client.h -> CSoundPartManager.h 경로로 Windows SDK의 진짜 <DSound.h>가
include되면서 basic/AudioTypes.h가 정의하는 SDL 기반 _DSBPOSITIONNOTIFY와
충돌해 DSound.h(457): error C2011 재정의 오류가 발생하던 문제 수정.
Client_PCH.h에 이미 명시된 "실제 DirectX/DirectSound 헤더를 include하지
않는다"는 마이그레이션 원칙에 맞춰, 같은 파일의 비-Windows 분기에 있던
basic/Platform.h 기반 경로로 통일함.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/CSoundPartManager.h` | `#ifdef PLATFORM_WINDOWS #include <DSound.h> #else ... #endif` 분기를 제거하고, 항상 `basic/Platform.h`를 include하도록 통일(`LPDIRECTSOUNDBUFFER`는 `Platform.h`가 이미 전방 선언하므로 중복 선언 삭제) |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `DSound.h(457,1): error C2011: '_DSBPOSITIONNOTIFY': 'struct' 형식 재정의`
- 수정 후: `DSound.h`/`_DSBPOSITIONNOTIFY` 관련 오류 0건
- 전체 오류 329 → 318건으로 감소 (real DSound.h가 끌어들이던 연쇄 오류까지 함께
  해소된 것으로 보임)
- 남은 선두 오류: `WinMain.cpp` - `gC_DD`/`g_pSDLInput` 관련 다수 오류, 문자열
  리터럴 미종료(1601, 1609, 1687, 1695 등)
