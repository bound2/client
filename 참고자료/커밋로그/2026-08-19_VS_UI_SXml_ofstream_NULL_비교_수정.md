# 커밋 로그

- 일시: 2026-08-19
- 대상: `VS_UI/SXml.cpp`
- 유형: fix (빌드 오류 수정, VS_UI - std::ofstream과 NULL 비교)

## 원인

`XMLTree::Save(const char*)`(416줄)에서 파일 오픈 성공 여부를
`if ( file == NULL ) return;`로 검사하고 있었음. VC6 시절 `std::ofstream`은
`operator void*()` 암시적 변환을 제공해 `NULL`(=0)과 직접 비교할 수 있었지만,
C++11 표준 iostream은 이 변환 연산자를 제거하고 명시적 `operator bool()`만
제공하므로 `error C2678: 이항 '==': 왼쪽 피연산자로 'std::ofstream' 형식을
사용하는 연산자가 없거나 허용되는 변환이 없습니다`가 발생했음.

같은 파일의 원본격인 `Client/SXml/SXml.cpp`(DarkEden 실행 파일 타겟에서 사용,
`VS_UI/SXml.cpp`와 대부분 동일한 코드의 별도 사본)에는 이미 동일 지점이
`if ( !file.is_open() ) return;`로 수정되어 있어, 이번 수정은 그 기존 수정을
`VS_UI/SXml.cpp`에도 동일하게 반영한 것임.

## 커밋 메시지

```
fix: VS_UI/SXml.cpp의 std::ofstream NULL 비교를 is_open() 검사로 수정

XMLTree::Save()의 'if ( file == NULL )'는 C++11 iostream이 operator void*()
암시적 변환을 제거하면서 'std::ofstream'에 대한 == 연산자가 없어 컴파일
오류(C2678)가 발생하던 문제 수정. Client/SXml/SXml.cpp에 이미 적용되어 있던
'if ( !file.is_open() )' 검사로 동일하게 교체함.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/SXml.cpp` | `XMLTree::Save(const char*)`(416줄)의 `if ( file == NULL ) return;`를 `if ( !file.is_open() ) return;`로 교체 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `SXml.cpp(416,20): error C2678`
- 수정 후: `SXml.cpp` 관련 오류 0건
- 전체 오류 330 → 329건으로 감소 (다른 오류를 가리고 있던 게 아니라 그 자체가
  독립된 단일 오류였음)
- 남은 선두 오류: `DSound.h(457)` - `_DSBPOSITIONNOTIFY` 재정의(DXLib 사운드
  헤더 충돌 추정), `WinMain.cpp` - `gC_DD`/`g_pSDLInput` 관련 다수 오류 및
  문자열 리터럴 미종료

## 참고

`Client/SXml/SXml.cpp`와 `VS_UI/SXml.cpp`는 대부분 동일한 코드의 별도 사본이며,
`Client/SXml/SXml.cpp` 쪽이 주석 등이 더 온전하게 남아 있고 이번 수정 지점도
이미 반영되어 있었음. 두 사본 간 다른 차이점(예: `MultiByteToWideChar` 호출의
`_WIN32`/비-Windows 분기 유무 등)은 이번 오류와 무관해 손대지 않음.
