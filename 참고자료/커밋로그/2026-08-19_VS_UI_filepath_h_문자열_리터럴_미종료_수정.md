# 커밋 로그

- 일시: 2026-08-19
- 대상: `VS_UI/src/header/VS_UI_filepath.h`
- 유형: fix (빌드 오류 수정, VS_UI - VS_UI_filepath.h 구문 오류)

## 원인

364번째 줄에 문자열 리터럴을 닫는 따옴표 없이 끝나는 오타가 있었음:

```cpp
#define DIR_PROFILE							_PROFILE_ROOT"
```

`_PROFILE_ROOT`(20/32줄에서 `"UserSet\\"`/`"UserSet/"`로 정의됨) 뒤에 여는 따옴표(`"`)만
남고 실제 문자열 내용과 닫는 따옴표가 없어, 매크로 전체가 다음 줄들까지 하나의 미종료
문자열로 이어지며 `error C2001: 상수에 줄 바꿈 문자가 있습니다`가 발생했음.

바로 위 305번째 줄에 같은 `_PROFILE_ROOT`를 쓰는 `SPK_PROFILE` 매크로가 있는데, 이건
`_PROFILE_ROOT`만 그대로 쓰고 끝나는 정상 형태라 이를 기준으로 잘못 붙은 `"`만 제거함.
`VS_UI/WinMain.cpp:1560`에서 `strcpy(saveBmpName, DIR_PROFILE);`처럼 이 매크로를 순수
C 문자열로 직접 사용하는 코드가 있어, 정상적인 문자열 매크로여야 함이 확인됨.

## 커밋 메시지

```
fix: VS_UI_filepath.h의 DIR_PROFILE 매크로 미종료 문자열 리터럴 수정

DIR_PROFILE 매크로 정의에 _PROFILE_ROOT 뒤로 여는 따옴표만 남고 닫는
따옴표가 없어 C2001(상수에 줄 바꿈 문자)이 발생하던 문제 수정. 바로
위 SPK_PROFILE 매크로와 같은 형태(_PROFILE_ROOT 단독 사용)로 맞춤.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `VS_UI/src/header/VS_UI_filepath.h` | `DIR_PROFILE` 매크로 끝의 잘못 붙은 `"` 제거 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `VS_UI_filepath.h(364,1)`/`VS_UI_Filepath.h(364,1)`(대소문자 차이는 include문의
  표기 차이일 뿐 동일 파일)의 `error C2001` 22건
- 수정 후: `filepath` 관련 오류 0건. 전체 오류 686 → 659건으로 감소
