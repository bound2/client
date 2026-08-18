# 커밋 로그

- 일시: 2026-08-18
- 대상: `Client/Packet/FileAPI.{h,cpp}` → `PacketFileAPI.{h,cpp}` 리네이밍 및 include 갱신
- 유형: refactor (구조적 위험 요소 제거 - 헤더 이름 충돌 예방)

## 배경

framelib LNK/컴파일 오류(직전 커밋 `5a5c552`)의 근본 원인은 `Client/Packet/FileAPI.h`라는
파일명이 진짜 Windows SDK 헤더 `<fileapi.h>`와 대소문자만 다르고 이름이 같아서, 이 디렉터리가
컴파일러 include 경로에 들어가면 `<windows.h>`가 요구하는 진짜 `<fileapi.h>`를 가려버리는
구조적 위험(헤더 셰도잉)이었음. framelib는 불필요한 include 경로를 제거해 우회했지만, 근본
원인인 파일명 충돌 자체는 남아 있었기에 이번에 리네이밍으로 정리함.

## 커밋 메시지

```
refactor: FileAPI.h/.cpp를 PacketFileAPI.h/.cpp로 리네이밍 (Windows SDK 헤더 충돌 예방)

Client/Packet/FileAPI.h(.cpp)는 실제 Win32 API와 무관한 프로젝트 자체
파일(FileAPI 네임스페이스의 예외 기반 파일 I/O 래퍼)인데, 파일명이 진짜
Windows SDK 헤더 <fileapi.h>와 겹쳐서 이 디렉터리가 컴파일러 include
경로에 들어가면 <windows.h> 내부의 #include <fileapi.h>가 이 파일로
셰도잉되는 구조적 위험이 있었음(framelib에서 실제로 발생, 5a5c552로 우회
수정함). 이름 충돌 자체를 없애기 위해 PacketFileAPI.h/.cpp로 리네이밍하고
이를 include하던 4개 파일(SocketImpl.cpp, SocketAPI.cpp, DatagramSocket.cpp,
자기 자신)의 include문을 갱신.

git mv로 이력을 보존했으며, 함수/클래스/네임스페이스 로직은 전혀 변경하지
않음(파일명·include문·상단 주석만 변경).
```

## 변경 파일 목록

| 파일 | 변경 내용 |
| --- | --- |
| `Client/Packet/FileAPI.h` → `Client/Packet/PacketFileAPI.h` | `git mv`로 리네이밍, 상단 주석에 리네이밍 사유 추가 |
| `Client/Packet/FileAPI.cpp` → `Client/Packet/PacketFileAPI.cpp` | `git mv`로 리네이밍, `#include "FileAPI.h"` → `#include "PacketFileAPI.h"`, 상단 주석 갱신 |
| `Client/Packet/SocketImpl.cpp` | `#include "FileAPI.h"` → `#include "PacketFileAPI.h"` |
| `Client/Packet/SocketAPI.cpp` | `#include "FileAPI.h"` → `#include "PacketFileAPI.h"` |
| `Client/Packet/DatagramSocket.cpp` | `#include "FileAPI.h"` → `#include "PacketFileAPI.h"` |

CMakeLists.txt는 `Client/Packet/*.cpp`를 glob으로 수집하므로 별도 수정 불필요(자동 반영 확인).

## 검증

- `grep`으로 저장소 전체에서 `#include "FileAPI.h"` 잔존 여부 확인 → 0건
- `cmake .` (build/vs2019 재구성) 정상 완료
- `MSBuild build/vs2019/DarkEden.vcxproj /t:Rebuild` 시도 → **VS_UI.vcxproj의 대량의 기존(이번
  작업과 무관한) 오류**로 DarkEden 자체 소스 컴파일 단계까지 도달하지 못함. 로그에서
  FileAPI/PacketFileAPI/Socket/Datagram 관련 오류는 0건으로 확인(리네이밍 자체는 문제 없음).
- 우회 검증: `cl.exe /c`로 `PacketFileAPI.cpp`, `SocketImpl.cpp`, `SocketAPI.cpp`,
  `DatagramSocket.cpp` 4개 파일을 DarkEden.vcxproj의 include 경로/매크로 그대로 개별 컴파일 →
  리네이밍/include 관련 오류 0건. 남은 오류는 전부 `__WINDOWS__`(빌드 시스템 어디에서도 정의되지
  않는 죽은 매크로)로 감싸진 기존 Windows 분기 코드의 미선언 식별자 오류, 그리고 `MinTr.h`의
  기존 구문 오류로, 둘 다 이번 리네이밍과 무관한 사전 존재 버그.

## 참고 (범위 외 발견 사항 — 이번 수정에 포함하지 않음)

- `Client/Packet/PacketFileAPI.cpp`, `SocketAPI.cpp`는 `#if __WINDOWS__` / `#elif __WINDOWS__`로
  Windows 전용 구현을 분기하는데, `__WINDOWS__`는 CMakeLists.txt 어디에서도 정의되지 않는 매크로라
  이 분기들은 Windows 빌드에서도 항상 비활성 상태(POSIX 분기만 사용됨)임. `fd`, `result`, `client`,
  `nSent`, `nrecv` 등 다수의 미선언 식별자 오류가 여기서 발생. 실제로 Windows용 소켓/파일 구현이
  필요한지, 아니면 죽은 코드로 정리해야 하는지 판단이 필요해 이번 작업(재탐색 금지) 범위에
  포함하지 않음.
- `Client/MinTr.h`(168, 236줄)의 구문 오류, `VS_UI` 프로젝트의 대량 오류(CDirectDraw.h 재정의,
  CImm.h 누락, VS_UI_Filepath.h 인코딩 문제 등)도 이번 범위 밖으로 남겨둠.
