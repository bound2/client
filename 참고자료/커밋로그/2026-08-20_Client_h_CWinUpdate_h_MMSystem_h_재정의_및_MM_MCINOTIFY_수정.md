# 커밋 로그

- 일시: 2026-08-20
- 대상: `Client/Client.h`, `Client/WinLib/CWinUpdate.h`, `basic/Platform.h`
- 유형: fix (빌드 오류 수정, Client - `HMMIO`/`MMCKINFO`/`platform_get_ticks` 재정의
  재발 및 `MM_MCINOTIFY` 미선언)

## 원인

`work 4 error.log`에서 `mmiscapi.h`/`timeapi.h`의 `HMMIO`/`_MMCKINFO`/
`platform_get_ticks` 재정의 오류(`Client.cpp`, `MItem.cpp`,
`VS_UI_GameCommon.h`, `CWinUpdate.h`)가 3회 반복 발생함. 2026-08-19에
`VS_UI/Client_PCH.h`에서 고쳤던 것과 정확히 같은 유형의 버그가 **Client
타깃 쪽에서 재발**한 것으로 확인됨. 두 갈래 원인이 있었음.

1. **`Client/Client.h`**: `#ifdef PLATFORM_WINDOWS` 분기에서
   `WIN32_LEAN_AND_MEAN` 없이 바로 `#include <Windows.h>`를 하고 있었음(반면
   `#else` 분기는 `basic/Platform.h`를 통해 올바르게 include). 이 파일이
   `basic/Platform.h`보다 먼저 처리되는 TU에서는, `<Windows.h>`가 lean 없이
   먼저 처리되며 `_WINDOWS_` 가드를 선점해버려서, 이후 `basic/Platform.h`의
   `#ifndef _WINDOWS_ / #define WIN32_LEAN_AND_MEAN / #include <windows.h>`가
   건너뛰어지고 `mmsystem.h`(`HMMIO`/`MMCKINFO`)가 통째로 딸려 들어옴.
2. **`Client/WinLib/CWinUpdate.h`**: `#ifdef PLATFORM_WINDOWS` 분기에서
   `<Windows.h>`에 이어 **`<MMSystem.h>`를 직접 include**하고 있었음. 이 파일이
   실제로 필요로 하는 건 `timeGetTime()`(→ `basic/Platform.h`가 이미
   `platform_get_ticks()`로 매크로 치환해줌)와 `DWORD` 뿐인데, 굳이 진짜
   `<MMSystem.h>`를 불러 `HMMIO`/`MMCKINFO`의 실제 정의 및
   `timeGetTime()`(매크로 치환으로 `platform_get_ticks`가 되어버린) 선언이
   `basic/AudioTypes.h`/`basic/Platform.h`의 SDL 대체 타입과 충돌함(연결
   규약이 다른 재정의: `error C2375`).

이 두 파일을 고치자, `Client.cpp(827)`의 `MM_MCINOTIFY`/`MCI_NOTIFY_SUCCESSFUL`
미선언 오류가 새로 드러남 — 이전에는 (`<MMSystem.h>`가 통째로 들어와 있었기
때문에) 우연히 정의되어 있었을 뿐, 별도로 관리되던 상수가 아니었음. AVI 인트로
재생 완료를 감지하는 `WindowProc`의 실제 메시지 처리 로직이라 값 자체는 필요해,
표준 Win32 SDK의 안정된 상수값을 `basic/Platform.h`에 직접 정의함(같은 파일에
이미 있는 `WM_TEXTINPUT` 등 메시지 상수 shim과 동일한 방식).

## 커밋 메시지

```
fix: Client.h/CWinUpdate.h의 <MMSystem.h> 계열 재정의 재발 및 MM_MCINOTIFY 미선언 수정

Client.h가 WIN32_LEAN_AND_MEAN 없이 <Windows.h>를 직접 include하고,
CWinUpdate.h는 한술 더 떠 <MMSystem.h>까지 직접 include하고 있어,
2026-08-19에 VS_UI/Client_PCH.h에서 고쳤던 HMMIO/MMCKINFO/
platform_get_ticks 재정의 버그가 Client 타깃 쪽에서 재발하던 문제 수정.
두 파일 모두 basic/Platform.h의 lean windows.h include 및
timeGetTime()->platform_get_ticks() 매크로 경로로 통일함. 이 수정으로
드러난 Client.cpp의 MM_MCINOTIFY/MCI_NOTIFY_SUCCESSFUL 미선언(AVI 인트로
재생 완료 감지용, 이전에는 <MMSystem.h> 통째 include로 우연히 정의됨)도
basic/Platform.h에 표준 Win32 SDK 상수값으로 직접 정의해 해결.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/Client.h` | `#ifdef PLATFORM_WINDOWS` 분기의 `<Windows.h>` include를 `#ifndef _WINDOWS_`/`WIN32_LEAN_AND_MEAN` 가드로 감쌈 |
| `Client/WinLib/CWinUpdate.h` | `#ifdef PLATFORM_WINDOWS`(`<Windows.h>`+`<MMSystem.h>`)/`#else`(`basic/Platform.h`) 분기를 제거하고 `basic/Platform.h` include로 통일 |
| `basic/Platform.h` | `MM_MCINOTIFY`(0x03B9), `MCI_NOTIFY_SUCCESSFUL`(0x0001) 상수를 `#ifndef` 가드로 추가(기존 `WM_TEXTINPUT` shim 바로 아래) |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64`
- `mmiscapi.h`/`timeapi.h`의 `HMMIO`/`_MMCKINFO`/`platform_get_ticks` 재정의
  오류(3회 반복분 포함) 전부 0건으로 해소됨을 확인
- `Client.cpp(827)`의 `MM_MCINOTIFY`/`MCI_NOTIFY_SUCCESSFUL` 미선언 오류도
  0건으로 해소됨을 확인
- 전체 오류: 52건 → 10건으로 감소(이 수정만으로 8건 해소, 나머지는 다른
  독립 수정에서 처리)
