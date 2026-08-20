# 커밋 로그

- 일시: 2026-08-21
- 대상: `VS_UI/src/VS_UI_DESC.cpp`, `VS_UI/src/VS_UI_GameCommon.cpp`,
  `VS_UI/src/vs_ui_gamecommon2.cpp`, `VS_UI/src/VS_UI_progress.cpp`,
  `VS_UI/src/VS_UI_Title.cpp`, `VS_UI/src/VS_UI_util.cpp`
- 유형: fix (빌드 오류 수정, VS_UI - `SetSurfaceInfo(S_SURFACEINFO*, DDSURFACEDESC2*)`
  오버로드에 SDL 대체 반환 타입을 잘못 넘기던 문제, 8곳)

## 원인

`work 4 error.log`에 `error C2665: 'SetSurfaceInfo': 2개의 오버로드 중 모든
인수 형식을 변환할 수 있는 오버로드가 없습니다`가 8개 파일에 걸쳐 반복 발생함.
전부 다음과 같은 동일한 코드 패턴이었음:

```cpp
S_SURFACEINFO surface_info;
SetSurfaceInfo(&surface_info, gpC_base->m_p_DDSurface_back->GetDDSD());
```

`basic/GL_import.h`가 선언한 `SetSurfaceInfo`의 두 오버로드는
`(S_SURFACEINFO*, const DDSURFACEDESC2*)`와 `(S_SURFACEINFO&, void*, int, int,
int)`뿐인데, `CSpriteSurface::GetDDSD()`는 `S_SURFACEINFO*`를 반환해서(SDL
마이그레이션 이후 "Windows API 호환용 래퍼"로 추가된 것 —
`Client/SpriteLib/CSpriteSurface_SDL.cpp(500)`, 내부적으로
`GetSurfaceInfo()`를 호출해 정적 버퍼에 채운 뒤 그 포인터를 반환) 두 오버로드
모두와 인수 타입이 맞지 않음.

`VS_UI_DESC.cpp(112)`에는 이미 `#ifdef PLATFORM_WINDOWS`(고장난
`SetSurfaceInfo`+`GetDDSD()` 호출) / `#else`(`GetSurfaceInfo()` 직접 호출)
분기가 있었고, `#else` 쪽이 정확히 올바른 SDL 경로였음. `GetDDSD()`가
내부적으로 `GetSurfaceInfo()`를 호출해 정적 버퍼에 채운 뒤 반환하는 것뿐이므로,
`GetSurfaceInfo(&surface_info)`를 직접 호출하는 것과 완전히 동등함. 나머지
7곳은 애초에 `#ifdef` 분기 없이 이 고장난 형태로만 존재했음.

## 커밋 메시지

```
fix: SetSurfaceInfo(S_SURFACEINFO*, GetDDSD()) 시그니처 불일치 8곳 일괄 수정

GetDDSD()는 SDL 마이그레이션 때 추가된 호환 래퍼로 S_SURFACEINFO*를
반환하는데, SetSurfaceInfo()의 두 오버로드(DDSURFACEDESC2* 또는
void*+width+height+pitch)는 이를 받을 수 없어 C2665가 8개 파일에서
반복 발생하던 문제 수정. GetDDSD()가 내부적으로 GetSurfaceInfo()를
호출해 반환하는 것뿐이므로(CSpriteSurface_SDL.cpp), 이미 VS_UI_DESC.cpp의
#else 분기에 있던 GetSurfaceInfo(&surface_info) 직접 호출 방식으로
전부 통일함.
```

## 변경 파일 목록

| 파일 | 줄 | 변경 내용 |
| --- | --- | --- |
| `VS_UI/src/VS_UI_DESC.cpp` | 110~116 | `#ifdef PLATFORM_WINDOWS`/`#else` 분기 제거, `GetSurfaceInfo(&surface_info)` 직접 호출만 유지 |
| `VS_UI/src/VS_UI_GameCommon.cpp` | 9969, 10071 | `SetSurfaceInfo(&surface_info, GetDDSD())` → `GetSurfaceInfo(&surface_info)` |
| `VS_UI/src/vs_ui_gamecommon2.cpp` | 1899 | 동일 |
| `VS_UI/src/VS_UI_progress.cpp` | 428 | 동일 |
| `VS_UI/src/VS_UI_Title.cpp` | 5곳(2467 포함, `surfaceinfo` 변수명) | 동일 패턴 전체를 `replace_all`로 일괄 수정 |
| `VS_UI/src/VS_UI_util.cpp` | 813, 1144 | 동일 |

## 검증

- `MSBuild build/vs2019/VS_UI.vcxproj /p:Configuration=Debug /p:Platform=x64`
- `SetSurfaceInfo` 관련 `C2665` 오류 8건 전부 0건으로 해소됨을 확인
- `VS_UI_Title.cpp`는 오류 로그에 1곳(2467줄)만 보고되어 있었으나, 동일한
  고장난 패턴이 파일 내 총 5곳에 있어 전부 일괄 수정함(나머지 4곳은 그
  시점까지 컴파일이 도달하지 못해 로그에 나타나지 않았을 뿐, 같은 버그였음)
