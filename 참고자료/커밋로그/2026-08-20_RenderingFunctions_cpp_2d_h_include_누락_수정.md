# 커밋 로그

- 일시: 2026-08-20
- 대상: `Client/RenderingFunctions.cpp`
- 유형: fix (빌드 오류 수정, Client - `S_SURFACEINFO` 타입 include 누락)

## 원인

`work 4 error.log`의 `RenderingFunctions.cpp(62, 67)` 오류(`S_SURFACEINFO`/
`pSurfaceInfo`/`pRect` 미선언 → 뒤이은 구문 오류 연쇄)는 `S_SURFACEINFO` 타입이
정의된 `basic/2d.h`를 이 파일이 include하지 않아서 발생함.

`S_SURFACEINFO`는 `basic/2d.h`에서 정의되고(`basic/GL_import.h`가 이를 다시
`#include`), `Client/SpriteLib/CSpriteSurface.h`는 이미 `"For S_SURFACEINFO"`라는
주석과 함께 `basic/2d.h`를 include하고 있음. 반면 `RenderingFunctions.cpp`는
`basic/BasicException.h`/`basic/BasicData.h`만 include하고 있어 `Rect`(전역
`class Rect`, `BasicData.h`에서 옴)는 이미 사용 가능했지만 `S_SURFACEINFO`만
빠져 있었음. `basic/GL_import.h`를 통째로 include하면 이 파일이 이미 다른
시그니처로 정의한 `InitializeGL()`과 충돌하므로, 필요한 `basic/2d.h`만
직접 include함.

## 커밋 메시지

```
fix: RenderingFunctions.cpp에 S_SURFACEINFO 정의를 위한 basic/2d.h include 추가

rectangle()/FillRect()가 S_SURFACEINFO*를 매개변수로 쓰는데 이 타입이 정의된
basic/2d.h를 include하지 않아 미선언 오류 및 뒤이은 구문 오류가 연쇄
발생하던 문제 수정. GL_import.h는 이미 다른 시그니처로 정의된
InitializeGL()과 충돌하므로 전체 대신 basic/2d.h만 include함.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/RenderingFunctions.cpp` | `#include "../basic/2d.h"` 추가 |

## 검증

- `MSBuild build/vs2019/DarkEden.vcxproj /p:Configuration=Debug /p:Platform=x64`
- `RenderingFunctions.cpp(62, 67)` 관련 오류 전부 0건으로 해소됨을 확인
