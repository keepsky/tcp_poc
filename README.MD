# TCP Client-Server PoC

이 프로젝트는 C 언어로 작성된 TCP 서버와 C# 전문을 JSON 형태로 통신하는 클라이언트 간의 Proof of Concept (PoC) 구현체입니다.

## 📌 구조 및 개요
1. **Server (`/server`)**
   * **언어**: `C`
   * **기능**: TCP 소켓(포트 `8890`)을 열고 대기합니다. 클라이언트가 1~4 중 하나의 메세지 타입을 요청하면, 내부에 하드코딩된 더미 `struct` 데이터를 JSON 문자열로 변환합니다. 변환된 JSON 길이(4 bytes)를 먼저 전송하고 뒤이어 실제 JSON 데이터를 전송합니다.

2. **Client (`/client`)**
   * **언어**: `C# (.NET Console App)`
   * **기능**: `127.0.0.1:8890`에 연결하여 1부터 4까지의 메세지 타입을 2초 간격으로 서버에 순차적으로 요청합니다. 서버로부터 4바이트 크기 정보를 읽어들인 후 JSON 버퍼를 수신하고, `System.Text.Json`을 이용해 C# 객체 클래스(`Type1` ~ `Type4`)로 변환(역직렬화)하여 콘솔에 내용을 출력합니다.

---

## 🚀 실행 가이드

### 1단계: C 서버 빌드 및 실행
C 서버는 `Winsock2` 라이브러리를 사용하므로 링킹 옵션(`-lws2_32`)이 필요합니다. Windows 환경에 설치된 `gcc` (MinGW) 를 통해 빌드합니다.

1. 터미널을 열고 `server` 디렉터리로 이동합니다.
   ```bash
   cd c:\WORK\SRC\tcp_poc\server
   ```
2. GCC 컴파일러를 이용해 아래 명령어로 빌드합니다.
   ```bash
   gcc server.c -o server.exe -lws2_32
   ```
3. 컴파일이 완료되면 서버를 실행합니다.
   ```bash
   .\server.exe
   ```
   *정상 실행 시 터미널에 `Server listening on port 8890...` 이라는 문구가 나타나며 클라이언트 연결을 대기합니다.*

### 2단계: C# 클라이언트 실행
1. 새로운 터미널을 열고 `client` 디렉터리로 이동합니다.
   ```bash
   cd c:\WORK\SRC\tcp_poc\client
   ```
2. .NET Core CLI를 통해 프로그램을 실행합니다.
   ```bash
   dotnet run
   ```

### 3단계: 동작 확인
* 클라이언트가 콘솔에 접속 성공 메시지를 띄우고 `Type 1`부터 `Type 4`까지의 데이터를 요청합니다. 
* 서버와 클라이언트 양쪽 터미널에서 송수신되는 JSON 데이터가 클래스로 정상적으로 변환(Deserialized)되어 화면에 출력되는지 확인합니다!
