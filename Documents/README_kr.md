# client

개발자가 아니라 플레이어라면, [단독 실행 튜토리얼](./standalone_version_kr.md)을 따라가면 됩니다.

요약(TL;DR)

## 컴파일

이 코드는 Win10에서 컴파일되며, VC6를 사용합니다.

xerces-c 라이브러리 3.2.3 버전을 받아서 client 프로젝트 루트 경로에 압축을 풉니다.
인터넷에서 받거나 여기 [미러 파일](https://github.com/opendarkeden/client/raw/data/xerces-c-3.2.3.zip)을 이용할 수 있습니다.

그러면 디렉토리 구조는 다음과 같이 보입니다:

```
$ ls -lah
total 202K
drwxr-xr-x 1 genius 197121   0 Jun  9 10:21 ./
drwxr-xr-x 1 genius 197121   0 Jun  9 09:53 ../
drwxr-xr-x 1 genius 197121   0 Jun  9 10:21 .git/
-rw-r--r-- 1 genius 197121 384 Jun  9 10:13 .gitignore
...
drwxr-xr-x 1 genius 197121   0 Oct 10  2015 xerces/
```

directx9 라이브러리를 받으세요. 오래되어 찾기 어려운데, 여기 [미러 파일](https://github.com/opendarkeden/client/raw/data/dx90bsdk.zip)이 있습니다.
원하는 위치에 압축을 풀고, VC6의 Include & Library 디렉토리에 추가하세요:

![image](https://user-images.githubusercontent.com/1420062/121283362-8949b900-c90d-11eb-8a7e-eeac6eb4135b.png)
![image](https://user-images.githubusercontent.com/1420062/121283745-3a505380-c90e-11eb-91cc-2c6ecfd76479.png)

반드시 Game(800) 경로일 필요는 없으며, 어느 경로에 두어도 됩니다.


VC6에서 client/Client/Client.dsw 워크스페이스를 열면 모든 것이 정상 동작해야 합니다.

## 실행

[Mediafire](https://www.mediafire.com/file/017bif66kyieviw/DARKEDEN.zip/file) 또는 [바이두 넷디스크](https://pan.baidu.com/s/1-DufSEmnydMbOtTwOo_h8A) (추출 코드 6bcl)에서 데이터 파일을 다운로드하여 `client/DARKEDEN`에 압축을 풀고, "window.bat"를 실행하세요.

`DARKEDEN/Data/Info/GameClient.inf`에서 IP/Port를 본인의 로그인 서버 IP/Port로 수정하세요.
로컬 Windows 시스템에서 도커를 사용 중이라면 IP는 `127.0.0.1`이어야 합니다.

"window.bat"는 인자를 붙여 "fengshen.exe"를 실행하는 단순한 스크립트입니다:

```
fengshen.exe 0000000001 
```

인자의 의미:

- 0000000001 창 모드
- 0000000002 전체화면
- 0000000003 창 모드 1024
- 0000000004 전체화면 1024


게임을 플레이하려면 [서버](https://github.com/opendarkeden/server)가 필요합니다.
서버를 배포하려면 [docker 설치 가이드](https://github.com/opendarkeden/server/blob/master/docker_install.md)를 참고하세요.
