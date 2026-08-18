# 커밋 로그 (실제 커밋은 수행하지 않음)

- 일시: 2026-08-18
- 대상: `basic/Platform.h`, `Client/SpriteLib/CFilter.h`
- 유형: fix (빌드 오류 수정, SpriteLib QWORD/CFilterPack)
- 실제 `git commit`은 수행하지 않았으며, 아래 로그는 참고용 기록입니다.

## 커밋 메시지 (초안)

```
fix: SpriteLib에서 QWORD 미선언 및 CFilterPack 재정의 오류 수정

- Platform.h: QWORD는 실제 Win32 API 타입이 아니라 프로젝트 자체
  커스텀 타입인데, 이전 수정에서 BYTE/DWORD/LONG 등 진짜 Windows
  타입과 함께 #ifndef PLATFORM_WINDOWS 블록에 갇혀 Windows 빌드에서
  완전히 사라짐. windows.h가 정의하지 않는 QWORD만 별도로 분리하여
  모든 플랫폼에서 항상 정의되도록 수정. (CShadowSprite.cpp의
  'QWORD': 선언되지 않은 식별자입니다. 오류 해결)

- CFilter.h: CFilterPack.h에 실제 구현된 CFilterPack 클래스
  (MTopView.h에서 m_LightFTP로 실사용 중)와, CFilter.h 안에 Windows
  전용으로 켜져 있던 죽은 typedef(`typedef CTypePack<CFilter>
  CFilterPack;`)가 이름 충돌을 일으켜 C2371(재정의) 및 연쇄적인
  m_pFilters/m_nFilters 미선언 오류가 발생. 사용처가 전혀 없는 죽은
  typedef를 제거하여 실제 구현체만 남김.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `basic/Platform.h` | `QWORD` typedef를 `#ifndef PLATFORM_WINDOWS` 블록 밖으로 분리, 모든 플랫폼에서 `QWORD_DEFINED` 가드로 항상 정의. UTF-8 BOM 적용 |
| `Client/SpriteLib/CFilter.h` | `#ifdef PLATFORM_WINDOWS` 하의 `typedef CTypePack<CFilter> CFilterPack;` 죽은 코드(2줄) 제거. UTF-8 BOM 적용 |

## 빌드 검증

- `MSBuild build/vs2019/Client/SpriteLib/SpriteLib.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `CShadowSprite.cpp(1587,2): error C2065: 'QWORD': 선언되지 않은 식별자입니다.` 외 연쇄 오류,
  `CFilterPack.h(17,7): error C2371: 'CFilterPack': 재정의. 기본 형식이 다릅니다.` 외 연쇄 오류(`m_pFilters`, `m_nFilters` 미선언 등)
- 수정 후: 위 오류 모두 해소, **`SpriteLib.lib` 빌드 성공 (exit code 0)**

## 참고 (범위 외 발견 사항 — 이번 수정에 포함하지 않음)

빌드는 성공했지만 링크 단계에서 다수의 `LNK4006`(중복 정의, 무시됨) 경고가 발생함:
`CSpriteSurface_SDL.obj`와 `CSpriteSurface_Adapter.obj`에 동일한 `CSpriteSurface::Blt*` 계열
멤버 함수가 중복 정의되어 있어 링커가 한쪽을 무시하고 있음. 빌드를 막는 오류는 아니라서
이번 작업(요구사항 4: 재탐색 금지) 범위에 포함하지 않았고, 다음 단계 작업 후보로 남겨둠.
