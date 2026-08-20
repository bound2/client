# 커밋 로그

- 일시: 2026-08-20
- 대상: `Client/MTopView.h`
- 유형: fix (빌드 오류 수정, Client - SDL 마이그레이션 때 삭제된 클래스를 여전히
  멤버로 선언 중이던 문제)

## 원인

`work 4 error.log`에서 `MTopView.h(738)`의 `error C3646`/`C4430`이 이 헤더를
include하는 5개 이상의 번역 단위(TU)에서 동일하게 반복 발생함.

`git log --diff-filter=D`로 추적한 결과 `Client/D3DLib/CD3DTextureEffect.h`/`.cpp`는
`a6e21d2`(Windows+DirectX → SDL 전환) 및 `bb70900`(죽은 코드 정리) 커밋에서
이미 삭제되었는데, `MTopView.h(737~739)`의 `#ifdef PLATFORM_WINDOWS` 멤버 선언
`CD3DTextureEffect m_TextureEffect;`만 지워지지 않고 남아있었음. `PLATFORM_WINDOWS`는
Windows 빌드에서 항상 참이라 이 멤버 선언이 항상 컴파일되는데, 정작
`CD3DTextureEffect` 타입 자체가 존재하지 않아 파서가 이를 override 지정자로
오인식(`C3646`)하고 암시적 `int`로 가정(`C4430`)함.

이 멤버의 실제 사용처(`.DrawEffect2D()` 호출)를 전부 확인한 결과:
- `DrawCreatureShadow.cpp(413)`: `/* ... */` 블록 주석(382~426줄) 안에 있어
  애초에 컴파일되지 않는 죽은 코드
- `MTopView.cpp(16055, 16144)`: 둘 다 어디에도 `#define`되지 않는
  `#ifdef __3D_IMAGE_OBJECT__` 안에 있어 항상 `#else` 분기만 컴파일됨
- `MTopViewDraw.inl`의 `DRAW_TEXTURE_SPRITE`/`DRAW_TEXTURE_SURFACE` 매크로도
  `MTopView.cpp`의 실제 호출부 2곳이 전부 주석 처리되어 있어 매크로 자체가
  전개되지 않음

즉 이 멤버는 어디서도 실제로 컴파일되어 쓰이지 않는 완전한 죽은 코드였음
(위쪽에 이미 주석 처리되어 있던 `m_EffectTPK`/`m_CreatureTPK`/`m_ShadowTPK`와
같은 패턴).

## 커밋 메시지

```
fix: MTopView.h의 삭제된 CD3DTextureEffect 멤버 선언 제거

SDL 마이그레이션(a6e21d2, bb70900)에서 Client/D3DLib/CD3DTextureEffect.h/.cpp
파일 자체는 삭제되었는데, MTopView.h의 #ifdef PLATFORM_WINDOWS 멤버 선언만
남아있어 이 헤더를 include하는 모든 TU에서 C3646/C4430이 반복 발생하던 문제
수정. 유일한 사용처(DrawCreatureShadow.cpp의 블록 주석, MTopView.cpp의
미정의 __3D_IMAGE_OBJECT__ 분기, MTopViewDraw.inl의 호출 안 되는 매크로)를
모두 확인해 실질적으로 죽은 코드였음을 검증한 뒤, 바로 위 m_EffectTPK 등과
같은 방식으로 주석 처리함.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/MTopView.h` | `#ifdef PLATFORM_WINDOWS` / `CD3DTextureEffect m_TextureEffect;` / `#endif`를 주석 처리(사유 설명 포함) |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64`
- `MTopView.h(738)` 관련 `C3646`/`C4430` 오류(5회 반복 발생분 포함) 전부 0건으로
  해소됨을 확인
