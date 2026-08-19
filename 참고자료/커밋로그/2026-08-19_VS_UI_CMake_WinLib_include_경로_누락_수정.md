# 커밋 로그

- 일시: 2026-08-19
- 대상: `CMakeLists.txt` (루트)
- 유형: fix (빌드 오류 수정, VS_UI - WinLib.h 누락)

## 원인

`Client/Client.h(34)`는 `PLATFORM_WINDOWS`일 때 `#include "WinLib.h"`로 비한정
참조하고 있고, 실제 파일은 `Client/WinLib/WinLib.h`에 존재함. 그런데 루트
`CMakeLists.txt`의 `VS_UI` 타겟 `target_include_directories`에는 `Client/DXLib`,
`Client/framelib` 등 다른 하위 디렉터리는 등록되어 있었지만 `Client/WinLib`만
빠져 있었음. `VS_UI` 타겟은 `Client/Client.cpp`를 직접 컴파일하므로(WinLib.cpp
자체는 컴파일하지 않지만 헤더는 필요), 컴파일러가 `WinLib.h`를 찾지 못해
`Client.h`를 include하는 시점에서 컴파일이 곧바로 중단됐음. `VS_UI/src/Imm`
누락 건(4c0435b)과 동일한 패턴.

## 커밋 메시지

```
fix: VS_UI CMake 타겟에 Client/WinLib include 경로 추가

Client.h(34)의 #include "WinLib.h"가 실제로는 Client/WinLib/WinLib.h인데,
루트 CMakeLists.txt의 VS_UI 타겟 target_include_directories에 해당
디렉터리가 빠져 있어 VS_UI.vcxproj 빌드 시 Client.cpp 컴파일이
'WinLib.h': No such file or directory fatal error로 즉시 중단되던 문제 수정.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `CMakeLists.txt` | VS_UI 타겟 `target_include_directories`에 `${CMAKE_CURRENT_SOURCE_DIR}/Client/WinLib` 추가 |

## 검증

- `cmake .` (build/vs2019 재구성) → `VS_UI.vcxproj`의 `AdditionalIncludeDirectories`에
  `Client\WinLib` 포함 확인
- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `Client.h(34,10): fatal error C1083: 'WinLib.h': No such file or directory`
  (Client.cpp 컴파일이 이 지점에서 즉시 중단)
- 수정 후: `WinLib.h` 관련 오류 0건. `Client.cpp`가 끝까지 파싱되면서 그 뒤에
  가려져 있던 별개의 기존 오류들이 새로 드러남(`CImm.h` 누락 건과 동일한 패턴):
  - `MitemTableInit.cpp` - `ITEM_CLASS_SUB_INVENTORY` 미선언 (다수)
  - `SXml.cpp(416)` - `std::ofstream`에 대한 `==` 연산자 없음
  - `DSound.h(457)` - `_DSBPOSITIONNOTIFY` 재정의 (DXLib/DSound.h 헤더 충돌 추정)
  - `WinMain.cpp` - `gC_DD`/`g_pSDLInput` 관련 다수 오류, 문자열 리터럴 미종료(1601, 1609, 1687, 1695 등)
  - `MParty.cpp` - `PARTY_VECTOR`/`m_pInfo` 관련 다수 오류 (STL iterator 관련 추정)
  - `RenderingFunctions.cpp` - `snprintf` 관련 오류, 문자열 리터럴 미종료(414, 419)
  - `MHelpMessageManager.cpp(487)` - 주석 안에서 예기치 않은 파일 끝
  - 전체 오류 306 → 360건으로 증가 - `WinLib.h` 누락에 막혀 컴파일이 중단되던
    파일이 끝까지 진행되면서 드러난 것으로, 지금까지의 작업 패턴과 동일함

## 참고

`build/`는 `.gitignore` 대상(cmake 생성물)이라 `VS_UI.vcxproj` 자체는 커밋
대상이 아님. `cmake .`을 다시 실행하면 이번에 고친 `CMakeLists.txt` 기준으로
자동 재생성됨.
