# 플랫폼 추상화 계층 구현 요약

## ✅ 완료된 작업

### 1. 플랫폼 추상화 계층 생성 (Level 1 - Foundation)

**파일 구조:**
```
basic/
├── Platform.h              # 통일된 플랫폼 추상화 인터페이스
├── PlatformWindows.cpp     # Windows 네이티브 구현
├── PlatformSDL.cpp         # SDL/POSIX 크로스 플랫폼 구현
├── Directory.cpp           # 디렉토리 연산의 크로스 플랫폼 구현
├── Typedef.h               # Platform.h를 포함하도록 갱신됨
├── PlatformUtil.h          # 플랫폼 추상화를 사용하도록 갱신됨
├── CMakeLists.txt          # 빌드 설정
└── README.md               # 상세 문서
```

### 2. API 매핑 표

| 원본 Windows API | 플랫폼 추상화 API | Windows | SDL/POSIX |
|-----------------|-------------|---------|-----------|
| **시간 함수** |
| `timeGetTime()` | `platform_get_ticks()` | timeGetTime | SDL_GetTicks |
| `GetTickCount()` | `platform_get_ticks()` | timeGetTime | SDL_GetTicks |
| `QueryPerformanceCounter()` | `platform_get_performance_counter()` | QueryPerformanceCounter | SDL_GetPerformanceCounter |
| `Sleep(ms)` | `platform_sleep(ms)` | Sleep | SDL_Delay |
| **스레드/동기화** |
| `CreateThread()` | `platform_thread_create()` | CreateThread | SDL_CreateThread |
| `WaitForSingleObject(thread)` | `platform_thread_wait()` | WaitForSingleObject | SDL_WaitThread |
| `CreateMutex()` | `platform_mutex_create()` | CreateMutex | SDL_CreateMutex |
| `WaitForSingleObject(mutex)` | `platform_mutex_lock()` | WaitForSingleObject | SDL_LockMutex |
| `ReleaseMutex()` | `platform_mutex_unlock()` | ReleaseMutex | SDL_UnlockMutex |
| `CreateEvent()` | `platform_event_create()` | CreateEvent | SDL_Cond + SDL_Mutex |
| `WaitForSingleObject(event)` | `platform_event_wait()` | WaitForSingleObject | SDL_CondWait |
| `SetEvent()` | `platform_event_signal()` | SetEvent | SDL_CondSignal |
| **파일 연산** |
| `GetModuleFileName()` | `platform_get_executable_dir()` | GetModuleFileNameA | readlink / _NSGetExecutablePath |
| `GetFileAttributes()` | `platform_file_exists()` | GetFileAttributes | stat |
| `CreateDirectory()` | `platform_create_directory()` | CreateDirectoryA | mkdir |
| **동적 라이브러리** |
| `LoadLibrary()` | `platform_lib_load()` | LoadLibraryA | SDL_LoadObject |
| `GetProcAddress()` | `platform_lib_get_symbol()` | GetProcAddress | SDL_LoadFunction |
| `FreeLibrary()` | `platform_lib_free()` | FreeLibrary | SDL_UnloadObject |
| **설정** |
| `RegOpenKeyEx()` | `platform_config_get_string()` | RegOpenKeyExA | 파일 읽기 |
| `RegQueryValueEx()` | (포함됨) | RegQueryValueExA | 설정 파일 파싱 |
| `RegSetValueEx()` | `platform_config_set_string()` | RegSetValueExA | 파일 쓰기 |

### 3. 빌드 시스템

**CMake 지원:**
- ✅ 플랫폼 자동 감지
- ✅ 선택적 SDL 백엔드 (Windows에서 선택 가능)
- ✅ 크로스 플랫폼 컴파일 (Windows/Linux/macOS)
- ✅ 테스트 통합

**빌드 명령:**
```bash
# Windows (네이티브 백엔드)
cmake -B build -DUSE_SDL_BACKEND=OFF
cmake --build build

# Windows (SDL 백엔드)
cmake -B build -DUSE_SDL_BACKEND=ON
cmake --build build

# Linux/macOS (자동으로 SDL 사용)
cmake -B build
cmake --build build
```

### 4. 테스트 프레임워크

