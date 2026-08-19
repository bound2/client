# 커밋 로그

- 일시: 2026-08-20
- 대상: `VS_UI/WinMain.cpp`, `Client/RenderingFunctions.cpp`,
  `Client/UtilityFunction.cpp`, `VS_UI/src/hangul/FL2.cpp`
- 유형: fix (빌드 오류 수정, VS_UI - CP949 오인식으로 인한 허위 구문 오류)

## 배경

`참고자료/인코딩 수정필요.md`에 기록해두었던 4개 파일. BOM 없는 UTF-8 파일을
MSVC가 CP949로 오인식하면서, 한글/한자 문자열 리터럴의 멀티바이트 시퀀스가
CP949 관점에서 우연히 다른 바이트로 쪼개져 실제로는 존재하지 않는 구문 오류
(문자열 리터럴 미종료, 미선언 식별자, 잘못된 리터럴 접두사 등)가 발생했음.
`VS_UI_GameCommon.cpp/.h`에 적용했던 것과 동일한 수정(56369e9)을 나머지
4개 파일에도 적용함.

## 커밋 메시지

```
fix: WinMain.cpp/RenderingFunctions.cpp/UtilityFunction.cpp/FL2.cpp에 UTF-8 BOM 추가

BOM 없는 UTF-8 파일을 MSVC가 CP949로 오인식하면서, 한글/한자 문자열
리터럴(서버 이름, "억"/"만" 단위 표시, 한글 자모 목록 등)의 멀티바이트
시퀀스가 우연히 다른 바이트로 쪼개져 실재하지 않는 구문 오류(문자열
리터럴 미종료, 미선언 식별자, 잘못된 리터럴 접두사)가 대량 발생하던 문제
수정. BOM을 추가해 UTF-8로 올바르게 인식되도록 함(VS_UI_GameCommon.cpp와
동일 패턴).
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/WinMain.cpp` | UTF-8 BOM 추가 (내용 변경 없음) |
| `Client/RenderingFunctions.cpp` | UTF-8 BOM 추가 (내용 변경 없음) |
| `Client/UtilityFunction.cpp` | UTF-8 BOM 추가 (내용 변경 없음) |
| `VS_UI/src/hangul/FL2.cpp` | UTF-8 BOM 추가 (내용 변경 없음) |
| `참고자료/인코딩 수정필요.md` | 위 4개 파일을 "수정 완료" 항목으로 갱신하고, `WinMain.cpp` 3201줄대 이후 오류가 인코딩 문제의 연쇄 효과라는 이전 추측을 정정(별개의 독립 버그로 확인) |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: 전체 오류 312건
- 수정 후: 전체 오류 257건 (55건 감소)
- 파일별 결과:
  - `UtilityFunction.cpp`: 관련 오류(C2001) 전부 해소, 이 파일에는 다른
    오류가 없어 완전히 정리됨
  - `RenderingFunctions.cpp`: 문자열 리터럴 관련 C2001 오류 해소. 62/67줄의
    `S_SURFACEINFO` 미선언 등은 인코딩과 무관한 별개 오류로 남음
  - `FL2.cpp`: 문자열 리터럴 관련 C2001 오류 해소. 230/342/359줄의
    `'c'` 미선언, `IDirectDrawSurface` 미정의는 별개 오류로 남음
  - `WinMain.cpp`: 1601~2679줄대의 C2001/C3680/C3688(문자열 리터럴 미종료,
    잘못된 리터럴 접두사) 오류 전부 해소. 단 3201줄대 이후의
    `FILE_INFO_*`/`SPK_*` 미선언, `ifstream` 오인식 오류들은 BOM 추가
    후에도 그대로 남아있어, 이전에 기록했던 "인코딩 오류의 연쇄 효과일
    가능성" 추측이 틀렸음을 확인함 - 별개의 독립적인 버그임
