# 텍스트 마이그레이션 계획 (SDL)

목표: 레거시 Win+DirectX 텍스트를, UI와 인게임 렌더링을 모두 지원하는 SDL 기반의 계층화된 텍스트 시스템으로 대체한다.

## 설계 개요

### 아키텍처
```
UI / 게임 코드
  -> TextService (레이아웃 + 측정 + 그리기)
       -> TextBackend (SDL_ttf)
            -> 글리프 캐시 (spritectl 스프라이트)
                -> RenderTarget (SpriteSurface 어댑터)
```

### 렌더 타겟 규칙
- UI 텍스트와 인게임 텍스트 모두 `SpriteSurfaceRenderTarget`을 통해 `CSpriteSurface`에 렌더링된다.
- 활성 타겟은 `g_SetFL2Surface(CSpriteSurface*)`를 통해 설정하며, 설정하지 않았을 경우 기본값은 `g_pLast`이다.
- `g_SetFL2Surface`에는 raw surface 포인터를 절대 전달하면 안 된다 (반드시 `CSpriteSurface*`여야 함).

### 인코딩 전략
- 모든 렌더링 경로는 UTF-8로 정규화한다.
- 입력이 유효한 UTF-8이 아니면 CP949, 그다음 EUC-KR 순으로 시도한다.
- 이 작업은 호출마다 변환하는 것을 피하기 위해 `TextService` 내부에서 중앙집중식으로 처리된다.

### 폰트 전략
- `PrintInfo::hfont`는 레거시에서는 Windows 핸들이지만, SDL 빌드에서는 크기 정보를 인코딩한 토큰이다.
- `Base::SetFont`가 `lf.lfHeight`를 핸들에 인코딩하며, `TextService::GetFont(size)`가 이를 사용한다.
- 이를 통해 GDI에 의존하지 않고도 UI 화면 간 기존의 크기 차이를 그대로 유지한다.

### 렌더링 API
- `g_Print`, `g_PrintColorStr*`, `g_GetStringWidth/Height`는 모두 `TextService`로 라우팅된다.
- 정렬은 가능한 경우 `PrintInfo::text_align`을 사용한다.
- 그림자/외곽선은 다중 그리기 오프셋을 통해 구현된다.

## 세부 계획

### 3단계 — 인게임 텍스트 렌더링
범위:
- 플레이어/NPC 이름, 시스템 메시지, 공지, 아이템 라벨, 채팅 말풍선.
- 여전히 `g_Print*`와 `g_GetStringWidth/Height`를 사용하는 모든 인게임 호출 지점.

계획:
1. `g_Print*` + `g_GetStringWidth/Height`가 TextService와 올바른 RenderTarget을 사용하도록 보장.
2. 인게임 UI 전반에서 폰트 크기 매핑이 일관되는지 검증.
3. 필요 시 정규화 로직을 확장하여 남아있는 깨진 문자열 수정.
4. `MTopView.cpp` 오버레이(중앙 정렬 이름 등)의 정렬 상태 검증.

완료된 작업 (2026-01-30):
1. ✅ RenderTarget 관리 검증 완료:
   - MTopView::m_pSurface는 g_pLast로 설정됨 (GameInit.cpp:765)
   - 텍스트 렌더링은 g_pFL2Surface → g_pLast → g_pBack 순으로 반환하는 ResolveTextSurface()를 사용
   - 일반 렌더링 중에는 g_pFL2Surface가 NULL이므로, 텍스트는 g_pLast에 렌더링됨
   - MTopView와 텍스트 렌더링이 동일한 surface를 사용함 - 수정 불필요

2. ✅ 폰트 크기 인코딩 검증 완료:
   - Base::SetFont()는 EncodeFontSizeHandle()을 사용해 lf.lfHeight를 인코딩함 (VS_UI_Base.cpp:128)
   - g_Print()는 DecodeFontSizeHandle()을 사용해 디코딩함 (RenderingFunctions.cpp:127)
   - 폰트 크기가 전체 파이프라인을 통해 올바르게 전달됨 - 수정 불필요

3. ✅ UTF-8 버퍼 오버플로우 수정:
   - Utf8Decode() 함수에 경계값 검사 추가
   - 모든 호출 지점 업데이트 (MeasureLineWidth, WrapText, DrawLine)
   - 잘못된 형식의 UTF-8 시퀀스로 인한 크래시 방지

