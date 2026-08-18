# 커밋 로그

- 일시: 2026-08-19
- 대상: `VS_UI/src/VS_UI_GameCommon.cpp`, `VS_UI/src/header/VS_UI_GameCommon.h`
- 유형: fix (빌드 오류 수정, VS_UI - 인코딩 오인식으로 인한 구문 오류)

## 원인

두 파일 모두 유효한 UTF-8이지만 BOM이 없는 상태로 저장되어 있었음(`C4819` 경고로
드러남). BOM이 없으면 MSVC는 소스를 현재 코드 페이지(949, 한글 완성형)로 해석하는데,
UTF-8로 인코딩된 한글 주석의 멀티바이트 시퀀스가 CP949 관점에서 우연히 다른 바이트로
쪼개져 해석되면서, 실제로는 존재하지 않는 구문 오류(미선언 식별자, 짝이 안 맞는
if/else, 예상치 못한 `#endif`)가 대량으로 발생했음.

`VS_UI_GameCommon.cpp(3597)`의 `p_old_item` 미선언 오류는 실제로는 바로 다음 줄에
`MItem* p_old_item = NULL;`로 정상 선언되어 있었음 - 코드 자체는 문제가 없었고, 순전히
인코딩 오인식으로 인한 허위 오류였음.

## 커밋 메시지

```
fix: VS_UI_GameCommon.cpp/.h에 UTF-8 BOM 추가 (CP949 오인식으로 인한 허위 구문 오류 수정)

BOM 없는 UTF-8 파일을 MSVC가 CP949로 오인식하면서, 한글 주석의
멀티바이트 시퀀스가 우연히 다른 바이트로 쪼개져 실재하지 않는
구문 오류(미선언 식별자, 잘못된 else, 예상치 못한 #endif)가
대량 발생하던 문제 수정. BOM을 추가해 UTF-8로 올바르게 인식되도록 함.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/src/VS_UI_GameCommon.cpp` | UTF-8 BOM 추가 (내용 변경 없음) |
| `VS_UI/src/header/VS_UI_GameCommon.h` | UTF-8 BOM 추가 (내용 변경 없음) |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `VS_UI_GameCommon.cpp(3597)`의 `error C2065`(미선언 식별자),
  `(4465)`의 `error C2181`(잘못된 else), `(6902)`의
  `fatal error C1020`(예기치 않은 #endif)
- 수정 후: 위 세 오류 모두 소멸. 대신 같은 파일의 `SetSurfaceInfo` 오버로드
  불일치(9969, 10071줄) 2건이 새로 드러남 - 인코딩 문제와 무관한 별개의 기존
  타입 불일치 버그로, 이번 수정 범위 밖
- 전체 오류 319 → 318건 (이 클러스터 자체는 대부분 허위 오류였어서 순감소는
  크지 않았지만, 실질적으로 파일 전체가 정상적으로 파싱되게 됨)
