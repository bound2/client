# 커밋 로그

- 일시: 2026-08-21
- 대상: `Client/Client.cpp`, `VS_UI/WinMain.cpp`, `VS_UI/src/VS_UI_Title.cpp`
- 유형: fix (빌드 오류 수정, Client/VS_UI - `WNDPROC` 호출 규약 타입 불일치,
  삭제된 `IDirectDraw` 스텁 호출)

## 원인

1. **`WNDPROC` 대입 오류**(`Client.cpp(1670)`, `WinMain.cpp(3424)`): 두 파일 모두
   `long FAR PASCAL WindowProc(HWND, UINT, WPARAM, LPARAM)`을 `wc.lpfnWndProc`
   (`WNDPROC` 타입)에 대입하는데, `Client.cpp`는 `(void*)`로 캐스트하고
   있었고(`void*`는 함수 포인터 타입에 대입 불가) `WinMain.cpp`는 캐스트가
   아예 없었음. x64 ABI에서는 `PASCAL`/`__stdcall`/`__cdecl`이 실질적으로
   동일한 호출 규약으로 정규화되지만, C++ 타입 시스템 상으로는 여전히 서로
   다른 함수 포인터 타입으로 취급되어 암시적 변환이 거부됨(`C2440`). 목표
   타입으로의 명시적 캐스트로 해결.
2. **삭제된 `IDirectDraw` 스텁 호출**(`WinMain.cpp(2350)`, `VS_UI_Title.cpp(5512)`):
   두 곳 모두 `CSDLGraphics::GetDD()->RestoreDisplayMode();`를 호출하는데,
   `CSDLGraphics::GetDD()`(`Client/DXLib/CDirectDraw.h:207`)는
   `static inline LPDIRECTDRAW7 GetDD() { return nullptr; }`로 완전히
   스텁화되어 있어 항상 `nullptr`을 반환함. `LPDIRECTDRAW7`은
   `struct IDirectDraw*`(전방 선언만 있고 정의가 없는 불완전 타입)라서
   `->RestoreDisplayMode()` 호출 자체가 컴파일되지 않음(`C2027`). 설령
   컴파일되더라도 `GetDD()`가 항상 `nullptr`을 반환하므로 실행 시
   널 포인터 역참조가 될 뿐인 죽은 코드였음.

## 커밋 메시지

```
fix: WNDPROC 호출 규약 캐스트 추가 및 삭제된 IDirectDraw 스텁 호출 제거

wc.lpfnWndProc 대입 시 x64에서는 실질적으로 동일한 호출 규약이지만 C++
타입 시스템상 서로 다른 함수 포인터 타입으로 취급되어 대입이 거부되던
문제를, 목표 타입(WNDPROC)으로의 명시적 캐스트로 수정(Client.cpp는 기존의
잘못된 (void*) 캐스트를 (WNDPROC)로 교체, WinMain.cpp는 캐스트 추가).
CSDLGraphics::GetDD()->RestoreDisplayMode() 호출은 GetDD()가 항상
nullptr을 반환하는 스텁이라 컴파일도 되지 않고 실행되어도 널 역참조일
뿐인 죽은 코드라 주석 처리함(WinMain.cpp, VS_UI_Title.cpp).
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/Client.cpp` | `wc.lpfnWndProc = (void*)WindowProc;` → `(WNDPROC)WindowProc;` |
| `VS_UI/WinMain.cpp` | `wcl.lpfnWndProc = WindowProc;` → `(WNDPROC)WindowProc;`. `CSDLGraphics::GetDD()->RestoreDisplayMode();` 주석 처리(사유 설명 포함) |
| `VS_UI/src/VS_UI_Title.cpp` | `CSDLGraphics::GetDD()->RestoreDisplayMode();` 주석 처리(사유 설명 포함) |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64`
- `WNDPROC` 관련 `C2440` 오류 2건, `IDirectDraw` 관련 `C2027` 오류 2건 전부
  0건으로 해소됨을 확인
