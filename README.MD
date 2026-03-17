# TCP WebSocket & JSON Client-Server PoC

이 프로젝트는 C 언어로 작성된 서버와 C# 클라이언트 간에 **WebSocket**을 통해 **JSON 포맷**의 데이터를 주고받는 Proof of Concept (PoC) 구현체입니다.

## 📌 구조 및 개요
1. **Server (`/server`)**
   * **언어**: `C` (Mongoose 라이브러리 사용)
   * **기능**: TCP 포트 `8890`을 열고 HTTP 연결을 WebSocket으로 업그레이드하여 대기합니다. 클라이언트로부터 `CONTROL_REQ` JSON 요청을 수신하면, 지정된 9개의 크기(`0.5K, 1K, 2K, 10K, 50K, 75K, 100K, 150K, 1K`)에 맞추어 `STATUS_UPDATE` JSON 페이로드를 동적으로 더미 문자열로 패딩(Padding)하여 연속해서 응답(Push)합니다.

2. **Client (`/client`)**
   * **언어**: `C# (.NET 9 Console App)`
   * **기능**: `ClientWebSocket`을 사용하여 `ws://127.0.0.1:8890`에 연결합니다. 연결 후 제어 요청(`CONTROL_REQ`) JSON 모델을 1줄(Single Line)로 축약하여 서버에 전송합니다. 이후 서버로부터 응답받는 9번의 메시지에 대해 전체 JSON 내용을 출력하는 대신 **수신 순서(Sequence)와 수신된 데이터 크기(Bytes)** 만 출력합니다. 9번의 수신 루프가 한 번 끝날 때마다 2초의 지연 시간을 갖고, 사용자가 프로그램을 강제 종료(Ctrl+C)할 때까지 메시지 송수신 과정을 무한 반복합니다.

---

## 🚀 실행 가이드

### 1단계: C 서버 빌드 및 실행
C 서버는 `mongoose` 네트워크 라이브러리를 사용하며 `Winsock2` 라이브러리를 링킹합니다. 제공된 `Makefile`을 통해 편리하게 빌드할 수 있습니다.

1. 터미널을 열고 `server` 디렉터리로 이동합니다.
   ```bash
   cd c:\WORK\SRC\tcp_poc\server
   ```
2. Make를 이용하여 정리 및 빌드합니다.
   ```bash
   make clean
   make
   ```
3. 컴파일이 완료되면 서버를 실행합니다.
   ```bash
   .\server.exe
   ```
   *정상 실행 시 터미널에 `Starting WS Server on ws://0.0.0.0:8890` 이라는 문구가 나타나며 클라이언트 연결을 대기합니다.*

### 2단계: C# 클라이언트 실행
1. 새로운 터미널을 열고 `client` 디렉터리로 이동합니다.
   ```bash
   cd c:\WORK\SRC\tcp_poc\client
   ```
2. .NET CLI를 통해 프로그램을 실행합니다.
   ```bash
   dotnet run
   ```

### 3단계: 동작 확인
* 클라이언트가 접속함과 동시에 제어 요청 JSON을 전송하고, 9개 크기의 가변 수신 메시지 크기 요약을 출력합니다.
   ```text
   [INFO] Connected.
   [SEND] {"header":{"msgType":"CONTROL_REQ"...}}
   [RECV 1/9] 데이터 크기: 512 bytes
   [RECV 2/9] 데이터 크기: 1024 bytes
   ...
   [RECV 9/9] 데이터 크기: 1024 bytes
   [INFO] Wait 2 seconds before next loop...
   ```
* 전체 송수신 루프가 백그라운드에서 무한히 반복되는 것을 확인합니다.
