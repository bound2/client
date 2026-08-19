# 커밋 로그

- 일시: 2026-08-20
- 대상: `VS_UI/WinMain.cpp`
- 유형: fix (빌드 오류 수정, VS_UI - 마이그레이션 후 남은 옛 타입명 참조)

## 원인

`WinMain.cpp(3284)`의 `g_pSDLInput = new CDirectInput;`가 이전에 고친
95/97줄과 같은 카테고리의 잔존 참조였음. `CDirectInput` 클래스는 SDL2
마이그레이션 과정에서 `CSDLInput`으로 이름이 바뀌었고, 97줄의 전역 변수
선언은 이미 `CSDLInput*`로 수정했지만(WinMain_gC_DD_g_pSDLInput 커밋), 실제
초기화 지점인 3284줄은 옛 이름 `CDirectInput`으로 `new`하고 있어
`error C2061: 구문 오류: 식별자 'CDirectInput'`이 발생했음.

## 커밋 메시지

```
fix: WinMain.cpp(3284)의 남은 CDirectInput 참조를 CSDLInput으로 수정

g_pSDLInput = new CDirectInput; 가 95/97줄과 같은 카테고리의 잔존
참조였음. CDirectInput 클래스는 SDL2 마이그레이션 과정에서 CSDLInput으로
이름이 바뀌었는데 이 초기화 지점만 옛 이름을 그대로 쓰고 있어
'CDirectInput': 구문 오류(C2061)가 발생하던 문제 수정.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/WinMain.cpp` | `g_pSDLInput = new CDirectInput;` → `g_pSDLInput = new CSDLInput;` (3284줄) |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `WinMain.cpp(3284,20): error C2061: 구문 오류: 식별자 'CDirectInput'`
- 수정 후: `CDirectInput` 관련 오류 0건
- 전체 오류 257 → 256건으로 감소 (독립된 단일 오류였음)

## 참고

`WinMain.cpp(3291)`의 `class ifstream questinfo(...)` 구문(및 3300, 3317,
3333줄의 동일 패턴)이 `error C2079: 정의되지 않은 class 'WinMain::ifstream'`의
실제 원인으로 보임 - VC6 시절에 쓰이던 `class ifstream` elaborated-type-specifier가
표준 준수 모드에서는 `std::ifstream`을 가리키지 않고 현재 함수(WinMain)
스코프에 새로운 미정의 클래스 `ifstream`을 선언해버리는 것으로 추정됨.
인코딩 문제와 무관한 별개 버그로, 이번 작업(CDirectInput 정리) 범위에는
포함하지 않음 - 다음 후보로 남겨둠.
