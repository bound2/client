# 커밋 로그

- 일시: 2026-08-19 (작업은 2026-08-18에 시작, 자정 넘어 마무리)
- 대상: `CMakeLists.txt`, `VS_UI/src/header/VS_UI_WebBrowser.h`, `VS_UI/src/VS_UI_WebBrowser.cpp`
- 유형: fix (빌드 오류 수정, VS_UI - ATLBASE.H 누락)

## 원인 (2갈래)

1. **ATL include 경로 누락 + 잘못된 경로 자동 추론**: `VS_UI_WebBrowser.h`(레거시 인게임
   IE 브라우저 임베딩 기능)가 `ATLBASE.H`(`CComBSTR`, `CComVariant` 등 ATL 스마트 타입)를
   include하는데, ATL 헤더는 MSVC 툴셋의 `atlmfc/include`에 있고 기본 include 경로에
   없음. 처음에 `CMAKE_CXX_COMPILER` 경로에서 툴셋 루트를 유도했는데, 이 값이 실제로는
   **삭제된 VS2019 BuildTools를 가리키는 오래된 CMake 캐시** 값이어서(`CMakeCache.txt`의
   `CMAKE_GENERATOR_INSTANCE`가 존재하지 않는 경로였음) 존재하지 않는 `atlmfc/include`
   경로가 생성됐음. 실제 설치된 VS(Visual Studio 18 Community)의 `atlmfc/include`를
   파일시스템에서 직접 탐색하도록 변경.
2. **ATLBASE.H/EXDISP.H include 순서가 반대**: `VS_UI_WebBrowser.h`가 `EXDISP.H`(IE COM
   자동화 인터페이스 선언)를 `ATLBASE.H`보다 먼저 include하고 있었음. `EXDISP.H`의
   `interface X : public IDispatch` 형태의 COM 인터페이스 선언은 `ATLBASE.H`가 미리
   준비해주는 COM/OLE 기반 타입에 의존하므로, 순서가 바뀌면서 `EXDISP.H` 파싱이 통째로
   깨져 수백 건의 `C2146`/`C4430`/`C2371`이 발생했음. 순서를 뒤집어 해결.

추가로, 이 두 가지를 고치고 나니 `VS_UI_WebBrowser.cpp(61)`의 사전 존재 64비트 이식성
버그(`get_HWND`에 `LONG*` 캐스트 - 이 SDK에서는 `SHANDLE_PTR*` 필요)가 드러나 함께 수정.

## 커밋 메시지

```
fix: ATLBASE.H 누락 및 EXDISP.H/ATLBASE.H include 순서 수정

VS_UI CMake 타겟에 ATL(atlmfc/include) 경로를 추가하되,
CMAKE_CXX_COMPILER(삭제된 VS2019 BuildTools를 가리키던 오래된 캐시
값)에 의존하지 않고 설치된 VS 인스턴스를 파일시스템에서 직접 탐색해
경로를 찾도록 함.

VS_UI_WebBrowser.h가 EXDISP.H를 ATLBASE.H보다 먼저 include하고 있어
COM 인터페이스 선언이 깨지던 문제도 순서를 바로잡아 수정.

VS_UI_WebBrowser.cpp의 get_HWND() 호출부에 남아있던 사전 존재 64비트
이식성 버그(LONG* -> SHANDLE_PTR*)도 함께 수정.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `CMakeLists.txt` | VS_UI 타겟에 ATL include 경로 추가. `file(GLOB ...)`로 설치된 VS의 `atlmfc/include`를 직접 탐색(컴파일러 캐시 경로에 의존하지 않음) |
| `VS_UI/src/header/VS_UI_WebBrowser.h` | `#include "EXDISP.H"` / `#include "ATLBASE.H"` 순서를 뒤집어 `ATLBASE.H`가 먼저 오도록 수정 |
| `VS_UI/src/VS_UI_WebBrowser.cpp` | `get_HWND((LONG*)&m_hWnd_Explorer)` → `get_HWND((SHANDLE_PTR*)&m_hWnd_Explorer)` |

## 검증

- `cmake .` (build/vs2019 재구성) → `VS_UI.vcxproj`의 `AdditionalIncludeDirectories`에
  `C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\
  atlmfc\include`(실존 확인) 포함
- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`를
  단계별로 반복 실행하며 검증
- 1차 수정 후(잘못된 ATL 경로): `ATLBASE.H` 오류는 일부 사라졌으나 `EXDISP.H`의
  `C2146`/`C4430`/`C2371`이 대량(전체 오류 359 → 728건) 발생
- 2차 수정(경로 탐색 방식 교체) 후: `ATLBASE.H` 자체 오류는 사라졌으나 `EXDISP.H` 파싱
  붕괴는 계속됨 (include 순서 문제였음이 드러남)
- 3차 수정(include 순서 반전) 후: `EXDISP.H` 관련 수백 건 오류 소멸, `VS_UI_WebBrowser.cpp`
  의 `get_HWND` 캐스트 오류 1건만 남음
- 최종 수정 후: `ATLBASE.H`/`WebBrowser`/`EXDISP` 관련 오류 0건. 전체 오류는 686건 —
  그동안 `EXDISP.H` 파싱 붕괴에 막혀 있던 다른 파일들이 더 진행되며 뒤에 가려진 별개의
  기존 오류가 다수 드러난 것으로, 지금까지의 작업 패턴과 동일함
