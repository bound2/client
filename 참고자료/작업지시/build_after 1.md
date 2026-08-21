

# 목표

`빌드관련/work 9.md`(2026-08-22 새벽, `DarkEden.exe` 빌드 성공 - 컴파일
오류 0건, 링크 오류 0건)를 이어서 진행. 이 문서는 **빌드 성공 이후**,
실제로 클라이언트를 실행하면서 나온 런타임 문제들을 정리하는 용도.

같이 참고할 것:
1) vs6 에서 빌드 하려고 했던 본래 소스
"H:\Source\GithubDesktop\client-master_vs6"
2) 빌드까지의 경위는 `참고자료\작업지시\빌드관련\work 1.md` ~ `work 9.md`
3) 오늘(빌드 성공 이후) 작업 상세 커밋 기록:
   - `6e6998f` fix: LeakMemoryDumper의 무한 재귀 및 x64 포인터 절단 버그 수정
   - `179fc7d` fix: DLL 화이트리스트에 SDL2/vcpkg 런타임 DLL 추가
   - `21c11b7` fix: DarkEden 타겟에 VS_DEBUGGER_WORKING_DIRECTORY 설정 추가

---

## 1. 실행 중 크래시 - `LeakMemoryDumper.h`의 `RemoveTrack()` 호출 시 접근 위반

빌드는 성공했지만 처음 실행했을 때 `RemoveTrack()` 호출 중
`0xC0000005`(잘못된 주소 `0x10` 읽기) 크래시 발생. `LeakMemoryDumper.cpp`
(2004년작 메모리 누수 추적 도구)가 이번 세션에 **처음으로 실제
컴파일+링크**됐기 때문에(그 전엔 616개 컴파일 오류에 막혀 한 번도 실행된
적이 없었음) 원본부터 있던 버그 두 개가 처음 드러남:

1. **`AddTrack()`의 무한 재귀**: `info = new (ALLOC_INFO);`도
   `#define new DEBUG_NEW`(`LeakMemoryDumper.h`) 매크로에 걸려서, 추적
   구조체 자신을 할당하려고 또 우리가 오버로드한 placement
   `operator new`를 호출하고, 그 안에서 `AddTrack()`을 다시 호출 -
   프로그램에서 `new`가 처음 호출되는 순간 무조건 재귀. `malloc()`으로
   직접 할당하도록 수정.
2. **x64 포인터를 `DWORD`(32비트)로 절단**: 실제 64비트 힙 주소를
   32비트로 잘라서 저장/비교하고 있어서, 서로 다른 두 포인터의 하위
   32비트가 우연히 같으면 `RemoveTrack()`이 엉뚱한 항목을 리스트에서
   지워 리스트가 깨질 수 있었음. `address`/`asize`를 `uintptr_t`/
   `size_t`로 넓혀서 해결, `DumpUnfreed()`의 `sprintf` 포맷 문자열도
   맞춤.

## 2. `Client.cpp`의 DLL 화이트리스트 검사가 `WinMain()`을 조용히 종료시킴

`WinMain()` 안의 2006년산 DLL 무결성 검사(`//add by sonic Check *.dll
have Bug. 2006.4.13`)가 실행 파일 폴더의 모든 `*.dll`을 훑어서 하드코딩된
화이트리스트(`ifc22.dll`/`gl.dll`/`xerces-c_2_4_0.dll` 등 VC6 시절
라이브러리)에 없으면 `return -1`로 `WinMain()` 자체를 즉시 종료(팝업도
주석 처리되어 있어서 조용히 꺼짐) - 게임 초기화가 시작되기도 전에.

vcpkg가 실행 파일 옆에 자동으로 복사하는 SDL2/SDL2_ttf/freetype/
libpng16/zlib/jpeg/brotli/bz2 런타임 DLL이 화이트리스트에 하나도 없어서
매번 걸리고 있었음. Debug/Release 설정마다 DLL 이름이 다르므로
(`SDL2d.dll` vs `SDL2.dll` 등) 둘 다 화이트리스트에 추가해서 해결.

