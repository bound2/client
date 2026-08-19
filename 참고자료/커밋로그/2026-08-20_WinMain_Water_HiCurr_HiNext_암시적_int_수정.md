# 커밋 로그

- 일시: 2026-08-20
- 대상: `VS_UI/WinMain.cpp`
- 유형: fix (빌드 오류 수정, VS_UI - 옛 K&R 스타일 암시적 int 선언)

## 원인

`Water()` 함수(1314줄)의 `static HiCurr, HiNext;`가 타입 지정자 없이
선언되어 있었음. C에서는 이런 경우 암시적으로 `int`로 간주됐지만
(K&R 관용구), C++은 기본 int(default-int)를 지원하지 않아
`error C4430: 형식 지정자가 없습니다. int로 가정합니다`가 발생했음.
바로 위아래 줄의 다른 지역 변수들(`cx,cy,cnt`, `addr1,addr2`,
`xhdif, yhdif, Chdif` 등)이 전부 `static int`로 선언되어 있고,
`HiCurr`/`HiNext`도 `Height[0][1]`(int 배열)에서 대입받아 정수로만
쓰이므로 동일하게 `int`로 명시함.

## 커밋 메시지

```
fix: WinMain.cpp의 Water() 함수 HiCurr/HiNext 암시적 int 선언 수정

'static HiCurr, HiNext;'가 타입 지정자 없이 선언되어 C++에서 error
C4430(형식 지정자 없음, int로 가정)이 발생하던 문제 수정. 같은 함수의
다른 지역 변수들과 동일하게 static int로 명시함.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/WinMain.cpp` | `Water()` 함수(1314줄)의 `static HiCurr, HiNext;`를 `static int HiCurr, HiNext;`로 수정 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `WinMain.cpp(1314,18)`/`(1314,26): error C4430`
- 수정 후: 해당 오류 0건