**테스트 커버리지:**
- ✅ 시간 함수 정밀도 테스트
- ✅ 스레드 생성 및 동기화 테스트
- ✅ 뮤텍스 테스트
- ✅ 이벤트 객체 테스트
- ✅ 파일 연산 테스트
- ✅ 디렉토리 클래스 테스트
- ✅ 키보드 상태 테스트
- ✅ 설정 시스템 테스트

**테스트 실행:**
```bash
cd build
./tests/test_platform
```

## 📊 영향 범위 분석

### 수정이 필요 없는 코드

`basic/` 헤더 파일을 포함하는 모든 코드는 **수정할 필요가 없습니다**:

```cpp
// ✅ 이 코드들은 계속 정상 동작함
#include "Typedef.h"          // DWORD, BYTE, WORD 등
#include "PlatformUtil.h"     // g_GetCtrlPushState(), SCAN_CODE
#include "Directory.h"        // gC_directory.GetProgramDirectory()
#include "BasicMemory.h"      // DeleteNew, DeleteNewArray
#include "BasicException.h"   // _Error, CheckMemAlloc
#include "DLL.h"              // DllExport, DllImport
```

### 수정이 필요한 코드 (향후)

**Windows API를 직접 사용하는 코드는 점진적으로 마이그레이션이 필요합니다:**

#### DXLib 디렉토리
```cpp
// ❌ 수정 필요
#include <windows.h>
HANDLE hThread = CreateThread(...);

// ✅ 다음과 같이 수정
#include "basic/Platform.h"
platform_thread_t hThread = platform_thread_create(...);
```

#### Client 디렉토리
```cpp
// ❌ 수정 필요
DWORD start = timeGetTime();
Sleep(100);

// ✅ 다음과 같이 수정
#include "basic/Platform.h"
DWORD start = platform_get_ticks();
platform_sleep(100);
```

## 🎯 의존 관계도 (갱신됨)

```
┌─────────────────────────────────────────────────────────┐
│  Level 5: Client (메인 실행 파일)                        │
│  - 게임 로직, 네트워크, UI 통합                          │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────┴────────────────────────────────────┐
│  Level 4: VS_UI (UI 프레임워크)                          │
│  - 위젯, 다이얼로그, 한글 IME                            │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────┴────────────────────────────────────┐
│  Level 3: 그래픽 & 렌더링                                │
│  ├─ SpriteLib  (스프라이트 애니메이션, 팔레트 시스템)    │
│  ├─ D3DLib     (Direct3D 래퍼)                          │
│  └─ framelib   (프레임 관리)                             │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────┴────────────────────────────────────┐
│  Level 2: 플랫폼 추상화                                  │
│  ├─ DXLib (DirectX 래퍼) ← 개조 필요                    │
│  └─ WinLib (Windows API) ← 개조 필요                    │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────┴────────────────────────────────────┐
│  Level 1: Foundation ✅ 완료                             │
│  ├─ basic/Platform.h   (통일 인터페이스)                │
│  ├─ basic/Platform*.cpp (구현)                          │
│  ├─ basic/Typedef.h    (타입 정의)                      │
│  └─ basic/Directory.h   (디렉토리 연산)                 │
└─────────────────────────────────────────────────────────┘
```

## 🚀 다음 단계 계획

### 2단계: DXLib 플랫폼 추상화 (예상 3-4주)

**목표:** basic/Platform.h를 사용하여 DXLib의 플랫폼 추상화 생성

**하위 작업:**
1. ✅ **CDirectInput** - 입력 시스템
   - `platform_thread_*`를 사용하여 입력 폴링 스레드 생성
   - SDL2 이벤트 시스템 또는 네이티브 Windows 메시지 사용

2. ✅ **CDirectSound** - 효과음 재생
   - Windows 백엔드: DirectSound 유지
   - SDL 백엔드: SDL_mixer 사용

3. ✅ **CDirectMusic** - 음악 재생
   - Windows 백엔드: DirectMusic 유지
   - SDL 백엔드: SDL_mixer music 사용

4. ✅ **CDirectDraw** - 렌더링 서페이스
   - engine/sprite의 SDL2 구현 사용을 고려 가능

### 3단계: SpriteLib 마이그레이션 (예상 2-3주)

**목표:** engine/sprite의 SDL2 구현 사용

