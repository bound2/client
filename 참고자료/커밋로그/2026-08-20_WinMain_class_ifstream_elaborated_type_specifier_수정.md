# 커밋 로그

- 일시: 2026-08-20
- 대상: `VS_UI/WinMain.cpp`
- 유형: fix (빌드 오류 수정, VS_UI - `class ifstream` elaborated-type-specifier 오용)

## 원인

`WinMain.cpp`에 `class ifstream 변수명(...)` 형태의 지역 변수 선언이 14곳
있었음(3291, 3300, 3317, 3333, 3342, 3357~3366줄). VC6 시절에는 `ifstream`을
"타입이 아니라 값처럼" 다시 선언하는 것을 막기 위해 흔히 붙이던 관용구였는데,
`ifstream`은 실제로는 `class`가 아니라 `typedef basic_ifstream<char> ifstream;`
(템플릿 별칭)이라서, 표준 준수 파서(C++11/VS2019)에서 `class ifstream`은
기존 `std::ifstream`을 가리키는 게 아니라 **현재 함수(WinMain) 스코프에
새로운 미정의(전방 선언만 된) 클래스 `ifstream`을 선언**해버림. 그 결과
바로 다음 줄부터 `error C2079: 'xxx'은(는) 정의되지 않은 class
'WinMain::ifstream'을(를) 사용합니다`, `error C2440`, `error C2664`(인수를
`std::ifstream&`로 변환 불가) 등이 대량 발생했고, 이 파싱 오류가 파일
뒤쪽까지 전파되어 `FILE_INFO_*`/`SPK_*` 매크로 미선언 등 무관해 보이는
오류까지 대거 동반했음(이전 세션에서 "인코딩 문제의 연쇄 효과일 수 있다"고
추측했던 부분의 실제 원인이 이것으로 확인됨).

같은 함수 안에 `class` 없이 `ifstream rankFile(...)`처럼 정상적으로 쓴
코드도 이미 존재해(3243, 3255, 3260, 3265, 3270, 3275줄 등) `ifstream`이
이 스코프에서 `std::ifstream`으로 정상 해석됨을 확인함 - `class` 접두어만
제거하면 되는 것으로 판단.

## 커밋 메시지

```
fix: WinMain.cpp의 'class ifstream' elaborated-type-specifier 오용 수정

VC6 시절 관용구인 'class ifstream 변수명(...)'이 표준 준수 파서에서는
std::ifstream을 가리키지 않고 현재 함수(WinMain) 스코프에 새로운 미정의
클래스 ifstream을 선언해버려, 뒤이은 코드가 'WinMain::ifstream'을
사용한다는 대량의 오류(C2079/C2440/C2664)와 그로 인한 파싱 오류 전파로
FILE_INFO_*/SPK_* 미선언 등 무관해 보이는 오류까지 동반 발생하던 문제
수정. 같은 함수에 이미 'class' 없이 쓰인 ifstream 선언들과 동일하게
class 접두어를 제거함(14곳).
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/WinMain.cpp` | 3291, 3300, 3317, 3333, 3342, 3357~3366줄의 `class ifstream 변수명(...)`에서 `class ` 접두어 제거 (14곳). 로직/인수 변경 없음 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `WinMain::ifstream` 관련 `C2079`/`C2440`/`C2664` 다수 +
  파싱 전파로 인한 `FILE_INFO_*`/`SPK_*` 계열 오류 동반, `WinMain.cpp` 단독으로
  `fatal error C1003`(오류 100개 초과)에 걸릴 정도였음
- 수정 후: `WinMain::ifstream` 관련 오류 0건. `WinMain.cpp`가 C1003 없이 끝까지
  파싱되어 남은 오류가 34건으로 명확히 드러남(더 이상 100개 컷오프에 가려지지 않음)
- 전체 오류 256 → 208건으로 감소 (48건)
- `FILE_INFO_*`/`SPK_*` 매크로 미선언 오류들은 이번 수정 후에도 그대로 남아있어
  - `class ifstream` 문제와는 별개의, 진짜 매크로/헤더 누락 문제로 확인됨
    (다음 후보)

## 참고 (범위 외 발견 사항)

- `WinMain.cpp(3491)`: `error C2039: 'Init': 'CSDLGraphics'의 멤버가 아닙니다`
  - 이전 세션에서 우려했던 대로, `gC_DD.Init(hwnd, ...)` 호출에 대응하는
    `CSDLGraphics::Init()` 메서드가 실제로 존재하지 않음. 창 생성 등 실질
    기능 구현이 필요한 별개 사안으로 이번 작업 범위 밖.
