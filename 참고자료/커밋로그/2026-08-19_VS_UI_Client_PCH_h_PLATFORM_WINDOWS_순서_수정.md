# 커밋 로그

- 일시: 2026-08-19
- 대상: `VS_UI/Client_PCH.h`
- 유형: fix (빌드 오류 수정, VS_UI - Client_PCH.h unistd.h 누락)

## 원인

`VS_UI/Client_PCH.h`가 파일 맨 위에서 `#ifdef PLATFORM_WINDOWS`를 검사하는데,
`PLATFORM_WINDOWS`는 `basic/Platform.h`가 처리되어야 정의되는 매크로임. 이 PCH가
번역 단위에서 가장 먼저 include되는 경우(다수의 VS_UI `.cpp` 파일에서 실제로 그러함),
이 시점엔 아직 매크로가 정의되지 않아 Windows 빌드에서도 `#else`(POSIX 전용) 분기로
빠져 `<unistd.h>`/`<SDL2/SDL.h>`를 include하려다 실패함. `Frame_PCH.h`, `DebugLog.cpp`,
`Client/Packet/types/SystemTypes.h`에서 이미 고쳤던 것과 동일한 패턴.

## 커밋 메시지

```
fix: VS_UI Client_PCH.h의 PLATFORM_WINDOWS 판별 순서 수정

PLATFORM_WINDOWS가 정의되기 전(Platform.h 미포함 상태)에
#ifdef PLATFORM_WINDOWS를 검사하여, 이 PCH가 번역 단위에서 가장
먼저 include되는 다수의 VS_UI .cpp 파일에서 Windows 빌드인데도
POSIX 전용 분기(<unistd.h> 등)로 빠지던 문제 수정. 파일 맨 위에서
_WIN32/_WIN64 기반으로 PLATFORM_WINDOWS를 미리 설정하도록 추가.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/Client_PCH.h` | 파일 최상단에 `#if defined(_WIN32) \|\| defined(_WIN64)` 기반 `PLATFORM_WINDOWS` 사전 정의 블록 추가 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `client_PCH.h(48,11)`/`Client_PCH.h(48,11)`(대소문자 차이는 include문 표기
  차이일 뿐 동일 파일)의 `fatal error C1083: 'unistd.h'` 53건
- 수정 후: `unistd.h` 관련 오류 0건

## 참고 (중요 — 예상보다 훨씬 큰 파급)

전체 오류 수가 189 → **2540건으로 대폭 증가**함. 이전까지의 수정들은 대체로 오류 수가
줄거나 소폭 늘어나는 패턴이었는데, 이번엔 `unistd.h` fatal error에 막혀 컴파일이 극히
초반에 중단되던 53개 번역 단위가 이제 훨씬 더 깊이까지 진행되면서, 그동안 전혀
컴파일된 적이 없어 드러나지 않았던 대규모의 사전 존재 오류 두 덩어리가 한꺼번에
노출됨:

1. `Client/DXLib/CDirectSoundStream.h`/`basic/AudioTypes.h`의 `HMMIO`/`_MMCKINFO`
   재정의 (144건) — 이전에 다른 파일들에서 고친 것과 같은 계열의 문제지만, 이 53개
   파일은 실제 `<windows.h>`를 (내가 이미 고친 `Client_PCH.h`/`SystemTypes.h`와는
   다른) 또 다른 경로로 먼저 include하고 있는 것으로 추정됨. 원인 경로를 아직
   특정하지 못함.
2. `VS_UI/src/header/VS_UI_GameCommon.h`의 3669~3796줄 부근에서 발생하는 대규모
   구문 오류 무더기(재정의, 잘못된 전역 소멸자, 짝 안 맞는 중괄호 등 추정) — 아직
   원인 미조사. 대형 헤더 파일 내부의 구조적 손상으로 보여 별도의 깊은 조사가 필요함.

이번 커밋 자체(`unistd.h` 수정)는 명백히 올바른 버그 수정이라 판단해 그대로 커밋하지만,
다음 단계로 위 두 덩어리 중 어느 것을 먼저 조사할지는 사용자 확인 후 진행 예정.
