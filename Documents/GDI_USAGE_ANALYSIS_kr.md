# Windows GDI 사용 현황 분석

## 결론

**네, Windows GDI는 여전히 사용되고 있지만, Windows 플랫폼에서만 사용됩니다.**

## 상세 분석

### 1. 이중 경로 아키텍처

현재 코드는 이중 경로 아키텍처를 사용합니다:

#### Windows 플랫폼 (PLATFORM_WINDOWS)
```cpp
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <DSound.h>

// GDI 사용
void Base::SetFont(...) {
    HFONT hfont = CreateFontIndirect(&lf);  // GDI
    pi.hfont = hfont;
}

void Base::~Base() {
    DeleteObject(m_small_pi.hfont);  // GDI
    DeleteObject(m_chatting_pi.hfont);
    // ... 그 외 DeleteObject 호출들
}
#endif
```

#### 비 Windows 플랫폼 (macOS/Linux)
```cpp
#else
#include "../../basic/Platform.h"
#include "TextSystem/FontHandleUtil.h"

// TextSystem 사용 (SDL + freetype2)
void Base::SetFont(...) {
    pi.hfont = TextSystem::EncodeFontSizeHandle(lf.lfHeight);  // SDL
}

// DeleteObject는 Platform.h에서 stub으로 구현됨
static inline int DeleteObject(void* hObject) {
    (void)hObject;
    return 1;  // 아무 동작도 하지 않고 TRUE 반환
}
#endif
```

### 2. 사용 위치

**주요 파일:** `VS_UI/src/VS_UI_Base.cpp`

| 기능 | Windows (GDI) | macOS/Linux (TextSystem) |
|------|---------------|---------------------------|
| 폰트 생성 | `CreateFontIndirect()` | `TextSystem::EncodeFontSizeHandle()` |
| 폰트 삭제 | `DeleteObject()` | Stub (아무 동작 없음) |
| 문자셋 | `HANGUL_CHARSET`, `GB2312_CHARSET` | `DEFAULT_CHARSET` |
| 폰트 이름 | "굴림체" | "Arial" |

### 3. Platform.h 내의 GDI Stub

비 Windows 플랫폼에서는 GDI 함수들의 stub 구현을 제공합니다:

```cpp
// basic/Platform.h

/* GDI 객체 관리 함수 - stub 구현 */
static inline int DeleteObject(void* hObject) {
    (void)hObject;
    /* Stub - Windows GDI 객체 삭제 */
    return 1; /* TRUE 반환 */
}

static inline void* GetStockObject(int nIndex) {
    (void)nIndex;
    return NULL;
}
```

**역할:** 코드가 컴파일되게는 하지만, 실제로는 아무 동작도 수행하지 않습니다.

### 4. 마이그레이션 현황

| 구성 요소 | Windows | macOS/Linux |
|------|---------|-------------|
| 텍스트 렌더링 | **GDI** (여전히 사용 중) | TextSystem ✅ |
| 폰트 관리 | GDI | TextSystem ✅ |
| DeleteObject | 실제 GDI 함수 | Stub (아무 동작 없음) |
| CreateFont | 실제 GDI 함수 | 사용 안 함 |

---

## GDI를 제거할 수 있을까?

### Windows 지원을 완전히 제거하는 경우 (Windows 빌드가 필요 없다면)

**가능하지만, 작업이 필요합니다:**

1. **`#ifdef PLATFORM_WINDOWS`로 감싸진 GDI 코드 전부 삭제**
   - CreateFontIndirect 호출 삭제
   - DeleteObject 호출 삭제
   - LOGFONT 관련 코드 삭제

2. **TextSystem으로 통일**
   - 모든 플랫폼에서 TextSystem::EncodeFontSizeHandle 사용
   - GDI stub 제거 (Platform.h 내)

3. **검증이 필요한 사항**
   - TextSystem 기능이 완전한지
   - Windows에서 폰트 렌더링이 정상 동작하는지
   - 한글/중국어 지원이 완전한지

### 현재 상태 유지 (권장)

**Windows 지원이 여전히 필요하다면, 이중 경로를 유지하는 것을 권장합니다:**

**장점:**
- Windows는 네이티브 GDI 사용 (안정적이고 성숙함)
- macOS/Linux는 TextSystem 사용 (크로스 플랫폼)
- 두 플랫폼이 독립적이며 서로 영향을 주지 않음

**단점:**
- 두 세트의 코드를 유지보수해야 함
- 코드 복잡도가 비교적 높음

---

## 의존 관계

```
VS_UI_Base.cpp
    ├─ #ifdef PLATFORM_WINDOWS
    │      └─ Windows GDI (CreateFontIndirect, DeleteObject)
    │
    └─ #else (macOS/Linux)
           └─ TextSystem (SDL + freetype2)
                  └─ FontHandleUtil.h
```

---

## 요약

1. **GDI는 여전히 사용되고 있음** - 단, Windows 플랫폼에서만
2. **macOS/Linux는 TextSystem으로 완전히 마이그레이션됨** - GDI를 더 이상 사용하지 않음
3. **이중 경로 아키텍처가 존재함** - Windows는 GDI, 나머지 플랫폼은 TextSystem 사용

---

## 제안

### 옵션 A: 현재 상태 유지 (권장)
Windows 지원이 필요하다면, 현재의 이중 경로 아키텍처를 유지합니다.

### 옵션 B: GDI 완전 제거
Windows 지원이 필요 없거나(또는 Windows도 TextSystem을 사용하도록 한다면) 다음이 필요합니다:
1. TextSystem이 Windows에서 정상 동작하는지 확인
2. `#ifdef PLATFORM_WINDOWS`로 감싸진 GDI 코드 전부 삭제
3. TextSystem으로 통일

### 옵션 C: TextSystem 먼저 검증
TextSystem이 GDI를 대체할 준비가 되었는지 확인:
```bash
# TextSystem 사용처 검색
grep -r "TextSystem" VS_UI/src/ --include="*.cpp" | wc -l

# TextSystem 구현 파일 확인
ls Client/TextSystem/
```

---

**원하시는 방향은:**
1. **현재 상태 유지** - Windows는 계속 GDI 사용
2. **GDI 완전 제거** - 모든 플랫폼이 TextSystem을 사용하도록 함 (테스트 필요)
3. **먼저 TextSystem 확인** - TextSystem의 현재 상태를 파악

선택해 주시기 바랍니다!