## 3. `Data` 폴더 위치 및 F5 디버깅 작업 디렉터리

- `GameInit.cpp`의 `g_pFileDef->load("Data/Info/FileDef.inf")`가 상대
  경로를 씀 - 실행 파일과 같은 폴더
  (`build\vs2019\bin\Debug\`)에 `Data\`를 놓아야 함(사용자가 직접 배치,
  완료).
- Visual Studio F5(로컬 Windows 디버거)가 기본적으로 `.vcxproj`가 있는
  `build\vs2019`를 작업 디렉터리로 써서 exe 폴더와 어긋나는 함정이
  있었음 - `VS_DEBUGGER_WORKING_DIRECTORY`를 exe 폴더로 맞춰서 해결.
  (참고: `build/` 산출물인 `DarkEden.vcxproj.user`는 Visual Studio가
  디버깅 인자(`LocalDebuggerCommandArguments`, 예:
  `0000000003` = 창모드 1024x768, `CLAUDE.md` 참고)를 저장할 때마다 IDE가
  자체적으로 다시 쓰는 파일이라, `WorkingDirectory` 설정이 IDE 조작으로
  없어져 있을 수 있음 - git에는 안 잡히는 파일이라 문제되면 다시
  넣어주면 됨.)

여기까지 정리되어 `Data` 폴더를 넣고 실행까지는 확인됨.

# 수정 대상

**`Client/GameInit.cpp` 1654번째 줄의 `if` 조건부터 디버깅 시작**:

```cpp
if (InitClientConfig() &&
    InitInfomation() &&
    InitUserOption() &&

    InitVolume() &&
    InitInput() &&
    //InitSound()  &&
    InitDraw() &&

    //InitThread() &&		// 2001.8.20 주석처리 - 로딩 Thread사용 안함
    InitGameObject() &&    // GameObject must be initialized before Surface
                            // because gC_vs_ui.Init() in InitSurface() depends on g_pMoneyManager
    InitSurface() &&

    InitGameUpdate()
    )		
{	
    // 성공 시: CheckInvalidProcess(), UI_AffectUserOption(),
    // ProfileManager::DeleteProfiles()/InitProfiles() 등으로 이어짐
    // (1758번째 줄~)
    ...
    return TRUE;
}

// Init가 실패한 경우 (1831~1832번째 줄)
return FALSE;
```

이 함수(`InitGame()`)가 게임 초기화의 최상위 게이트 - 9개의
`InitXxx()` 서브함수를 순서대로 `&&`로 묶어서, 하나라도 `FALSE`를
반환하면 전체가 `FALSE`가 되고 `InitGame()` 자체가 실패로 끝남(어느
지점에서 실패했는지 알려주는 로그/메시지는 딱히 없어 보임 - 그냥
`return FALSE;`).

**다음 세션에서 할 일**: 이 줄에 브레이크포인트를 걸고 9개
`InitXxx()`를 하나씩 스텝오버하면서 어디서 `FALSE`가 나오는지(또는
전부 `TRUE`인데 그 이후 로직에서 문제가 나는지) 추적. 순서대로:
1. `InitClientConfig()`
2. `InitInfomation()`
3. `InitUserOption()`
4. `InitVolume()`
5. `InitInput()`
6. `InitDraw()` (오늘 세션에 `CSDLGraphics::Init()`/`Flip()` 실구현한
   부분 - `빌드관련/work 8.md` 참고)
7. `InitGameObject()`
8. `InitSurface()` (오늘 `InitializeGL()` 호출 제거한 부분 - 위 커밋
   `8f26428` 참고)
9. `InitGameUpdate()`

# 요구사항
3. 코드 스타일: 
   - Visual Studio 2019 환경 , C++ 11 표준 사용중
   - Allman 스타일(중괄호 다음 줄), 헝가리언 표기법 준수.
   - 한 줄 제어문도 줄바꿈/중괄호 필수.
   - 인코딩: UTF-8 with BOM.   
4. 기존 로직 보존: 기존 프로젝트의 다른 코드는 건드리지 마세요(재탐색 금지).
5. 응답은 항상 한글로 해주세요.
