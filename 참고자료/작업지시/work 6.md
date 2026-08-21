

# 목표

오늘(2026-08-21) `VS_UI/WinMain.cpp`를 죽은 진입점으로 제외하고 `Client.cpp`
쪽을 진짜 진입점으로 확정한 뒤, `DarkEden.exe`(실제 실행파일) 링크를 처음으로
끝까지 시도해봤어. 그동안 `VS_UI.vcxproj`만 빌드해왔어서 `CLIENT_MAIN_SOURCES`
(GameMain.cpp, MCreature.cpp 등 실제 게임플레이 파일들)는 이번에 처음
컴파일된 거라 새 오류가 대량으로 나왔어. 아래 파일 보고 수정해줘.

참고자료\작업지시\work 5 - DarkEden exe 최초 링크 오류.log

(오류 761건, 파일별로는 CSystemInfo.cpp/MWorkThread.cpp/BloodBibleSignInfo.h/
CAvi.cpp/GameInit.cpp/MMusic.cpp/GameUI.cpp/Packet 쪽 Gpackets·Cpackets 다수
순으로 많음. 오류 유형은 C2065(미선언 식별자)가 324건으로 제일 많고,
C2143/C2146(구문 오류) 159건, C2039(멤버 아님) 80건 순)

같이 참고할 것:
1) vs6 에서 빌드 하려고 했던 본래 소스
"H:\Source\GithubDesktop\client-master_vs6"

2) 빈 stub 관련 정리문서
참고자료\작업필요stub.md

3) 오늘 WinMain.cpp 정리 작업 기록
참고자료\커밋로그\2026-08-21_VS_UI_WinMain_cpp_죽은_진입점_제외.md


# 수정 대상



# 요구사항
3. 코드 스타일: 
   - Visual Studio 2019 환경 , C++ 11 표준 사용중
   - Allman 스타일(중괄호 다음 줄), 헝가리언 표기법 준수.
   - 한 줄 제어문도 줄바꿈/중괄호 필수.
   - 인코딩: UTF-8 with BOM.   
4. 기존 로직 보존: 기존 프로젝트의 다른 코드는 건드리지 마세요(재탐색 금지).
5. 응답은 항상 한글로 해주세요.


