# plan.md: Windows 전용 의존성 정리

## 기술 방안

### 현재 상태 분석

코드 감사 결과, 다음과 같은 Windows 전용 코드 패턴이 발견되었다:

#### 패턴 1: 폐기된 DirectX 코드 블록 (`if (true)` 패턴)
```cpp
#ifdef PLATFORM_WINDOWS
    if (true) {
        // 빈 DirectX 관련 코드
    }
#endif
```
**처리:** 이런 빈 블록을 완전히 제거

#### 패턴 2: DirectSound 정리 코드
```cpp
#ifdef PLATFORM_WINDOWS
    if( g_SDLAudio.IsInit() ) {
        // DirectSound 정리
    }
#endif
```
**처리:** SDL_mixer가 자동으로 처리하므로, 이 블록들을 제거

#### 패턴 3: 조건부 렌더링 경로
```cpp
#ifdef PLATFORM_WINDOWS
    // Windows 전용 렌더링
#else
    // SDL 렌더링
#endif
```
**처리:** SDL 경로로 통일하고, Windows 분기를 제거

#### 패턴 4: 조건부 헤더 파일 포함
```cpp
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <MMSystem.h>
#else
#include "basic/Platform.h"
#endif
```
**처리:** `#include "basic/Platform.h"`로 통일

#### 패턴 5: GDI vs TextSystem
```cpp
#ifdef PLATFORM_WINDOWS
    HFONT hfont = CreateFontIndirect(&lf);  // GDI
#else
    pi.hfont = TextSystem::EncodeFontSizeHandle(lf.lfHeight);  // TextSystem
#endif
```
**처리:** (Windows에서도) TextSystem으로 통일

---

## 실행 계획

### Phase 1: 렌더링 관련 조건부 컴파일 정리 (우선순위 높음)

**대상 파일:**
1. `Client/MTopView.cpp` (PLATFORM_WINDOWS 17곳)
2. `Client/MTopViewDraw.cpp` (8곳)
3. `Client/GameMain.cpp` (56곳 - 일부는 렌더링 관련)

**작업 내용:**
- `if (true)` 형태의 빈 코드 블록 제거
- SDL 렌더링 경로로 통일
- 폐기된 DirectDraw/Direct3D 호출 제거

### Phase 2: 오디오 관련 조건부 컴파일 정리

**대상 파일:**
1. `Client/GameMain.cpp` - MP3/OGG 재생
2. `Client/soundbuf.cpp`
3. `Client/MMusic.cpp`

**작업 내용:**
- DirectSound 정리 코드 제거
- SDL_mixer 경로로 통일

### Phase 3: 헤더 파일 포함 통일

**대상 파일:**
`#ifdef PLATFORM_WINDOWS`를 포함한 모든 헤더 포함 블록

**작업 내용:**
- `#include <windows.h>`를 `#include "basic/Platform.h"`로 교체
- `<MMSystem.h>` 등 Windows 전용 헤더 제거

### Phase 4: 텍스트 렌더링 경로 통일

**대상 파일:**
1. `VS_UI/src/VS_UI_Base.cpp`
2. `VS_UI/src/VS_UI_Title.cpp`
3. 기타 UI 파일

**작업 내용:**
- GDI 폰트 생성 코드 제거
- TextSystem으로 통일

### Phase 5: 유지해야 할 플랫폼 전용 코드 정리

**유지하되 단순화할 것:**
1. 네트워크 초기화 (WSAStartup) - 조건부 호출로 변경
2. 레지스트리 접근 - 유지하되 Windows 전용으로 표시
3. 안티치트 탐지 - 유지하되 Windows 전용으로 표시

---

## 파일 목록

### 수정 대상 파일 (우선순위순)

| 우선순위 | 파일 | PLATFORM_WINDOWS 수 | 주요 수정 내용 |
|--------|------|----------------------|----------|
| P0 | Client/GameMain.cpp | 56 | 렌더링/오디오 조건부 컴파일 정리 |
| P0 | Client/MTopView.cpp | 17 | 렌더링 경로 통일 |
| P0 | Client/GameInit.cpp | 17 | 초기화 조건부 컴파일 정리 |
| P1 | Client/MinTr.h | 12 | 추적 시스템 조건부 컴파일 정리 |
| P1 | Client/MTopViewDraw.cpp | 8 | 렌더링 경로 통일 |
| P1 | Client/Client.cpp | 8 | 클라이언트 조건부 컴파일 정리 |
| P2 | Client/PacketFunction.cpp | 7 | 네트워크 함수 |
| P2 | Client/UIMessageManager.cpp | 5 | UI 메시지 |
| P2 | Client/ProfileManager.cpp | 4 | 설정 파일 |
| P2 | Client/MPlayer.cpp | 4 | 플레이어 시스템 |
| P3 | VS_UI 파일들 (다수) | 각 2~3개 | UI 컴포넌트 |

### 삭제가 필요할 수 있는 파일

없음 - 이번 정리는 조건부 컴파일 단순화에 집중하며, 파일 삭제는 하지 않는다

---

## 위험 평가와 완화 방안

| 위험 | 가능성 | 영향 | 완화 방안 |
|------|--------|------|----------|
| Windows 컴파일 실패 | 중간 | 높음 | 각 phase 완료 후 Windows에서 컴파일 테스트 |
| 런타임 크래시 | 낮음 | 높음 | macOS에서 테스트 후 Windows 수정 진행 |
| 기능 손실 | 낮음 | 중간 | 제거하는 코드 블록을 하나씩 검토 |

---

## 테스트 전략

1. **컴파일 테스트**
   ```bash
   # macOS
   make debug-asan

   # Windows (환경이 있는 경우)
   cmake --build build --config Debug
   ```

2. **기능 테스트**
   - 게임 실행
   - UI 표시
   - 텍스트 렌더링
   - 오디오 재생

---

## 실행 순서

1. ✅ 코드베이스 분석 (완료)
2. Phase 1: 렌더링 관련 조건부 컴파일 정리
3. Phase 2: 오디오 관련 조건부 컴파일 정리
4. Phase 3: 헤더 파일 포함 통일
5. Phase 4: 텍스트 렌더링 경로 통일
6. Phase 5: 유지할 플랫폼 전용 코드 정리
7. 빌드 검증
8. 기능 테스트
