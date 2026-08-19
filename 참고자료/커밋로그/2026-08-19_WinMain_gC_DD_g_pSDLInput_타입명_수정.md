# 커밋 로그

- 일시: 2026-08-19
- 대상: `VS_UI/WinMain.cpp`
- 유형: fix (빌드 오류 수정, VS_UI - 마이그레이션 후 남은 옛 타입명 참조)

## 원인

`WinMain.cpp`(95, 97줄)의 전역 변수 선언이 SDL2 마이그레이션 이전의 옛 클래스
이름을 그대로 쓰고 있었음:

- `CDirectDraw gC_DD;` - `Client/DXLib/CDirectDraw.h`의 실제 클래스는 마이그레이션
  과정에서 `CSDLGraphics`로 이름이 바뀌었고, `CDirectDraw`라는 이름의 클래스는
  프로젝트 어디에도 더 이상 존재하지 않음. 컴파일러가 `CDirectDraw`를 타입이
  아닌 암시적 int 변수 선언 시도로 오인해 `error C4430`/`C2146`이 발생했음.
- `extern CDirectInput* g_pSDLInput;` - 실제 클래스는 `CSDLInput`으로 이름이
  바뀌었고, `Client/CDirectInput.h`/`Client/InputService.h` 등에는 이미
  `extern CSDLInput* g_pSDLInput;`로 선언되어 있음. `WinMain.cpp`만 옛 이름으로
  다시 선언하면서 같은 이름의 전역 변수를 서로 다른 기본 타입으로 재선언하게
  되어 `error C2371: 재정의. 기본 형식이 다릅니다`가 발생했음.

이번 세션에서 이미 고친 `CSoundPartManager.h`/`MitemTableInit.cpp` 건과 마찬가지로,
클래스/식별자 이름이 마이그레이션되면서 일부 호출부만 갱신되지 않고 남은 사례임.

## 커밋 메시지

```
fix: WinMain.cpp의 gC_DD/g_pSDLInput 선언을 마이그레이션된 타입명으로 수정

CDirectDraw/CDirectInput 클래스는 SDL2 마이그레이션 과정에서 각각
CSDLGraphics/CSDLInput으로 이름이 바뀌었는데, WinMain.cpp의 전역 변수
선언(95, 97줄)만 옛 이름을 그대로 참조하고 있어 'CDirectDraw': 타입이
아님(C4430/C2146)과 'g_pSDLInput' 재정의(C2371) 오류가 발생하던 문제 수정.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/WinMain.cpp` | `CDirectDraw gC_DD;` → `CSDLGraphics gC_DD;`, `extern CDirectInput* g_pSDLInput;` → `extern CSDLInput* g_pSDLInput;` (95, 97줄). 주석의 `= new CDirectInput`도 `= new CSDLInput`으로 함께 수정 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `WinMain.cpp(95,21) C4430`, `(95,16) C2146`, `(97,20) C2143`,
  `(97,20/97,35) C4430`, `(97,24) C2371: 'g_pSDLInput' 재정의` 등 파일 최상단
  전역 선언부 오류
- 수정 후: 해당 오류 0건. 전체 오류 318 → 315건으로 감소
- 전체 오류 수 감소폭이 크지 않은 이유: `WinMain.cpp`는 파일 하나에서만
  `fatal error C1003`(오류 100개 초과로 컴파일 중단)에 걸릴 만큼 이번 건과
  무관한 별개 문제가 대량으로 남아 있음 - 인코딩 깨짐으로 인한 미선언
  식별자(`'댁갹'` 등, 2301/2656줄), 문자열 리터럴 줄바꿈(1601~2680줄대
  다수), `FILE_INFO_*`/`SPK_*` 매크로 미선언(3200~3340줄대), `ifstream`이
  `WinMain::ifstream`으로 잘못 해석됨(3291~3342줄대), `IDirectDraw` 미정의
  형식 사용(2347줄), 그리고 3284줄에 `CDirectInput`(옛 타입명) 참조가 하나
  더 남아있음. 이번 건(gC_DD/g_pSDLInput)이 뚫었던 파일 최상단 파싱 실패는
  해소됐지만, 나머지는 서로 무관한 별개 오류들이라 다음 작업으로 넘김.

## 참고 (범위 외 발견 사항 — 이번 수정에 포함하지 않음)

- `WinMain.cpp(3284)`: `CDirectInput`(옛 타입명) 참조가 한 곳 더 있음 - 이번
  건과 같은 카테고리라 다음 작업에서 함께 처리 가능
- `WinMain.cpp(2347)`: `IDirectDraw` 미정의 형식 사용 - `CDirectDraw`가
  `CSDLGraphics`로 대체되면서 관련 포인터 타입도 함께 정리가 필요해 보임
- 그 외 인코딩/문자열 리터럴/매크로 미선언 계열은 이번 작업과 무관한 독립
  이슈로 별도 조사가 필요함
