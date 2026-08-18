# 커밋 로그

- 일시: 2026-08-19
- 대상: `VS_UI/Client_PCH.h`
- 유형: fix (빌드 오류 수정, VS_UI - HMMIO/MMCKINFO 재정의 재발)

## 원인

이전 CDirectDraw 재정의 일괄 수정(`ca514cc`)에서 `VS_UI/Client_PCH.h`의 실제
`<MMSystem.h>`/`<Digitalv.h>`/`<DDraw.h>` include는 제거했지만, `#include <windows.h>`
자체에 `WIN32_LEAN_AND_MEAN`을 붙이는 걸 빠뜨렸음. `WIN32_LEAN_AND_MEAN` 없이
`<windows.h>`를 include하면 내부적으로 `lzexpand.h` → `mmsystem.h` → `mmiscapi.h`
등 부가 헤더가 자동으로 딸려 들어와, 결국 `HMMIO`/`_MMCKINFO`가 다시 실제 정의로
채워지고 `basic/AudioTypes.h`의 SDL 스텁과 재정의 충돌이 발생함.

이 버그는 직전 커밋(`2716be2`, PLATFORM_WINDOWS 판별 순서 수정)까지는 드러나지
않았는데, 그 이유는 `PLATFORM_WINDOWS`가 이 지점에서 아직 정의되지 않아 `#else`
(POSIX) 분기로 빠져 `<unistd.h>`에서 먼저 fatal error로 죽어버렸기 때문임. 순서
버그를 고치자 비로소 이 `#include <windows.h>` 줄까지 도달하게 되면서 숨어있던
버그가 드러남.

## 커밋 메시지

```
fix: Client_PCH.h의 <windows.h> include에 WIN32_LEAN_AND_MEAN 누락 수정

지난 CDirectDraw 재정의 수정에서 실제 MMSystem.h/DDraw.h는 제거했지만
WIN32_LEAN_AND_MEAN을 빠뜨려서, <windows.h> 자체가 lzexpand.h ->
mmsystem.h를 자동으로 끌어들여 HMMIO/_MMCKINFO 재정의가 재발하던
문제 수정. PLATFORM_WINDOWS 판별 순서를 고치면서(2716be2) 비로소
이 지점까지 도달해 드러남.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/Client_PCH.h` | `#include <windows.h>` 앞에 `#define WIN32_LEAN_AND_MEAN` 추가, `_WINDOWS_` 가드로 감쌈 |

## 검증

- `cl.exe /P`로 `VS_UI_Base.cpp`를 재전처리하여 `mmsystem.h`가 여전히
  `VS_UI/client_PCH.h`의 `#include <windows.h>`를 통해 들어오는 것을 `#line` 마커로
  확인
- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `HMMIO`/`_MMCKINFO` 관련 오류 144건 포함 전체 2540건
- 수정 후: `HMMIO`/`MMCKINFO` 관련 오류 0건. 전체 오류 2540 → 319건으로 대폭 감소