수용 기준:
- 화면에 보이는 어떤 인게임 오버레이에도 깨진 텍스트가 없어야 함.
- 중앙 정렬 라벨 및 공지의 정렬이 올바라야 함.
- 렌더 타겟 전환 시 크래시가 없어야 함.
- [ ] 대기 중: 실제 한글 텍스트로 인게임 테스트
- [ ] 대기 중: 실제 환경에서 정렬 검증

### 4단계 — 텍스트 입력
범위:
- LineEditorVisual (채팅 입력, 로그인, 대화상자 입력).
- SDL 텍스트 입력을 통한 IME (한글/중국어).

계획:
1. LineEditorVisual을 TextService를 통해 렌더링 (GDI / FL2 미사용).
2. UTF-8 커서 위치 계산이 렌더링된 글리프와 일치하도록 보장.
3. 입력 버퍼는 UTF-8로 유지하고, 레거시 패킷 형식이 필요한 경우에만 변환.

수용 기준:
- 채팅 입력이 처음부터 끝까지 동작함 (입력 -> 화면 표시 -> 전송).
- 커서 위치가 글리프 위치와 일치함.
- IME 조합 중 입력 버퍼가 손상되지 않음.

### 4단계 — 텍스트 입력 (완료 ✅)
범위:
- LineEditorVisual (채팅 입력, 로그인, 대화상자 입력).
- SDL 텍스트 입력을 통한 IME (한글/중국어).

완료된 작업 (2026-01-31):
1. ✅ InputFocusManager 생성:
   - 텍스트 입력 포커스를 관리하는 싱글턴 패턴
   - SDL_TEXTINPUT, SDL_TEXTEDITING, SDL_KEYDOWN 이벤트를 라우팅
   - 파일: VS_UI/src/InputFocusManager.{h,cpp}

2. ✅ SDL 이벤트 라우팅 수정:
   - DXLibBackendSDL.cpp가 이제 텍스트 이벤트를 InputFocusManager로 라우팅함
   - 제어키(백스페이스, delete, 화살표, home/end)에 대한 VK_* 키 매핑 추가

3. ✅ LineEditorVisual 업데이트:
   - Acquire()가 InputFocusManager에 등록하고 SDL 텍스트 입력을 활성화함
   - Unacquire()가 포커스를 해제하고 SDL 텍스트 입력을 비활성화함
   - 소멸자가 InputFocusManager 참조를 정리함 (use-after-free 방지)

4. ✅ 텍스트 렌더링 통합:
   - CGlyphCache/CTextLayout 이중 시스템 제거 (약 160줄)
   - 이제 모든 렌더링에 TextService를 사용
   - 올바른 베이스라인 및 커서 위치 계산

5. ✅ 인코딩 문제 수정:
   - GetStringWide()가 이제 UTF-32 → UTF-16을 올바르게 변환함
   - 채팅은 GetString()을 통해 UTF-8을 직접 사용
   - AddWhisperID()의 버퍼 오버플로우 수정

6. ✅ 제어키 수정:
   - 백스페이스가 올바르게 동작함 (커서가 이중으로 움직이지 않음)
   - Delete, 화살표, home/end 모두 정상 작동
   - 디센더(descender) 문자(g, j, p, q, y)의 정렬 수정

수용 기준:
- ✅ 채팅 입력이 처음부터 끝까지 동작함 (입력 → 화면 표시 → 전송)
- ✅ 커서 위치가 글리프 위치와 일치함
- ✅ IME 조합 중 입력 버퍼가 손상되지 않음
- ✅ UI 파괴 시 크래시 없음

### 5단계 — UI 모듈 전수 조사 (완료 ✅)
범위:
- 메일 / 상점 / 거래 / 기타 대화상자.

완료된 작업 (2026-01-31):
1. ✅ 모든 VS_UI 파일에서 Windows 전용 텍스트 코드 스캔:
   - Windows GDI TextOut 호출은 2건만 발견됨
   - VS_UI_GameCommon.cpp:5115: g_PrintColorStr를 사용하는 #else 분기가 존재함
   - hangul/FL2.cpp:188: 레거시 FL2 렌더러 (폴백 시스템)

2. ✅ 모든 주요 대화상자가 올바른 API를 사용하는지 검증:
   - VS_UI_Shop.cpp: 아이템은 스프라이트 시스템, 텍스트는 버튼 사용
   - VS_UI_Storage.cpp: g_Print 사용 (4회 호출)
   - VS_UI_Exchange.cpp: g_Print 사용 (14회 호출)
   - VS_UI_Description.cpp: g_Print를 광범위하게 사용 (335회 호출)

