# spec.md: Windows 전용 의존성 정리

## 목표

SDL2 마이그레이션 완료 이후, 코드베이스에 남아 있는 Windows 전용 의존성을 정리하여 SDL2 크로스 플랫폼 경로로 통일한다. Windows 환경이라 하더라도 SDL2 백엔드를 사용해야 한다.

## 배경

프로젝트는 Windows + DirectX에서 SDL2로의 마이그레이션을 완료했다. 하지만 코드 안에는 여전히 많은 `#ifdef PLATFORM_WINDOWS` 조건부 컴파일 블록이 남아 있으며, 그중 다수는 다음과 같다:
1. 폐기된 DirectX/DirectSound 코드
2. Windows GDI 코드 (이미 TextSystem으로 대체됨)
3. Windows에서만 의미가 있는 코드 (예: 안티치트, 레지스트리 등)

## 분석 결과

### 코드 통계
- **총 321곳**의 `#ifdef PLATFORM_WINDOWS` 조건부 컴파일
- **60개 이상의 파일**에 걸쳐 존재

### 주요 분류

| 분류 | 수량 | 처리 전략 |
|------|------|----------|
| 폐기된 DirectX 호출 | 21 | **제거** - SDL2로 이미 대체됨 |
| DirectSound 정리 코드 | 15 | **제거** - SDL_mixer로 이미 대체됨 |
| GDI 텍스트 렌더링 | 다수 | **제거** - TextSystem으로 이미 대체됨 |
| Windows 헤더 포함 | 173 | **검토 후 제거** |
| 안티치트/레지스트리 | 일부 | **유지** - Windows 전용 기능 |
| 네트워크 관련 (WSA) | 일부 | **유지** - 플랫폼별 구현 |

### 영향도가 큰 파일 (PLATFORM_WINDOWS 등장 횟수 기준)

1. **Client/GameMain.cpp** (56곳) - 게임 메인 루프
2. **Client/MTopView.cpp** (17곳) - 렌더링 시스템
3. **Client/GameInit.cpp** (17곳) - 초기화
4. **Client/MinTr.h** (12곳) - 추적 시스템
5. **Client/MTopViewDraw.cpp** (8곳) - 렌더링 드로잉

## 정리 범위

### Phase 1: 폐기된 렌더링 코드 제거

**대상 파일:**
- `Client/MTopView.cpp` - `if (true)` 패턴의 빈 DirectX 코드 블록 제거
- `Client/MTopViewDraw.cpp` - 렌더링 경로 통일
- `Client/GameMain.cpp` - Direct3D 복구 코드 제거

**패턴 예시:**
```cpp
// 현재 코드
#ifdef PLATFORM_WINDOWS
    if (true) {
        // 빈 DirectX 코드 블록
    }
#endif

// 정리 후
// (완전히 제거)
```

### Phase 2: DirectSound 관련 코드 제거

**대상 파일:**
- `Client/GameMain.cpp` - DirectSound 정리
- `Client/soundbuf.cpp` - 사운드 버퍼
- `Client/DXLib/soundbuf.cpp`

**패턴 예시:**
```cpp
// 현재 코드
#ifdef PLATFORM_WINDOWS
    if( g_SDLAudio.IsInit() ) {
        // DirectSound 전용 정리
    }
#endif

// 정리 후
// SDL_mixer가 자동으로 처리하므로 이 블록 제거
```

### Phase 3: 텍스트 렌더링 경로 통일

**대상 파일:**
- `VS_UI/src/VS_UI_Base.cpp` - GDI vs TextSystem
- `Client/SpriteLib/*.h` - windows.h 포함 제거

**패턴 예시:**
```cpp
// 현재 코드
#ifdef PLATFORM_WINDOWS
#include <windows.h>
// GDI 사용
#else
#include "TextSystem/FontHandleUtil.h"
// TextSystem 사용
#endif

// 정리 후 (Windows에서도 TextSystem 사용)
#include "TextSystem/FontHandleUtil.h"
// TextSystem으로 통일
```

### Phase 4: 헤더 파일 포함 정리

**대상 파일:**
- `#include <windows.h>`를 포함한 모든 파일
- Windows 전용 헤더를 조건부로 포함하는 코드

**패턴 예시:**
```cpp
// 현재 코드
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <MMSystem.h>
#endif

// 정리 후
#include "basic/Platform.h"  // 필요한 stub 제공
```

### Phase 5: 필요한 플랫폼 전용 코드 유지

**유지할 코드:**
- 네트워크 초기화 (WSAStartup/WSACleanup) - BSD 소켓에는 불필요하지만 호환성을 위해 유지
- 레지스트리 접근 - Windows에서만 의미가 있음
- 안티치트 탐지 - Windows에서만 필요
- 프로세스 탐지 - Windows에서만 의미가 있음

## 범위 밖

- `basic/Platform.h`의 stub 정의 제거 (여전히 호환성을 위해 필요함)
- Windows 플랫폼 전용 기능(레지스트리, 안티치트) 제거
- CMake 빌드 시스템 수정

## 성공 기준

1. ✅ 모든 `#ifdef PLATFORM_WINDOWS` 블록 검토 완료
2. ✅ 폐기된 DirectX/DirectSound 코드 제거
3. ✅ GDI 텍스트 렌더링 코드 제거 (TextSystem으로 통일)
4. ✅ macOS에서 프로젝트 빌드 성공
5. ✅ Windows에서 프로젝트 빌드 성공 (SDL2 사용)
6. ✅ 게임 기능 테스트 통과

## 위험 평가

| 위험 | 등급 | 완화 방안 |
|------|------|----------|
| Windows 컴파일 실패 | 중간 | 수정할 때마다 검증 |
| 런타임 기능 누락 | 중간 | 기능 테스트 |
| 성능 저하 | 낮음 | 성능 비교 테스트 |

## 의존 관계

- **전제 조건:** SDL2 마이그레이션 완료 ✅
- **전제 조건:** TextSystem 구현 완료 ✅
- **병행 작업:** 없음
