
## 클라이언트 다운로드


1. 클라이언트 데이터 파일

바이두 넷디스크 https://pan.baidu.com/s/1-DufSEmnydMbOtTwOo_h8A 추출 코드 6bcl

또는 Mediafire https://www.mediafire.com/file/017bif66kyieviw/DARKEDEN.zip/file

2. `DARKEDEN/Data/Info/GameClient.inf` 파일 내용을 수정한다. 단독 실행판(단일 서버)에서는 `127.0.0.1`이어야 한다.

3. DARKEDEN 디렉터리 아래에 여러 개의 bat 파일이 있으며, 이를 클릭해서 실행하면 '창 모드' 또는 '전체 화면 모드'로 게임이 시작된다.

## 서버 설치

Docker Desktop을 다운로드하여 설치한다

https://docs.docker.com/desktop/install/windows-install/


미리 패키징된 서버용 docker 이미지를 다운로드한다


```
docker pull tiancaiamao/darkeden
```

mysql의 docker 이미지를 다운로드한다


```
docker pull mysql/mysql-server:5.7
```

아래 내용을 docker-compose.yml 파일로 저장한다


```
version: '3'

services:

  odk-mysql:
    image: mysql/mysql-server:5.7
    volumes:
      - ../initdb:/docker-entrypoint-initdb.d/
    environment:
        MYSQL_ROOT_PASSWORD: 123456
    restart: on-failure
    command: mysqld --sql_mode="ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION"
    networks:
      - odk-network

  odk-server:
    image: tiancaiamao/darkeden:latest
    build:
      context: ..
      dockerfile: Dockerfile
    command: ["sleep","infinity"]
    ports:
      - "9999:9999"
      - "9998:9998"
      - "9997:9997"
      - "9997:9997/udp"
    networks:
      - odk-network

networks:
  odk-network:
```


docker 이미지를 실행한다


```
docker-compose -f docker-compose.yml up -d
```

서버를 시작한다

```
docker exec -it docker_odk-server_1  ./start.sh
```

종료 방법


```sh
docker exec -it docker_odk-server_1  ./stop.sh
docker-compose down
```