3. ✅ 하드코딩된 너비 계산 없음을 확인:
   - 모든 대화상자가 측정에 g_GetStringWidth/Height를 사용
   - strlen() * FONT_SIZE 같은 편법이 없음

4. ✅ 모든 플랫폼 분기에 SDL 대안이 존재함:
   - 총 15개의 #ifdef PLATFORM_WINDOWS 블록
   - 모두 대응하는 #else 또는 플랫폼별 구현이 존재함

수용 기준:
- ✅ UI 화면이 텍스트를 일관되게 렌더링함
- ✅ SDL 빌드에서 GDI/DirectX에 대한 의존성 없음
- ✅ 모든 텍스트가 g_Print* 또는 TextService를 사용함

계획:
1. 남아있는 임의(ad-hoc) 텍스트 렌더링을 `g_Print*` 또는 TextService 직접 호출로 대체.
2. 너비/높이 계산 편법을 `g_GetStringWidth/Height`로 대체.
3. SDL 빌드에서 Windows 전용 텍스트 코드 경로 제거.

수용 기준:
- UI 화면이 텍스트를 일관되게 렌더링함.
- SDL 빌드에서 GDI/DirectX에 대한 의존성 없음.

## 예정된 파일 변경 목록 (지속 업데이트)

### 핵심 텍스트 시스템
- `Client/TextSystem/TextService.*`
- `Client/TextSystem/TextBackendSDL.cpp`
- `Client/TextSystem/RenderTargetSpriteSurface.*`
- `Client/TextSystem/FontHandleUtil.h`

### 전역 렌더링 훅
- `Client/RenderingFunctions.cpp`
- `Client/GameInit.cpp`
- `Client/CGameUpdate.cpp`
- `VS_UI/src/VS_UI_Base.cpp`

### 인게임 렌더링
- `Client/MTopView.cpp`
- `Client/CGameUpdate.cpp`

### UI 입력
- `VS_UI/src/widget/U_edit.cpp`
- `VS_UI/src/VS_UI_Title.cpp`
- `VS_UI/src/VS_UI_GameCommon.cpp`

## 위험 요소 / 주의 사항
- RenderTarget은 항상 `CSpriteSurface*`여야 한다 (raw surface 포인터 금지).
- 폰트 크기 차이는 `lf.lfHeight`가 보존되는 것에 의존한다.
- 일부 레거시 문자열은 여전히 CP949/EUC-KR일 수 있으므로, 정규화는 중앙집중식으로 유지되어야 한다.

## 테스트 체크리스트
- NPC 대화 텍스트 (다중 라인 + 메뉴 옵션).
- 인게임 시스템 공지 + 플레이어/NPC 이름 라벨.
- 채팅 입력 (타이핑 + 커서 + IME).
- 폰트 크기가 혼합된 UI 화면.

## 마일스톤

1) 텍스트 시스템 기반
   - TextService 구현 (레이아웃, 측정, 그리기)
   - SDL_ttf 백엔드 + 글리프 캐시
   - RenderTarget 추상화 (SpriteSurface 어댑터)
   - 상태: ✅ 완료

2) NPC 대화 텍스트
   - TextService를 통한 대화 메시지 + 메뉴 렌더링
   - 인코딩 정규화 (CP949/EUC-KR -> UTF-8)
   - 상태: ✅ 완료

3) 인게임 텍스트 렌더링
   - g_Print/g_GetStringWidth/Height를 TextService로 라우팅
   - 올바른 RenderTarget 보장 (g_pLast / g_pBack)
   - 남아있는 깨진 텍스트나 정렬 문제 수정
   - 상태: ⏳ 진행 중

4) 텍스트 입력
   - LineEditorVisual 렌더링 + 커서
   - UTF-8/IME 정확성 (SDL 텍스트 입력 경로)
   - 남아있는 GDI/FL2 의존성 제거
   - 상태: ✅ 완료 (2026-01-31)

5) UI 모듈 전수 조사
   - 메일 / 거래 / 상점 / 기타 대화상자
   - 필요한 곳에 남아있는 g_PrintColorStr/g_GetStringWidth 사용 대체
   - 상태: ✅ 완료 (2026-01-31)

## 참고사항

- SDL만이 대상 플랫폼이며, 레거시 Win+DirectX 경로는 더 이상 필요하지 않다.
- 깔끔한 추상화를 지향한다: UI/게임 로직에서 DXLib/Surface를 직접 사용하지 않는다.
- 가능한 모든 곳에서 텍스트 렌더링과 측정에 TextService를 사용한다.
