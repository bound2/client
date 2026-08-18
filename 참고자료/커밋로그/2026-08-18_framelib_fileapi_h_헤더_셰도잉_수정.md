# 커밋 로그

- 일시: 2026-08-18
- 대상: `Client/framelib/CMakeLists.txt`
- 유형: fix (빌드 오류 수정, framelib)

## 커밋 메시지

```
fix: framelib CMakeLists.txt에서 ../Packet include 경로 제거 (fileapi.h 셰도잉)

Client/framelib/CMakeLists.txt가 target_include_directories에
${CMAKE_CURRENT_SOURCE_DIR}/../Packet 를 추가하고 있었는데, framelib
소스 중 실제로 Packet 헤더를 include하는 곳은 없음. 문제는 이 경로가
컴파일러 include 검색 경로에 들어가면서, Client/Packet/fileapi.h
(FileAPI 네임스페이스의 예외 기반 파일 I/O 래퍼, 실제 Win32 API와
무관한 프로젝트 자체 파일)가 <windows.h>가 내부적으로 요구하는 진짜
Windows SDK <fileapi.h> 대신 잡히는 헤더 셰도잉이 발생.

그 결과:
- Packet::fileapi.h의 오버로드 함수(open_ex, fcntl_ex)가
  extern "C" 링키지로 처리되어 C2733(오버로드 불가) 오류
- 진짜 fileapi.h가 정의해야 할 LPCREATEFILE2_EXTENDED_PARAMETERS 등의
  타입이 없어 이후 fileapifromapp.h/winbase.h 파싱이 연쇄적으로 깨짐
  (C2061 구문 오류)

framelib가 필요로 하지 않는 ../Packet include 경로를 제거하여 셰도잉을
해소.
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/framelib/CMakeLists.txt` | `target_include_directories(framelib PUBLIC ...)`에서 `${CMAKE_CURRENT_SOURCE_DIR}/../Packet` 항목 제거 + 사유 주석 추가 |

## 빌드 검증

- `cmake .` (build/vs2019 재구성)
- `MSBuild build/vs2019/Client/framelib/framelib.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild`
- 수정 전: `fileapi.h(32,5): error C2733`, `fileapi.h(62,5): error C2733`,
  `fileapifromapp.h(80,14): error C2061`, `winbase.h(6376,16): error C2061` (CAnimationFrame.cpp,
  CAnimationFrameSetManager.cpp, CCreatureFrameSetManager.cpp, CFrame.cpp, CFramePack.cpp,
  CFrameSetManager.cpp 등 framelib 전 파일에서 반복)
- 수정 후: 위 오류 모두 해소, `framelib.lib` 빌드 성공 (exit code 0)

## 참고 (범위 외 발견 사항 — 이번 수정에 포함하지 않음)

`Client/Packet/fileapi.h`라는 파일명 자체가 Windows SDK의 실제 `<fileapi.h>`와 충돌하는
구조적 위험 요소임. 이번에는 framelib의 불필요한 include 경로만 제거해 문제를 피했지만,
다른 프로젝트(vcxproj/CMakeLists)가 향후 `Client/Packet`을 include 경로에 추가하면 동일한
셰도잉이 재발할 수 있음. 근본적으로는 `Client/Packet/fileapi.h`를 다른 이름으로 리네이밍하는
것이 안전하나, 이는 FileAPI.cpp/SocketAPI.cpp/SocketImpl.cpp/DatagramSocket.cpp 등 여러 파일을
함께 고쳐야 해서 이번 작업(요구사항 4: 재탐색 금지) 범위를 벗어나 포함하지 않음.
