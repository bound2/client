

# 목표

`work 5 - DarkEden exe 최초 링크 오류.log`(798건) 정리를 이어서 진행 중.
오늘(2026-08-21) 오전에 다음 두 건을 끝냄:

1. **사운드 서브시스템 클러스터** - `VS_UI/WinMain.cpp`와 같은 성격의 "중복
   구현" 문제(VC6 원본에서는 `DXLib` 전용이었던 `Client/soundbuf.cpp`·
   `mp3.cpp`가 CMake 글롭 때문에 루트에도 컴파일되고 있었음), `basic/AudioTypes.h`
   vs 실제 `<mmsystem.h>`/`<dsound.h>` 충돌, `ServerInformation`과 Windows SDK
   `combaseapi.h` 이름 충돌, `IDirectSoundBuffer` 불완전 타입 직접 호출 등을
   해소. 자세한 내용은
   `참고자료\커밋로그\2026-08-21_work5_사운드_서브시스템_정리.md` (커밋 `9e60d44`).
2. **`CSystemInfo.cpp`의 VC6 인라인 어셈블리(`__asm`)** - x64는 인라인
   어셈블리를 지원하지 않아(`error C4235`) 전부 오류였음. `<intrin.h>`의
   `__rdtsc()`/`__cpuid()` 인트린식으로 교체(커밋 `3e38317`).

이 두 건으로 오류가 **798건(중복 제거 후 673건 고유) → 374건 고유**로 줄었음.

다음으로 오류가 가장 많이 남은 건 **`Client/MWorkThread.cpp`(58건)** - 이건
**오후에 진행 예정**. 이 문서는 오후 작업을 시작할 때 바로 참고할 수 있도록
현재 상태만 기록해두는 용도(아직 수정 작업은 안 함).

같이 참고할 것:
1) vs6 에서 빌드 하려고 했던 본래 소스
"H:\Source\GithubDesktop\client-master_vs6"


## `MWorkThread.cpp` 오류 미리보기

오류가 전부 `MWorkNode`/`MWorkThread` 클래스의 멤버가 선언되지 않았다는
`C2039`/`C2065`(예: `m_pNext`, `m_pPrev`, `m_pFirstWorkNode`,
`m_pLastWorkNode`, `m_pCurrentWork`, `m_hHasWorkEvent`, `GetFirstWorkNode()`,
`ExecuteWorkNode()`)와, 시그니처가 안 맞는다는 `C2511`/`C2660`/`C2664`
(`MWorkThread::Remove()`, `MWorkNode::Execute()`,
`platform_thread_create()`)로 구성됨. `.cpp`의 구현이 참조하는 멤버들이
`.h` 선언에 없거나 시그니처가 어긋난 상태로 보임 - 헤더(`MWorkThread.h`)와
구현(`MWorkThread.cpp`)이 마이그레이션 과정에서 서로 어긋난 것인지, VC6
원본(`client-master_vs6`) 대비 어떻게 달라졌는지부터 확인 필요.

# 수정 대상

`Client/MWorkThread.cpp` (및 필요시 `Client/MWorkThread.h`)

# 요구사항
3. 코드 스타일: 
   - Visual Studio 2019 환경 , C++ 11 표준 사용중
   - Allman 스타일(중괄호 다음 줄), 헝가리언 표기법 준수.
   - 한 줄 제어문도 줄바꿈/중괄호 필수.
   - 인코딩: UTF-8 with BOM.   
4. 기존 로직 보존: 기존 프로젝트의 다른 코드는 건드리지 마세요(재탐색 금지).
5. 응답은 항상 한글로 해주세요.

