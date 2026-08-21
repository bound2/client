

# 목표

`work 5 - DarkEden exe 최초 링크 오류.log`(798건) 정리를 이어서 진행 중.
오늘(2026-08-21) 오후에 다음 네 건을 끝냄:

1. **`Client/MWorkThread.cpp`(58건)** - Windows 구현부(`#ifdef PLATFORM_WINDOWS`)가
   헤더(`MWorkThread.h`)의 `std::deque` 기반 설계와 다른, 헤더에 없는 멤버
   (`m_pFirstWorkNode`/`m_pLastWorkNode`/`pNode->m_pNext`/`m_pPrev`/
   `m_pWorkThread`, `GetFirstWorkNode()`, `ExecuteWorkNode()` 등)를 참조하는
   연결리스트 방식으로 잘못 구현돼 있었음. VC6 원본(`client-master_vs6`)의
   deque 기반 로직을 참고해 `ReleaseWork()`/`Execute()`/`Remove(int)`/
   `AddFirst()`/`AddLast()`를 재작성하고, `platform_thread_create()` 3인자
   호출 오류 수정, `LockDeque`/`LockCurrent`가 실제로 동작하도록
   `Init()`/`Release()`에 critical section 초기화/해제 추가(커밋 `9718974`).
2. **`Client/BloodBibleSignInfo.h`(56건)** - 오전 세션에서 이미 `#include
   <vector>` 누락만 고쳐두고 커밋을 안 한 상태였음. 확인해보니 실제로
   오류가 전부 해소된 상태라 그대로 커밋만 함(커밋 `7f635a4`).
3. **`ServerInformation.h`/`.cpp` → `CServerInformation` 파일명 리네임(56건+α)** -
   오전 커밋(`9e60d44`, work5 사운드 서브시스템 정리)에서 Windows SDK
   `combaseapi.h`의 `tagServerInformation`(및 typedef `ServerInformation`)과의
   이름 충돌을 피하려고 **클래스명만** `ServerInformation` → `CServerInformation`
   으로 바꾸고 `#include`도 `"CServerInformation.h"`로 고쳤는데, **실제 파일
   리네임을 빠뜨렸음**. 그 결과 이미 새 이름으로 include하던
   `GameInit.cpp`/`GameUI.cpp`/`CWaitUIUpdate.cpp`/`LCWorldListHandler.cpp`/
   `SizeOfObjects.cpp`/`ServerInformation.cpp` 자신까지 전부 "파일을 찾을 수
   없음"(`C1083`) 치명적 오류로 막혀 있었음. `git mv`로 실제 리네임하고, 아직
   옛 이름을 쓰던 6개 파일도 맞춤. **`GameUI.cpp` 오류 30건 → 0건 완전 해소**,
   `GameInit.cpp`/`CWaitUIUpdate.cpp`는 막혀있던 게 풀려서 더 깊은(이전엔
   도달조차 못했던) 오류가 새로 드러남(커밋 `96197fe`).
4. **`CSDLGraphics::Init()`/`Flip()`/`ReleaseAll()` 실구현 + `GameInit.cpp`
   잔여 오류(76건)** - `CSDLGraphics`(`Client/DXLib/CDirectDraw.h`)는 대부분
   no-op 스텁이었는데(`참고자료/작업필요stub.md`에 "실구현 vs 최소 스텁 중
   상의해서 결정"으로 기록되어 있던 항목), `GameInit.cpp`의 `InitDraw()`가
   실제로 호출하는 `Init()`이 아예 없어 컴파일조차 안 됐음. `GameMain.cpp`를
   포함해 `Flip()`이 실제 게임 루프 곳곳(프레임마다 화면 present)에서 쓰이고
   있어 사용자와 상의 후 **실제 구현**을 택함 - 이미 만들어진 네이티브 HWND를
   `SDL_CreateWindowFrom()`으로 감싸고 `SDL_CreateRenderer()`로 렌더러를 만든
   뒤 `spritectl_init()`으로 SpriteLib SDL 백엔드를 켬(`Client/SDLMain.cpp`의
   `InitApp()` 로직을 Windows 네이티브 HWND 버전으로 이식). `Flip()`은
   `CSpriteSurface` 전체 정의가 필요한데 `CDirectDraw.cpp`는 `/IClient` 없는
   별도 `dxlib` 라이브러리로 빌드되어 안전하게 끌어올 수 없어서, 새 파일
   `Client/CSDLGraphicsFlip.cpp`(DarkEden.exe 쪽 소스)에 따로 구현. 같이
   `GameInit.cpp`의 `COGGSTREAM` 생성자 인자 누락 1건과, 항상 `nullptr`을
   반환하는 `CSDLGraphics::GetDD()`로 DirectDraw VidMem을 조회하던 죽은
   코드(컴파일 자체가 안 됨)도 정리. **`GameInit.cpp` 자체 오류 0건**(커밋
   `04657f8`).
5. **`platform_get_ticks`/`<MMSystem.h>` 이름 충돌** - `basic/Platform.h`는
   "이 프로젝트는 real `<MMSystem.h>`를 어디서도 include하지 않는다"는 전제로
   `timeGetTime()`/`GetTickCount()`를 `platform_get_ticks()`로 치환하는
   매크로를 걸어두고 있는데, `CGameUpdate.cpp`/`CWaitUIUpdate.cpp`/
   `GameMain.cpp`/`GameInit.cpp`/`GameInitInfo.cpp`/`Profiler.cpp`가 실제로는
   `timeGetTime()`/`GetTickCount()`(또는 아무것도) 말고는 real MM_ 심볼을 안
   쓰면서도 `<MMSystem.h>`를 계속 include하고 있어서 전제가 깨져 있었음.
   include되면 실제 `timeGetTime(void)` 선언이 매크로에 치환되어 서로 다른
   연결(linkage)로 재정의되며 `timeapi.h`에서 `C2375`가 나고, 같은 번역
   단위의 `MItem.h`/`CWinUpdate.h`/`VS_UI_GameCommon.h`까지 연쇄로 깨졌음.
   6개 파일은 `<MMSystem.h>` include를 제거했고, `WavePackFileManager.cpp`만
   진짜 WAV 파싱용 MMIO API(`mmioOpen` 등)가 필요해서 지울 수 없어 include
   직전에 `#undef timeGetTime`/`GetTickCount`로 매크로만 국소적으로 비활성화
   (커밋 `684fb1c`).

이 다섯 건으로 (work5 log 기준 첫 배치 컴파일) **오류가 704건 → 616건**으로
줄었음(단, `ServerInformation` 파일명 리네임으로 `GameInit.cpp`/
`CWaitUIUpdate.cpp` 등이 막혀있다가 풀리면서 새로 드러난 오류가 섞여있어
work5 log 원본의 798건과 직접 비교는 안 맞음 - 지금부터는 실제 빌드
재실행 기준으로 추적하는 게 정확함).

오늘은 여기까지. 이 문서는 다음 작업을 시작할 때 바로 참고할 수 있도록
현재 상태만 기록해두는 용도(아직 수정 작업은 안 함).

같이 참고할 것:
1) vs6 에서 빌드 하려고 했던 본래 소스
"H:\Source\GithubDesktop\client-master_vs6"

