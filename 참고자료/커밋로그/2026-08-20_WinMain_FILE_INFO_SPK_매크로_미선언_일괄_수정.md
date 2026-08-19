# 커밋 로그

- 일시: 2026-08-20
- 대상: `VS_UI/src/header/VS_UI_filepath.h`, `Client/MGameStringTable.h`,
  `Client/MGameStringTable.cpp`
- 유형: fix (빌드 오류 수정, VS_UI - 빌드 타겟 매크로 정의 불일치로 인한
  FILE_INFO_*/SPK_*/InitGameStringTable 미선언)

## 배경

`WinMain.cpp`의 `FILE_INFO_FILEDEF`/`SPK_GAME_BACK` 등 다수의 `error C2065`가
`class ifstream` 수정(c45b824) 이후에도 남아있었음. 원인을 추적한 결과, 서로
연결된 세 가지 문제가 연쇄적으로 발견됨(하나를 고치면 다음 문제가 드러나는
구조라 한 번에 정리함).

### 1) `VS_UI_filepath.h`의 `#ifndef _LIB` 가드

`FILE_INFO_STR_EXP`~`SPK_LEVEL_MARK`(75~101줄)가 `#ifndef _LIB`로 감싸져
있었음. 루트 `CMakeLists.txt`가 `target_compile_definitions(VS_UI PRIVATE
_LIB)`로 VS_UI 타겟에 `_LIB`를 정의하는데(원래 VC6 라이브러리 빌드 설정을
맞추기 위함), `WinMain.cpp`는 이제 VS_UI 타겟 안에서 컴파일되면서도 이
매크로들을 실제로 필요로 하는 진짜 진입점 코드임. 어디에도 이 매크로들의
부재에 의존하는 코드가 없어(다른 값으로 재정의하는 곳도 없음) 가드를
제거함.

### 2) `MGameStringTable.h`/`.cpp`의 `#ifndef __GAME_CLIENT__` 가드

`_LIB` 가드를 걷어낸 뒤에도 `InitGameStringTable`이 `error C3861`로 남음.
`cl.exe /P`로 `WinMain.cpp`를 실제 프로젝트 include 경로/매크로 그대로
전처리해 추적한 결과, `__GAME_CLIENT__`가 CMake 타겟 정의
(`target_compile_definitions(DarkEden PRIVATE __GAME_CLIENT__=1)`)가 아니라
`Client/Client_PCH.h`(16줄, "Define this as a game client build")의 무조건
`#define __GAME_CLIENT__`를 통해 들어오고 있었음. `WinMain.cpp` ->
`VS_UI_Mouse_pointer.h` -> `MItem.h` -> `MObject.h`가 자신의 디렉터리
기준으로 `#include "Client_PCH.h"`를 하면서 `Client/Client_PCH.h`(VS_UI의
것이 아님)를 끌어들였고, 그 결과 VS_UI 타겟에서도 `__GAME_CLIENT__`가
항상 정의됨. 즉 `Client/` 아래 파일이 `Client_PCH.h`를 include하는 한
빌드 타겟과 무관하게 `__GAME_CLIENT__`는 항상 정의되는 구조라
`#ifndef __GAME_CLIENT__`로 감싸진 `InitGameStringTable()` 선언(.h)과
정의(.cpp)는 현재 아키텍처에서 사실상 영구적으로 죽은 코드였음(어느
타겟에서도 컴파일되지 않음). `WinMain.cpp`가 이 함수를 실제로 호출하고
있고 다른 어떤 코드도 이 함수의 부재에 의존하지 않아 가드를 제거함.

### 3) `MGameStringTable.h`의 `UI_STRING_MESSAGE_FRIEND` 열거형 값 누락

`__GAME_CLIENT__` 가드를 걷어내자 `InitGameStringTable()` 본문이 처음으로
실제 컴파일되면서, 그 안에서 참조하는 `UI_STRING_MESSAGE_FRIEND`가
`enum GAME_STRINGID`에서 `//add by viva : friend button description` 주석과
함께 통째로 주석 처리되어 있던 게 드러남(`ITEM_CLASS_SUB_INVENTORY`와
동일한 패턴). 같은 블록의 나머지 14개 값(`UI_STRING_MESSAGE_FRIEND_LIST`
등)은 전부 주석 처리된 코드에서만 참조되고 있어 그대로 두고,
실제로 쓰이는 `UI_STRING_MESSAGE_FRIEND` 하나만 주석 해제함.
`MAX_GAME_STRING`이 값 목록의 맨 끝에서 뒤이어 정의되므로 1만큼 밀리는데,
`GAME_STRINGID`는 `g_pGameStringTable`(로컬 UI 문자열 배열) 인덱스로만
쓰이고 네트워크 프로토콜 값이 아니며, `InitGameStringTable()`이 항상
`MAX_GAME_STRING + 1`개를 초기화하므로 안전함.

## 커밋 메시지

```
fix: WinMain.cpp의 FILE_INFO_*/SPK_*/InitGameStringTable 미선언 연쇄 수정

VS_UI_filepath.h의 #ifndef _LIB, MGameStringTable.h/.cpp의 #ifndef
__GAME_CLIENT__ 가드가 VS_UI 타겟에서 컴파일되는 WinMain.cpp가 실제로
필요로 하는 매크로/함수를 걷어내고 있던 문제 수정. __GAME_CLIENT__는
CMake 타겟 정의가 아니라 Client/Client_PCH.h의 무조건적인 #define을 통해
Client/ 아래 파일이 Client_PCH.h를 include할 때마다 항상 켜지는 구조라,
해당 가드로 감싼 InitGameStringTable()은 어느 타겟에서도 컴파일되지 않는
죽은 코드였음. 가드를 걷어낸 뒤 InitGameStringTable()이 실제로
컴파일되면서 드러난 UI_STRING_MESSAGE_FRIEND 열거형 누락(주석 처리)도
함께 해결.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/src/header/VS_UI_filepath.h` | `FILE_INFO_STR_EXP`~`SPK_LEVEL_MARK`(75~101줄)를 감싸던 `#ifndef _LIB`/`#endif` 제거 |
| `Client/MGameStringTable.h` | `extern void InitGameStringTable();`를 감싸던 `#ifndef __GAME_CLIENT__`/`#endif` 제거. `enum GAME_STRINGID`의 `UI_STRING_MESSAGE_FRIEND` 주석 해제 |
| `Client/MGameStringTable.cpp` | `InitGameStringTable()` 정의 전체를 감싸던 `#ifndef __GAME_CLIENT__`/`#endif` 제거 (내용 변경 없음) |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`를
  단계마다 반복 실행
- `cl.exe /P /C`로 `WinMain.cpp`를 실제 프로젝트 include 경로/매크로 그대로
  전처리하여, `__GAME_CLIENT__`가 어느 파일을 통해 들어오는지 `#line` 마커로
  역추적함(`Client/MItem.h` -> `Client/MObject.h` -> `Client/Client_PCH.h` 경로 확인)
- 단계별 결과:
  - `_LIB` 가드 제거: 전체 오류 208 → 181건
  - `__GAME_CLIENT__` 가드 제거: 181 → 181건 (InitGameStringTable 오류는
    사라졌지만 그동안 컴파일된 적 없던 함수 본문에서 `UI_STRING_MESSAGE_FRIEND`
    미선언 오류가 새로 1건 드러남 - 상쇄)
  - `UI_STRING_MESSAGE_FRIEND` 주석 해제: 181 → 180건
- 최종: `FILE_INFO_*`/`SPK_*`/`InitGameStringTable`/`MGameStringTable` 관련
  오류 0건
