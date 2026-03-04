# TCP Client-Server PoC (Protobuf Version)

이 프로젝트는 C 언어로 작성된 TCP 서버와 C# 간에 **Google Protocol Buffers (Protobuf)** 바이너리 포맷을 통하여 구조체 데이터를 통신하는 Proof of Concept (PoC) 구현체입니다.

## 📌 구조 및 개요
1. **Schema (`message.proto`)**
   * 통신 데이터의 기준이 되는 Protobuf 스키마 정의 파일입니다. `Type1` ~ `Type4` 구조가 정의되어 있습니다.

2. **Server (`/server`)**
   * **언어**: `C`
   * **기능**: TCP 소켓(포트 `8890`)을 열고 대기합니다. 클라이언트가 1~4 중 하나의 메세지 타입을 요청하면, 내부에 하드코딩된 더미 `struct` 데이터를 Protobuf 바이너리 형식으로 직접 인코딩하여 변환합니다. 변환된 바이너리 데이터의 길이(4 bytes)를 먼저 전송하고 뒤이어 실제 Protobuf 페이로드를 전송합니다.

3. **Client (`/client`)**
   * **언어**: `C# (.NET Console App)`
   * **기능**: `127.0.0.1:8890`에 연결하여 1부터 4까지의 메세지 타입을 2초 간격으로 서버에 순차적으로 요청합니다. 서버로부터 4바이트 크기 정보를 읽어들인 후 Protobuf 바이너리 버퍼를 수신하고, `protobuf-net` 라이브러리를 이용해 C# 객체 클래스(`Type1` ~ `Type4`)로 변환(역직렬화)하여 콘솔에 내용을 출력합니다.

---

## 🚀 실행 가이드

### 1단계: C 서버 빌드 및 실행
C 서버는 `Winsock2` 라이브러리를 사용하므로 링킹 옵션이 필요합니다. 제공된 `Makefile`을 통해 편리하게 빌드할 수 있습니다.

1. 터미널을 열고 `server` 디렉터리로 이동합니다.
   ```bash
   cd c:\WORK\SRC\tcp_poc\server
   ```
2. Make를 이용하여 빌드합니다.
   ```bash
   make
   ```
3. 컴파일이 완료되면 서버를 실행합니다. (`make run`을 입력하셔도 됩니다)
   ```bash
   .\server.exe
   ```
   *정상 실행 시 터미널에 `Server listening on port 8890...` 이라는 문구가 나타나며 클라이언트 연결을 대기합니다.*

### 2단계: C# 클라이언트 실행
1. 새로운 터미널을 열고 `client` 디렉터리로 이동합니다.
   ```bash
   cd c:\WORK\SRC\tcp_poc\client
   ```
2. .NET Core CLI를 통해 프로그램을 실행합니다. (필요 시 패키지를 자동으로 복원합니다)
   ```bash
   dotnet run
   ```

### 3단계: 동작 확인
* 클라이언트가 접속 후 `Type 1`부터 `Type 4`까지의 데이터를 순차적으로 요청합니다.
* 서버에서 전송한 Protobuf 바이너리 데이터가 C# 클래스로 정상 변환(Deserialized)되어 화면에 구조체 데이터가 정확히 출력되는지 확인합니다!