2) 오늘 오후 작업 상세 커밋 기록 (순서대로)
- `9718974` fix: MWorkThread.cpp를 deque 기반 헤더에 맞춰 재구현
- `7f635a4` fix: BloodBibleSignInfo.h에 `<vector>` include 누락 수정
- `96197fe` fix: ServerInformation.h/.cpp를 CServerInformation으로 실제 파일명 리네임
- `04657f8` feat: CSDLGraphics::Init()/Flip()/ReleaseAll() 실구현 및 GameInit.cpp 잔여 오류 해소
- `684fb1c` fix: platform_get_ticks/<MMSystem.h> 이름 충돌 해소

3) 빈 스텁 관련 정리 문서 (CSDLGraphics 실구현으로 일부 항목 해소됨,
   업데이트 필요할 수 있음)
참고자료\작업필요stub.md


## 다음 오류 미리보기 (같은 배치 재컴파일 기준, 대략적인 건수)

work5 log는 이제 상당 부분 stale(당시엔 안 보이던 오류가 오늘 여러 파일의
막힌 include가 풀리면서 새로 드러남)이라, 아래는 오늘 마지막에 다시 빌드
돌린 결과 기준. 큰 것부터:

- `Client/Packet/Gpackets/GCKnocksTargetBackOK1.cpp` (40건)
- `Client/Packet/Gpackets/GCAttackArmsOK1.cpp` (40건)
- `Client/CDirectDrawSurface.h` (30건)
- `Client/Packet/PacketFileAPI.cpp` (26건)
- `Client/MInternetConnection.cpp` (26건)
- `Client/Packet/SocketAPI.cpp` (24건)
- Stash 관련 `Gpackets`/`Cpackets` 다수: `GCStashSell.cpp`/`GCStashList.cpp`/
  `GCPetStashList.cpp`/`GCGoodsList.cpp`/`CGStashWithdraw.cpp`/
  `CGStashToMouse.cpp`/`CGStashRequestBuy.cpp`/`CGStashList.cpp`/
  `CGStashDeposit.cpp`/`CGMouseToStash.cpp` (각 24건)
- `Client/CrashReport.cpp` (24건) - DbgHelp 관련 구조체(`_tagSTACKFRAME64`,
  `_KDHELP64`, `_tagADDRESS64` 등) 재정의(`C2011`), `_CONTEXT`에 `Eip`/`Ebp`
  없음(`C2039`) 등. real `<dbghelp.h>`/`<imagehlp.h>`와 프로젝트 자체 스텁
  구조체가 충돌하는 것으로 보임 - work5 사운드 서브시스템 정리 때 다룬
  `DSound.h`/`combaseapi.h` 충돌과 같은 성격일 가능성.
- `Client/PCConfigTable.cpp` (20건)
- `Client/MTopView.cpp` (16건)
- `Client/Packet/ClientCommunicationManager.cpp` (14건)
- `Client/GameMain.cpp` (10건 남음) - `_O_RDONLY` 미선언, `SaveSurfaceToImage`
  인자 타입 불일치(`CSpriteSurface`→`CDirectDrawSurface&`), `SOUND_ERR_OK`/
  `SOUND_PLAY_ONCE` 미선언 등 `platform_get_ticks`와 무관한 별개 오류들.
- 이 밖에 `PacketValidator.cpp`/여러 `Handler.cpp`(각 8건), `DatagramSocket.cpp`(8건),
  `ProfileManager.cpp`(6건) 등 소규모 다수.

# 수정 대상

(다음 세션에서 위 목록 중 우선순위 상의해서 결정 - `GCKnocksTargetBackOK1.cpp`/
`GCAttackArmsOK1.cpp`가 동률로 가장 크고, `CrashReport.cpp`는 성격이 달라
보여서 별도 조사 필요)

# 요구사항
3. 코드 스타일: 
   - Visual Studio 2019 환경 , C++ 11 표준 사용중
   - Allman 스타일(중괄호 다음 줄), 헝가리언 표기법 준수.
   - 한 줄 제어문도 줄바꿈/중괄호 필수.
   - 인코딩: UTF-8 with BOM.   
4. 기존 로직 보존: 기존 프로젝트의 다른 코드는 건드리지 마세요(재탐색 금지).
5. 응답은 항상 한글로 해주세요.