**방안 A:** API를 유지하고 내부적으로만 engine 사용
```cpp
// CSprite 내부적으로 engine의 DecodedSprite 사용
class CSprite {
    DecodedSprite* m_engineSprite;
public:
    void Blt(WORD* pDest, WORD pitch) {
        // engine으로 호출 변환
    }
};
```

**방안 B:** engine API를 직접 사용 (권장)
```cpp
// 새 코드는 engine을 직접 사용
#include "engine/sprite/include/sprite.h"
Sprite sprite;
sprite_load(&sprite, file);
```

### 4단계: WinLib 마이그레이션 (예상 2-3주)

**목표:** CWinMain이 SDL2 윈도우를 사용하도록 함

```cpp
// CWinMain 내부적으로 SDL_Window 사용
class CWinMain {
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
public:
    BOOL Init(const WINMAIN_INFO* pInfo) {
        // SDL로 윈도우 생성
    }
};
```

### 5단계: VS_UI 마이그레이션 (예상 4-6주)

**목표:** UI 시스템이 engine/ui를 사용하거나 새 추상화 계층을 생성하도록 함

### 6단계: Client 메인 프로그램 (예상 2-3주)

**목표:** 메인 게임 루프가 SDL2 이벤트 시스템을 사용하도록 함

## 📝 마이그레이션 체크리스트

### ✅ Level 1: Foundation (완료)
- [x] Platform.h 인터페이스 정의
- [x] PlatformWindows.cpp 구현
- [x] PlatformSDL.cpp 구현
- [x] Directory.cpp 크로스 플랫폼 구현
- [x] Typedef.h 갱신
- [x] PlatformUtil.h 갱신
- [x] CMakeLists.txt 빌드 설정
- [x] 테스트 프로그램 test_platform
- [x] 문서 README.md

### ⏳ Level 2: DXLib (다음 단계)
- [ ] CDirectInput 플랫폼 추상화
- [ ] CDirectSound 플랫폼 추상화
- [ ] CDirectMusic 플랫폼 추상화
- [ ] CDirectDraw 플랫폼 추상화 (선택 사항)

### ⏳ Level 3: SpriteLib
- [ ] engine/sprite 호환성 평가
- [ ] 어댑터 계층 생성 또는 직접 마이그레이션
- [ ] 스프라이트 로딩 및 렌더링 테스트
- [ ] ColorSet 시스템 테스트

### ⏳ Level 4: WinLib
- [ ] CWinMain SDL2 윈도우 추상화
- [ ] 메시지 루프 마이그레이션
- [ ] WinMain 대체

### ⏳ Level 5: VS_UI
- [ ] UI 프레임워크 평가
- [ ] engine/ui 사용 또는 어댑터 계층 생성

## 💡 주요 장점

1. **점진적 마이그레이션** - 각 레벨을 독립적으로 완료할 수 있고, 언제든 테스트 가능
2. **하위 호환성** - 상위 레벨 코드를 대규모로 수정할 필요 없음
3. **검증 가능** - 각 단계마다 테스트로 검증됨
4. **성능 최적화** - Windows는 계속 네이티브 API 사용
5. **크로스 플랫폼** - Linux/macOS는 SDL2 사용

## 🔧 도구 및 자원

**생성된 파일:**
```
basic/
├── Platform.h              # 310줄, 완전한 인터페이스 정의
├── PlatformWindows.cpp     # 230줄, Windows 구현
├── PlatformSDL.cpp         # 320줄, SDL/POSIX 구현
├── Directory.cpp           # 80줄, 디렉토리 연산
├── Typedef.h               # 갱신됨
├── PlatformUtil.h          # 갱신됨
├── CMakeLists.txt          # 빌드 설정
└── README.md               # 280줄 문서

tests/
├── CMakeLists.txt          # 테스트 설정
└── test_platform.cpp       # 270줄 테스트 프로그램

CMakeLists.txt              # 최상위 설정
PLATFORM_MIGRATION.md       # 본 문서
```

**총 코드량:** 약 1,500줄의 새 코드

---

**작성일:** 2025.01.14
**상태:** Level 1 (Foundation) 완료 ✅
**다음 단계:** Level 2 (DXLib) 플랫폼 추상화 시작
