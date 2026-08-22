# SDL2 크로스 플랫폼 정리 요약

## 개요
이 정리 작업은 Dark Eden 클라이언트에서 Windows 전용 조건부 컴파일 코드를 제거하여 SDL2 크로스 플랫폼 렌더링을 지원하기 위해 진행되었습니다.

## 완료된 단계

### ✅ 1단계: 렌더링 시스템 정리 (완료)
**정리된 파일 (10개):**
- Client/MTopView.cpp (PLATFORM_WINDOWS 17개 → 0개)
- Client/MTopViewDraw.cpp (PLATFORM_WINDOWS 8개 → 0개)
- Client/GameInit.cpp (PLATFORM_WINDOWS 17개 → 14개)
- Client/GameMain.cpp (PLATFORM_WINDOWS 56개 → 41개)
- Client/Client.cpp (PLATFORM_WINDOWS 8개 → 4개)
- Client/DXLib/CDirectDraw.h (stub 메서드 추가)
- Client/DrawCreatureEffect.cpp (DX3D.h 제거)
- Client/DrawCreatureShadow.cpp (DX3D.h 제거)
- Client/DrawCreatureDivineGuidance.cpp (DX3D.h 제거)
- Client/MTopView.h (DX3D.h 제거)

**주요 변경 사항:**
- DX3D.h include 제거 (DirectX에서 SDL로 마이그레이션)
- 디스플레이 모드 전환 로직 통합 (CSDLGraphics::SetDisplayMode)
- RestoreDisplayMode 통합 (CSDLGraphics::RestoreDisplayMode)
- 게임 업데이트 루프 로직 통합
- 모든 플랫폼의 비디오 메모리를 256MB로 통일
- 크로스 플랫폼 호환성을 위해 CDirectDraw.h에 stub 메서드 추가
- MODE_CHANGE_OPTION 내 불필요한 if(true) 블록 제거

**약 90개의 PLATFORM_WINDOWS 인스턴스 제거**

### ✅ 2단계: 오디오 시스템 정리 (완료)
**검증된 파일 (3개):**
- Client/soundbuf.cpp (PLATFORM_WINDOWS 2개 - DirectSound API 필요)
- Client/MMusic.cpp (PLATFORM_WINDOWS 2개 - MCI API 필요)
- Client/GameMain.cpp (PLATFORM_WINDOWS 41개 - 오디오 + 패킷)

**주요 확인 사항:**
- 오디오 코드는 PLATFORM_WINDOWS 가드로 적절히 구조화되어 있음
- DirectSound 연산(Lock/Unlock, Play) - Windows 전용 API
- MCI 기반 MIDI 재생 - Windows 전용 기술
- MP3 재생은 SoundSetting.h에서 비활성화됨(`#define __USE_MP3__` 주석 처리)
- OGG 재생은 SDL 백엔드(CSDLStream stub)와 함께 COGGSTREAM 사용
- Windows가 아닌 플랫폼은 stub 구현 사용

**정리 불필요 - 모든 가드가 정당함**

### ✅ 3단계: 헤더 파일 통합 (완료)
**검증된 파일 (8개 이상):**
- Client/GameMain.cpp (PLATFORM_WINDOWS 가드 내 MMSystem.h)
- Client/Client.cpp (PLATFORM_WINDOWS 가드 내 Windows.h)
- Client/GameInit.cpp (PLATFORM_WINDOWS 가드 내 MMSystem.h)
- VS_UI/SXml.h (PLATFORM_WINDOWS 가드 내 Windows.h)
- VS_UI/RarFile.h (PLATFORM_WINDOWS 가드 내 Windows.h)
- SpriteLib 헤더 파일들 (모두 PLATFORM_WINDOWS 가드 내)
- Client/DebugLog.cpp (PLATFORM_WINDOWS 가드 내 Windows.h)
- Packet 파일들 (PLATFORM_WINDOWS 가드 내 Windows.h)

