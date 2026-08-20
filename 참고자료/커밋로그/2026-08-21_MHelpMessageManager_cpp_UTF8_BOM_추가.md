# 커밋 로그

- 일시: 2026-08-21
- 대상: `Client/MHelpMessageManager.cpp`, `참고자료/인코딩 수정필요.md`
- 유형: fix (빌드 오류 수정, Client - CP949 오인식으로 인한 허위
  "주석 미종료" 오류)

## 원인

`work 4 error.log`의 `MHelpMessageManager.cpp(487,1): fatal error C1071:
주석에서 예기치 않은 파일의 끝이 나타났습니다`는 이 파일이 UTF-8(BOM 없음)로
저장되어 있어 MSVC가 현재 코드 페이지(949)로 오인식하면서 발생한 허위
오류였음. 파일 안의 실제 `/* */` 블록 주석은 287~294줄 한 쌍뿐이고
정상적으로 닫혀 있는데도, 한글 문자열 리터럴/주석(`"Sender"`, `"Level 조건표"`
등)의 멀티바이트 시퀀스가 CP949 관점에서 우연히 다른 바이트로 잘못 쪼개지며
어딘가에서 `/*`처럼 보이는 바이트열이 만들어져 파일 끝까지 주석으로
잘못 인식됨. `file` 명령으로 실제 인코딩이 BOM 없는 UTF-8임을 확인함.

같은 유형으로 이미 2026-08-19(`VS_UI_GameCommon.cpp/.h`)와 2026-08-20
(`WinMain.cpp`, `RenderingFunctions.cpp`, `UtilityFunction.cpp`, `FL2.cpp`,
`MGuildMarkManager.h`)에서 반복 확인된 패턴으로, 내용 변경 없이 BOM만
추가하면 해결됨.

## 커밋 메시지

```
fix: MHelpMessageManager.cpp에 UTF-8 BOM 추가

BOM 없는 UTF-8 파일을 MSVC가 CP949로 오인식하면서, 한글 문자열/주석의
멀티바이트 시퀀스가 우연히 다른 바이트로 쪼개져 실재하지 않는 주석
미종료(fatal error C1071)가 발생하던 문제 수정. BOM을 추가해 UTF-8로
올바르게 인식되도록 함(기존 VS_UI_GameCommon.cpp 등과 동일 패턴).
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/MHelpMessageManager.cpp` | UTF-8 BOM 추가 (내용 변경 없음) |
| `참고자료/인코딩 수정필요.md` | 이번에 새로 발견된 `MHelpMessageManager.cpp` 사례를 "수정 완료" 목록에 추가 |

## 검증

- `MSBuild build/vs2019/DarkEden.vcxproj /p:Configuration=Debug /p:Platform=x64`
- `MHelpMessageManager.cpp` 관련 `fatal error C1071` 0건으로 해소됨을 확인
