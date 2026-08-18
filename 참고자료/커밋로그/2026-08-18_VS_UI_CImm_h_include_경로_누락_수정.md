# 커밋 로그

- 일시: 2026-08-18
- 대상: `CMakeLists.txt` (루트)
- 유형: fix (빌드 오류 수정, VS_UI - CImm.h 누락)

## 원인

`VS_UI/src/Imm/CImm.h`는 실제로 존재하고, `VS_UI/src/header/VS_UI_widget.h`,
`VS_UI/src/Vs_ui.cpp`, `VS_UI/src/widget/u_window.cpp` 등에서 `#include "CImm.h"`로
비한정 참조하고 있음. 파일 자체는 문제 없었고, 루트 `CMakeLists.txt`의 `VS_UI` 타겟
`target_include_directories`에 `VS_UI/src/Imm` 디렉터리가 등록되어 있지 않아서
컴파일러가 이 헤더를 찾지 못했음(`DarkEden` 실행 파일 타겟의 include 목록에는 이미
포함되어 있었음 - VS_UI 타겟만 누락).

## 커밋 메시지

```
fix: VS_UI CMake 타겟에 VS_UI/src/Imm include 경로 추가

VS_UI/src/Imm/CImm.h는 실제로 존재하지만, 루트 CMakeLists.txt의 VS_UI
타겟 target_include_directories에 해당 디렉터리가 빠져 있어 VS_UI.vcxproj
빌드 시 'CImm.h: No such file or directory' fatal error가 발생하던 문제
수정. DarkEden 타겟에는 이미 등록되어 있던 항목을 VS_UI 타겟에도 추가.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `CMakeLists.txt` | VS_UI 타겟 `target_include_directories`에 `${CMAKE_CURRENT_SOURCE_DIR}/VS_UI/src/Imm` 추가 |

## 검증

- `cmake .` (build/vs2019 재구성) → `VS_UI.vcxproj`의 `AdditionalIncludeDirectories`에
  `VS_UI\src\Imm` 포함 확인
- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `VS_UI_Widget.h(17,10): fatal error C1083: 'CImm.h': No such file or directory`
  (30건)
- 수정 후: `CImm.h` 관련 오류 0건. 전체 오류는 439 → 558건으로 증가 — `CImm.h` 누락에
  막혀 컴파일이 중단되던 파일들이 더 진행되면서 그 뒤에 가려진 별개의 기존 오류가 드러난
  것으로, 지금까지의 작업 패턴과 동일함

## 참고

`build/`는 `.gitignore` 대상(cmake 생성물)이라 `VS_UI.vcxproj` 자체는 커밋 대상이 아님.
`cmake .`을 다시 실행하면 이번에 고친 `CMakeLists.txt` 기준으로 자동 재생성됨.