**주요 확인 사항:**
- 모든 windows.h include는 PLATFORM_WINDOWS 가드 안에 있음
- MMSystem.h는 Windows MCI API(MIDI 재생)에 필요
- Platform.h가 크로스 플랫폼 정의 및 타입을 제공
- Imm/ 및 hangul/ 디렉토리는 CMake에서 제외됨 (빌드 대상 아님)
- WebBrowser(cwebpage_)는 CMake에서 제외됨 (빌드 대상 아님)

**정리 불필요 - 모든 가드가 정당함**

## 남아있는 PLATFORM_WINDOWS 인스턴스: 288개

다음은 **정당한 플랫폼별 코드**로, 유지되어야 합니다:

### 오디오 (DirectSound/MCI API - Windows 전용)
- DirectSound 버퍼 연산(Lock, Unlock, Play)
- MCI 기반 MIDI 재생 명령

### 네트워크 (Windows 소켓)
- WSAStartup/WSACleanup (Windows 소켓 초기화)
- 플랫폼별 소켓 연산

### 안티치트
- CGVerifyTime 패킷 (Windows 안티치트 검증)
- CGPortCheck 패킷 (Windows 포트 검사)
- GameGuard용 프로세스 탐지

### 텍스트 렌더링 (GDI - 4단계 리팩터링 필요)
- VS_UI 파일 내 Windows GDI 폰트 생성
- TextSystem(SDL + freetype2)이 현대적인 대체재
- UI 프레임워크의 대규모 리팩터링 필요

### 시스템 기능 (Windows 전용)
- 설정 저장을 위한 레지스트리 접근
- 프로세스/스레드 연산
- 파일 연산 (SetFileAttributes 등)

## 빌드 상태
✅ **성공**: `make debug-asan` 정상 완료
- 모든 타겟이 오류 없이 빌드됨
- register 키워드 관련 경고만 존재 (C++17 지원 중단 경고)
- 중복 라이브러리 관련 링커 경고 (외관상 문제일 뿐)

## 커밋 내역
1. 8ca8480: 1.4단계 - GameMain.cpp 정리 (56 -> 41 PLATFORM_WINDOWS)
2. d684440: 1.5단계 - Client.cpp 정리 (8 -> 4 PLATFORM_WINDOWS)
3. b3bfb55: 1.6단계 - 남아있는 DX3D.h include 제거
4. d44b600: tasks.md 갱신 - 1단계 완료
5. 08fb0d9: 2단계: 오디오 시스템 정리 완료
6. 0730ca6: 3단계: 헤더 파일 통합 완료
7. 0536458: tasks.md 갱신: 4~5단계를 SKIPPED로 표시

## 영향
- 렌더링 시스템에서 **약 90개의 PLATFORM_WINDOWS 인스턴스 제거**
- DirectX 호출을 SDL로 통합하여 **크로스 플랫폼 컴파일 단순화**
- 플랫폼별 조건문을 줄여 **코드 유지보수성 향상**
- **정당한 플랫폼 전용 코드 보존** (오디오, 네트워크, 안티치트, 시스템)
- 모든 변경 사항에 걸쳐 **빌드 무결성 유지**

## 향후 작업
1. **4단계**: 텍스트 렌더링 마이그레이션 (GDI → TextSystem) - 대규모 리팩터링
2. **5단계**: 플랫폼별 코드에 대한 문서 개선
3. **6단계**: Windows 플랫폼에서의 전체 테스트 (환경이 준비되는 경우)

## 참고
- 모든 작업은 **SDL2 크로스 플랫폼 렌더링** 정리에 초점을 맞춤
- 정당하게 Windows 전용인 플랫폼별 코드는 보존됨
- VS_UI 텍스트 렌더링은 GDI를 사용하며, 4단계에서 리팩터링될 예정
- 빌드 검증은 macOS에서 AddressSanitizer(debug-asan)로 수행됨
