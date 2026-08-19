# 커밋 로그

- 일시: 2026-08-20
- 대상: `Client/MGuildMarkManager.h`
- 유형: fix (예방적 조치 - CP949 오인식 경고를 BOM 추가로 선제 해소)

## 배경

`WinMain.cpp`를 `cl.exe /P`로 직접 전처리하며 `__GAME_CLIENT__` 매크로
유입 경로를 추적하던 중, `Client/MGuildMarkManager.h(1)`에서
`warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에
들어 있습니다`가 발생하는 것을 확인함. 이 파일은 BOM 없는 UTF-8이면서
한글 주석(`길드ID`, `길드마크Sprite` 등)을 다수 포함하고 있어,
`VS_UI_GameCommon.cpp`/`WinMain.cpp` 등에서 이미 두 차례 겪었던 것과
동일한 유형 - 아직 실제 구문 오류(error)로 번지지는 않았지만, 이 파일이
포함되는 번역 단위나 컴파일러 설정이 바뀌면 언제든 허위 구문 오류로
번질 수 있는 후보였음. 현재 시점에 VS_UI 빌드 오류를 유발하고 있지는
않지만, 이미 확립된 예방 조치(BOM 추가)를 선제적으로 적용함.

## 커밋 메시지

```
fix: MGuildMarkManager.h에 UTF-8 BOM 추가 (CP949 오인식 경고 예방)

BOM 없는 UTF-8 한글 주석 파일이 MSVC에 의해 CP949로 오인식될 수 있는
warning C4819가 발생하던 문제를 VS_UI_GameCommon.cpp와 동일하게 BOM
추가로 선제 해소. cl.exe /P로 WinMain.cpp를 전처리하던 중 발견함.
아직 실제 구문 오류로 번지지는 않았으나 동일 유형의 잠재 위험 파일이라
미리 정리함.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/MGuildMarkManager.h` | UTF-8 BOM 추가 (내용 변경 없음) |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전/후 모두 이 파일 자체가 유발하는 오류는 0건(예방적 조치이므로
  오류 수 변화 없음, 179건 유지)
- `warning C4819`는 더 이상 이 파일에서 발생하지 않음
